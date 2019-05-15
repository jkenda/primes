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
#include "funkcije.h"    // moje funkcije

#define lock           pthread_mutex_lock
#define unlock         pthread_mutex_unlock
#define cond_signal    pthread_cond_signal
#define cond_broadcast pthread_cond_broadcast
#define cond_wait      pthread_cond_wait

/*
typedef struct mt {
  bool je_izracunana;
  unsigned char id_niti;
  unsigned char delitelji;
} podatkiNiti;
*/

bool izhod = false;
bool imaDelitelje;
unsigned char ST_NITI, maska, izracunaneNiti;
unsigned  int kandidat, *eratosten, stPrastevil, napaka;
unsigned long pomnilnik, time_z;

pthread_mutex_t kandidat_lock, stDeliteljevLock, izracunaneNitiLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t izracunaj, izracunano = PTHREAD_COND_INITIALIZER;

void sigFPE() {
  fprintf(stderr, "\nNAPAKA! Prišlo je do preliva (ali deljenja z 0).\n");
  izhod = true; imaDelitelje = 1;
  izracunaneNiti = maska; cond_signal(&izracunano);
}

void sigILL() {
  fprintf(stderr, "\nNAPAKA! Koda je pokvarjena ali pa program poskuša izvesti podatke.\n");
  izhod = true; imaDelitelje = 1;
  izracunaneNiti = maska; cond_signal(&izracunano);
}

void sigSEGV() {
  fprintf(stderr, "\nNAPAKA! Program je hotel brati izven svojega pomnilnika (segmentation fault).\n");
  izhod = true; imaDelitelje = 1;
  izracunaneNiti = maska; cond_signal(&izracunano);
}

void ctrlC() {
  izhod = true; imaDelitelje = 1;
  izracunaneNiti = maska; cond_signal(&izracunano);
}

/* nit za izpisovanje */
void* izpisi(void* param) {
  int lokalni_kandidat;
  while (!izhod) {
    lock(&kandidat_lock); lokalni_kandidat = kandidat; unlock(&kandidat_lock);
    printf("\r%d (%d / 1000), %s ", kandidat, (int) (lokalni_kandidat * 1000 / pomnilnik),
          d_h_m_s(time(NULL) - time_z));
    fflush(stdout);
    sleep(1);
  }
  return NULL;
}

