/* Copyright (C) 2019 Jakob Kenda */

#include <stdio.h>       // I/0 (branje in izpisovanje na zaslon / v datoteko)
#include <stdlib.h>      // malloc (dodeljevanje pomnilnika)
#include <time.h>        // merjenje časa
#include <pthread.h>     // multithreading
#include <unistd.h>      // spanje in pridobivanje velikosti preostalega pomnilnika
#include <signal.h>      // zaznavanje signalov (Ctrl+C, napake)
#include <sys/sysinfo.h> // število niti
#include <stdbool.h>     // knjižnica za boolean
#include <string.h>      // knjižnica za string

#define lock pthread_mutex_lock
#define unlock pthread_mutex_unlock
#define cond_signal    pthread_cond_signal
#define cond_broadcast pthread_cond_broadcast
#define cond_wait      pthread_cond_wait

bool izhod = false;
unsigned char ST_NITI, stIzracunanih;
unsigned  int kandidat, *eratosten, stPrastevil, stDeliteljev, napaka;
unsigned long pomnilnik, time_z;

pthread_mutex_t kandidat_lock, stDeliteljevLock, izracunano_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t izracunaj, izracunano = PTHREAD_COND_INITIALIZER;

void sigFPE() {
  printf("\nNAPAKA! Prišlo je do preliva (ali deljenja z 0).\n");
  izhod = true;
  stPrastevil--;
}

void sigILL() {
  printf("\nNAPAKA! Koda je pokvarjena ali pa program poskuša izvesti podatke.\n");
  izhod = true;
  stPrastevil--;
}

void sigSEGV() {
  printf("\nNAPAKA! Program je hotel brati izven svojega pomnilnika (segmentation fault).\n");
  izhod = true;
  stPrastevil--;
}

void ctrlC() {
  izhod = true;
}

unsigned long vrniPomnilnik() {
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);
  return pages * page_size;
}

const char* vrniVelikost(long bytes) {
  static char velikost_enota[9];
  char enota[3] = "B";
  if (bytes >= 1000) {
    bytes /= 1000; strcpy(enota, "kB");
  }
  if (bytes >= 1000) {
    bytes /= 1000; strcpy(enota, "MB");
  }
  if (bytes >= 1000) {
    bytes /= 1000; strcpy(enota, "GB");
  }
  sprintf(velikost_enota, "%d %s", (unsigned short) bytes, enota);
  return velikost_enota;
}

/* nit za izpisovanje */
void *izpisi(void* param) {
  int trenutnaSt;
  while (!izhod) {
    lock(&kandidat_lock); trenutnaSt = kandidat; unlock(&kandidat_lock);
    printf("\r%d (%d / 1000), %ld s", kandidat, (int) (trenutnaSt * 1000 / pomnilnik),
          time(NULL) - time_z);
    fflush(stdout);
    sleep(1);
  }
  return NULL;
}

void *racunaj(void* nit) {
  unsigned char *idNiti = (unsigned char*) nit;

  while (stPrastevil < pomnilnik && !izhod) {
    lock(&izracunano_lock);
    while(stIzracunanih == ST_NITI) cond_wait(&izracunaj, &izracunano_lock);
    unlock(&izracunano_lock);
    for (int j = *idNiti; j < stPrastevil; j += ST_NITI) {
      lock(&stDeliteljevLock);
      if (kandidat % eratosten[j] == 0) {
        stDeliteljev++;
      }
      if (stDeliteljev > 0) {
        unlock(&stDeliteljevLock);
        break;
      }
      unlock(&stDeliteljevLock);
    }
    lock(&izracunano_lock);
    stIzracunanih++;
    if (stIzracunanih == ST_NITI) cond_signal(&izracunano);
    unlock(&izracunano_lock);
  }
  return NULL;
}

