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
unsigned int i, *eratosten, stPrastevil, stDeliteljev, napaka;
unsigned long pomnilnik, time_z;
unsigned short ST_NITI, stIzracunanih;

pthread_mutex_t i_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stDeliteljev_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t izracunano_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t izracunaj = PTHREAD_COND_INITIALIZER;
pthread_cond_t izracunano = PTHREAD_COND_INITIALIZER;

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

/* nit za izpisovanje */
void *izpisi(void* param) {
  while (!izhod) {
    sleep(1);
    printf("\r%d (%d / 1000), %ld s", i, (int) ((float) i / pomnilnik * 1000),
                                           time(NULL) - time_z);
    fflush(stdout);
  }
  return NULL;
}

void ctrlC() {
  izhod = true;
}

unsigned long getTotalSystemMemory() {
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);
  return pages * page_size;
}

void *racunaj(void* nit) {
  unsigned short *idNiti = (unsigned short*) nit;
  while (stPrastevil < pomnilnik && !izhod) {
    cond_wait(&izracunaj, &stDeliteljev_lock);
    printf("Začeta nit %hu\n", *idNiti);
    for (int j = *idNiti; j < stPrastevil; j += ST_NITI) {
      if (stDeliteljev >= 2) break;
      if (i % eratosten[j] == 0) {
        lock(&stDeliteljev_lock);
        stDeliteljev++;
        unlock(&stDeliteljev_lock);
      }
    }
    lock(&izracunano_lock);
    stIzracunanih++;
    unlock(&izracunano_lock);
    if (stIzracunanih == ST_NITI) cond_broadcast(&izracunano);
    printf("Končana nit %hu\n", *idNiti);
  }
  return NULL;
}

int main() {
  /* ctrl + C, terminate */
  signal(SIGINT, ctrlC); signal(SIGTERM, ctrlC);
  /* handlanje napak */
  signal(SIGFPE, sigFPE); signal(SIGILL, sigILL); signal(SIGSEGV, sigSEGV);

  /* preveri, koliko pomnilnika je na voljo */
  printf("Štejem, koliko pomnilnika je na voljo ...\r"); fflush(stdout);
  pomnilnik = getTotalSystemMemory() / sizeof(unsigned int);
  pomnilnik -= pomnilnik / 10; // pusti 10 %

  /* preveri, koliko niti je na voljo */
  ST_NITI = get_nprocs() - 1;

  /* izpiši prost pomnilnik in št.niti */
  printf("V pomnilniku je prostora za %ld praštevil (%f GB).\n",
          pomnilnik, (float) pomnilnik * sizeof(unsigned int) / 1000000000);

  /* slovnica 😃 */
  char mnozina[4]; if (ST_NITI == 2) strcpy(mnozina, "sta");
  else if (ST_NITI == 3 || ST_NITI == 4) strcpy(mnozina, "so"); else strcpy(mnozina, "je");
  printf("Na voljo %s %hu niti za računanje in 1 za izpisovanje napredka.\n", mnozina, ST_NITI);

  /* dodeli vse razen 10 % Eratostenovenu rešetu */
  eratosten = (unsigned int*) malloc(pomnilnik * sizeof(unsigned int));

  /* preberi že izračunana praštevila iz datoteke */
  stPrastevil = 0;
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
  }
  printf("\b\bŠt. praštevil: %d (do %u)\n", stPrastevil, eratosten[stPrastevil - 1]);
  int stZapisanih = stPrastevil + 1;

  printf("Ali želite nadaljevati? [Y/n] ");
  char nadaljujem = getchar();
  if (nadaljujem == 'n' || nadaljujem == 'N') exit(0);

  printf("\rUstvarjam niti ...\r"); fflush(stdout);

  /* inicializiraj in zaženi nit za izpisovanje */
  time_z = time(NULL);
  pthread_t izpisovalnik;
  pthread_create(&izpisovalnik, NULL, izpisi, NULL);

  /* inicializiraj in zaženi niti za računanje */
  pthread_t racunanje[ST_NITI];
  unsigned short idNiti[ST_NITI];
  for (int n = 0; n < ST_NITI; n++) idNiti[n] = n;
  for (int n = 0; n < ST_NITI; n++) pthread_create(&racunanje[n], NULL, racunaj, &idNiti[n]);

  /* začni računati praštevila */
  i = stPrastevil + 1;
  while (stPrastevil <= pomnilnik && !izhod) {
    stDeliteljev = 0;
    /* daj signal za računanje */
    lock(&izracunano_lock);
    stIzracunanih = 0;
    unlock(&izracunano_lock);
    cond_broadcast(&izracunaj);
    /* počakaj, da se izračuna */
    cond_wait(&izracunano, &i_lock);
    /* preveri št. deliteljev */
    if(stDeliteljev < 2) {
      eratosten[stPrastevil] = i;
      stPrastevil++;
    }
    /* naslednje število */
    lock(&i_lock);
    i++;
    unlock(&i_lock);
  }

  /* ob končanem izvajanju */
  izhod = true; pthread_cancel(izpisovalnik); for (int n = 0; n < ST_NITI; n++) pthread_cancel(racunanje[n]);
  printf("\rPraštevil do %d je %d         \nZapisovanje ...\r", eratosten[stPrastevil - 1], stPrastevil);
  fflush(stdout);
  /* zapiši na novo izračuana praštevila */
  f = fopen("Prastevila-izpis.txt", "a");
  for (int i = stZapisanih + 1; i < stPrastevil; i++) fprintf(f, "%d, ", eratosten[i]);
  free(eratosten); fclose(f); // sprosti pomnilnik, zapri datoteko
  /* izračunaj porabljen čas v H, M, S */
  int s = time(NULL) - time_z;
  int h = s / 3600;
  s %= 3600;
  int m = s / 60;
  s %= 60;
  printf("Končano. Čas: %dh, %dm, %ds\n", h, m, s);
  return 0;
}