void* racunaj(void* nitPtr) {
  unsigned char nit = *((unsigned char*) nitPtr);

  while (stPrastevil < pomnilnik && !izhod) {
    lock(&izracunaneNitiLock);
    while((izracunaneNiti >> nit) & 1U) cond_wait(&izracunaj, &izracunaneNitiLock);
    unlock(&izracunaneNitiLock);
    for (int i = nit; i < stPrastevil; i += ST_NITI) {
      if (kandidat % eratosten[i] == 0) imaDelitelje = true;
      lock(&stDeliteljevLock);
      if (imaDelitelje) {
        unlock(&stDeliteljevLock);
        break;
      }
      unlock(&stDeliteljevLock);
    }
    lock(&izracunaneNitiLock);
    izracunaneNiti |= 1U << nit;   // nastavi bit niti na 1
    if (izracunaneNiti == maska) { // končajo vse niti
      unlock(&izracunaneNitiLock);
      cond_signal(&izracunano);
    }
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
  /* nastavitev maske za sinhronizacijo niti */
  for (int i = 0; i < ST_NITI; i++) maska |= 1 << i; // nastavi i-ti bit maske na 1

  /* izpiši prost pomnilnik in št.niti */
  printf("V pomnilniku je prostor za %ld praštevil (%s).\n",
          pomnilnik, vrniVelikost(pomnilnik * sizeof(unsigned int)));

  /* slovnica 😃 */
  printf("Na voljo %s %hu niti za računanje.\n", je_sta_so(ST_NITI), ST_NITI);

  /* dodeli vse razen 10 % Eratostenovenu rešetu */
  eratosten = (unsigned int*) malloc(pomnilnik * sizeof(unsigned int));

  /* preberi že izračunana praštevila iz datoteke */
  unsigned int stZapisanih; char odgovor;

  FILE *f;
  printf("Odpiram datoteko s praštevili ... "); fflush(stdout);
  f = fopen("Prastevila-izpis.csv", "r");
  if (f != NULL) { // če datoteka obstaja in je dostopna
    printf("Berem datoteko ...   \r"); fflush(stdout);
    stPrastevil = 0;
    while (napaka != EOF) {
      // if (izhod) exit(0);
      napaka = fscanf(f, "%u, ", &eratosten[stPrastevil]);
      stPrastevil++;
    }
    fclose(f);
    printf("Datoteka prebrana.   "); fflush(stdout);
    stPrastevil -= 1; // Brez EOF (0)
    stZapisanih = stPrastevil;
    printf("\b\bŠt. praštevil: %d (do %u)\n", stPrastevil, eratosten[stPrastevil - 1]);
    printf("Ali želite nadaljevati? [Y/n] ");
    odgovor = getchar();
    if (odgovor == 'n' || odgovor == 'N') exit(0);
  }
  else {
    printf("\b\b\b\b\b. Datoteke ne morem odpreti. Ustvarim novo? [Y/n] ");
    odgovor = getchar(); if (odgovor == 'n' || odgovor == 'N') exit(0);
    eratosten[0] = 2;
    stPrastevil = 1;
    stZapisanih = 0;
  }

  printf("Ustvarjam niti ...\r"); fflush(stdout);

  /* inicializiraj in zaženi nit za izpisovanje */
  time_z = time(NULL);
  pthread_t izpisovalnik;
  pthread_create(&izpisovalnik, NULL, izpisi, NULL);

  /* inicializiraj in zaženi niti za računanje */
  izracunaneNiti = maska;
  pthread_t racunanje[ST_NITI];
  short idNiti[ST_NITI];
  for (short i = 0; i < ST_NITI; i++) idNiti[i] = i;
  for (short i = 0; i < ST_NITI; i++) pthread_create(&racunanje[i], NULL, racunaj, &idNiti[i]);

  /* začni računati praštevila */
  kandidat = eratosten[stPrastevil - 1] + 1;
  time_z = time(NULL);
  while (stPrastevil < pomnilnik && !izhod) {
    imaDelitelje = false; izracunaneNiti = 0;
    /* daj signal za računanje */
    cond_broadcast(&izracunaj);
    /* počakaj, da se izračuna */
    lock(&izracunaneNitiLock);
    while(izracunaneNiti != maska) cond_wait(&izracunano, &izracunaneNitiLock);
    unlock(&izracunaneNitiLock);
    /* preveri št. deliteljev */
    if (!imaDelitelje) {
      eratosten[stPrastevil] = kandidat;
      stPrastevil++;
      // printf("%u je praštevilo. %u\n", kandidat, imaDelitelje); // debugging
    }
    // else printf("%u ni praštevilo. %u\n", kandidat, imaDelitelje); // debugging

    lock(&kandidat_lock);
    kandidat++;
    unlock(&kandidat_lock);
    /* preveri naslednjega kandidata */
  }

  /* ob končanem izvajanju */
  unsigned long time_k = time(NULL);
  izhod = true; pthread_cancel(izpisovalnik); for (int i = 0; i < ST_NITI; i++) pthread_cancel(racunanje[i]);
  printf("\rPraštevil do %d je %d         \nZapisovanje ...\r", eratosten[stPrastevil - 1], stPrastevil);
  fflush(stdout);
  /* zapiši na novo izračuana praštevila kot comma-separated values (CSV) */
  f = fopen("Prastevila-izpis.csv", "a");
  for (int i = stZapisanih; i < stPrastevil; i++) fprintf(f, "%d,", eratosten[i]);
  free(eratosten); fclose(f); // sprosti pomnilnik, zapri datoteko
  printf("Končano. Čas: %s\n", d_h_m_s(time_k - time_z));
  return 0;
}