int main(int argc, char **args) {
  /* ctrl + C, terminate */
  signal(SIGINT, ctrlC); //signal(SIGTERM, ctrlC);
  /* handlanje napak */
  // signal(SIGFPE, sigFPE); signal(SIGILL, sigILL); signal(SIGSEGV, sigSEGV);

  if (argc > 1) pomnilnik = strtol(args[1], NULL, 10);
  else {
    /* preveri, koliko pomnilnika je na voljo */
    printf("Štejem, koliko pomnilnika je na voljo ...\r"); fflush(stdout);
    pomnilnik = vrniPomnilnik() / sizeof(unsigned int);
    pomnilnik -= pomnilnik / 10; // pusti 10 %
  }

  /* preveri, koliko niti je na voljo */
  ST_NITI = get_nprocs();

  /* izpiši prost pomnilnik in št.niti */
  printf("V pomnilniku je prostora za %ld praštevil (%s).\n",
          pomnilnik, vrniVelikost(pomnilnik * sizeof(unsigned int)));

  /* slovnica 😃 */
  char je_sta_so[4]; if (ST_NITI == 2) strcpy(je_sta_so, "sta");
  else if (ST_NITI == 3 || ST_NITI == 4) strcpy(je_sta_so, "so"); else strcpy(je_sta_so, "je");
  printf("Na voljo %s %hu niti za računanje.\n", je_sta_so, ST_NITI);

  /* dodeli vse razen 10 % Eratostenovenu rešetu */
  eratosten = (unsigned int*) malloc(pomnilnik * sizeof(unsigned int));

  /* preberi že izračunana praštevila iz datoteke */
  stPrastevil = 0;
  int stZapisanih;
  FILE *f;
  printf("Odpiram datoteko ...\r"); fflush(stdout);
  f = fopen("Prastevila-izpis.txt", "r");
  if (f != NULL) { // če datoteka obstaja in je dostopna
    printf("Berem datoteko ...   \r"); fflush(stdout);
    while (napaka != EOF) {
      if (izhod) exit(0);
      napaka = fscanf(f, "%u, ", &eratosten[stPrastevil]);
      stPrastevil++;
    }
    fclose(f);
    printf("Datoteka prebrana.   "); fflush(stdout);
    stPrastevil -= 1; // Brez EOF (0)
    stZapisanih = stPrastevil;
  }
  else {
    eratosten[0] = 2;
    stPrastevil = 1;
    stZapisanih = 0;
  }
  printf("\b\bŠt. praštevil: %d (do %u)\n", stPrastevil, eratosten[stPrastevil - 1]);

  printf("Ali želite nadaljevati? [Y/n] ");
  char nadaljujem = getchar();
  if (nadaljujem == 'n' || nadaljujem == 'N') exit(0);

  printf("\rUstvarjam niti ...\r"); fflush(stdout);

  /* inicializiraj in zaženi nit za izpisovanje */
  pthread_t izpisovalnik;
  pthread_create(&izpisovalnik, NULL, izpisi, NULL);

  /* inicializiraj in zaženi niti za računanje */
  stIzracunanih = ST_NITI;
  pthread_t racunanje[ST_NITI];
  unsigned short idNiti[ST_NITI];
  for (int n = 0; n < ST_NITI; n++) idNiti[n] = n;
  for (int n = 0; n < ST_NITI; n++) pthread_create(&racunanje[n], NULL, racunaj, &idNiti[n]);

  /* začni računati praštevila */
  kandidat = eratosten[stPrastevil - 1] + 1;
  time_z = time(NULL);
  while (stPrastevil < pomnilnik && !izhod) {
    stDeliteljev = 0; stIzracunanih = 0;
    /* daj signal za računanje */
    cond_broadcast(&izracunaj);
    /* počakaj, da se izračuna */
    lock(&izracunano_lock);
    while(stIzracunanih < ST_NITI) cond_wait(&izracunano, &izracunano_lock);
    unlock(&izracunano_lock);
    /* preveri št. deliteljev */
    if (stDeliteljev == 0) {
      eratosten[stPrastevil] = kandidat;
      stPrastevil++;
      // printf("%u je praštevilo. %u\n", kandidat, stDeliteljev); // debugging
    }
    // else printf("%u ni praštevilo. %u\n", kandidat, stDeliteljev); // debugging
    /* naslednje število */
    lock(&kandidat_lock);
    kandidat++;
    unlock(&kandidat_lock);
  }

  /* ob končanem izvajanju */
  unsigned int time_k = time(NULL);
  izhod = true; pthread_cancel(izpisovalnik); for (int n = 0; n < ST_NITI; n++) pthread_cancel(racunanje[n]);
  printf("\rPraštevil do %d je %d         \nZapisovanje ...\r", eratosten[stPrastevil - 1], stPrastevil);
  fflush(stdout);
  /* zapiši na novo izračuana praštevila */
  f = fopen("Prastevila-izpis.txt", "a");
  for (int kandidat = stZapisanih; kandidat < stPrastevil; kandidat++) fprintf(f, "%d, ", eratosten[kandidat]);
  free(eratosten); fclose(f); // sprosti pomnilnik, zapri datoteko
  /* izračunaj porabljen čas v H, M, S */
  int s = time_k - time_z;
  int h = s / 3600;
  s %= 3600;
  int m = s / 60;
  s %= 60;
  printf("Končano. Čas: %dh, %dm, %ds\n", h, m, s);
  return 0;
}
