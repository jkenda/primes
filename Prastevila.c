/* Copyright (C) 2019 Jakob Kenda */

#include <stdio.h>       // I/0 (branje in izpisovanje na zaslon / v datoteko)
#include <stdlib.h>      // malloc (dodeljevanje stPrastevilMaxa)
#include <time.h>        // merjenje časa
#include <pthread.h>     // nit za izpisovanje napredka
#include <omp.h>         // multithreading
#ifdef _WIN32
  #include <windows.h>   // proklet windows rab svojo knjižnico za stPrastevilMax
#endif
#include <unistd.h>      // spanje in pridobivanje velikosti preostalega stPrastevilMaxa na ne-windowsu
#include <signal.h>      // zaznavbanje signalov (Ctrl+C, napake)
#include <stdbool.h>     // knjižnica za boolean
#include <string.h>      // knjižnica za string

#include "lib/funkcije.h"    // moje funkcije
#include "lib/strings.h"     // prevodi

#define _FILENAME_PRIMES "results/primes.js"
#define _FILENAME_SPEED  "results/speed.js"

#define lock   pthread_mutex_lock
#define unlock pthread_mutex_unlock
#define jeDeljiv(deljenec, delitelj) ((deljenec) % (delitelj) == 0)

pthread_mutex_t stPrastevilLock = PTHREAD_MUTEX_INITIALIZER;

bool izhod = false;
bool imaDelitelje;
unsigned  int kandidat, *eratosten, stPrastevil;
unsigned long stPrastevilMax, time_z;

void sigFPE() {
  fprintf(stderr, STRING_ERR_FPE);
  izhod = true;
}

void sigILL() {
  fprintf(stderr, STRING_ERR_ILL);
  izhod = true;
}

void sigSEGV() {
  fprintf(stderr, STRING_ERR_SEGV);
  izhod = true;
}

void ctrlC() {
  izhod = true; imaDelitelje = true;
}

/* nit za izpisovanje */
void* izpisi(void* param) {
  int stPrastevilLocal = 0, stPrastevilLocalOld = 0, speed = 0, cas;
  FILE *s; s = fopen(_FILENAME_SPEED, "w"); fprintf(s, "speed = [");
  while (!izhod) {
    stPrastevilLocalOld = stPrastevilLocal;
    lock(&stPrastevilLock); stPrastevilLocal = stPrastevil; unlock(&stPrastevilLock);
    speed = stPrastevilLocal - stPrastevilLocalOld; cas = time(NULL) - time_z;
    printf("\r%u (%d / 1000), %s [%d/s]   ", stPrastevilLocal, (int) (stPrastevilLocal * 1000 / stPrastevilMax),
          d_h_m_s(cas), speed); fflush(stdout);
    fprintf(s, "{x:%d,y:%d},", cas, speed);
    sleep(1);
  }
  cas = time(NULL) - time_z;
  fprintf(s, "{x:%d,y:%d},", cas, speed);
  if (speed != 0) fseek(s, -1, SEEK_CUR);
  fprintf(s, "]"); fclose(s);
  return NULL;
}

int main(int argc, char **args) {
  /* ctrl + C, terminate */
  signal(SIGINT, ctrlC); signal(SIGTERM, ctrlC);
  /* handlanje napak */
  signal(SIGFPE, sigFPE); signal(SIGILL, sigILL); signal(SIGSEGV, sigSEGV);

  if (argc > 1) stPrastevilMax = strtol(args[1], NULL, 10);
  else {
    /* preveri, koliko stPrastevilMaxa je na voljo */
    printf(STRING_MEMORY_COUNTING); fflush(stdout);
    stPrastevilMax = vrniPomnilnik();
    stPrastevilMax -= stPrastevilMax / 10; // pusti 10 %
  }

  /* preveri, koliko niti je na voljo */
  int ST_NITI;
  #pragma omp parallel
  ST_NITI = omp_get_max_threads();

  /* izpiši prost stPrastevilMax in št.niti */
  printf(STRING_MEMORY_REMAINING,
          stPrastevilMax, vrniVelikost(stPrastevilMax * sizeof(unsigned int)));

  /* slovnica 😃 */
  printf(STRING_THREADS_AVAILABLE, je_sta_so(ST_NITI), ST_NITI);

  /* dodeli vse razen 10 % Eratostenovenu rešetu */
  eratosten = (unsigned int*) malloc(stPrastevilMax * sizeof(unsigned int));

  /* preberi že izračunana praštevila iz datoteke */
  unsigned int stZapisanih; char odgovor;

  int napaka = 0;
  unsigned long skupniCas = 0;
  FILE *f;
  printf(STRING_FILE_OPENING); fflush(stdout);
  f = fopen(_FILENAME_PRIMES, "r");
  if (f != NULL && strcmp(args[2], "--override")) { // če datoteka obstaja in je dostopna ter ni argumenta -override
    printf(STRING_FILE_READING); fflush(stdout);
    stPrastevil = 0;
    fseek(f, strlen("primes = ["), SEEK_CUR); // preskoči "primes = ["
    printf("%c\n", getchar());
    while (napaka != EOF) {
      napaka = fscanf(f, "%u,", &eratosten[stPrastevil]);
      stPrastevil++;
    }
    fclose(f);
    printf(STRING_FILE_READ); fflush(stdout);
    stPrastevil -= 2; // Brez EOF in ']'
    stZapisanih = stPrastevil;
    printf(STRING_PRIME_NUMBER_OF, stPrastevil, eratosten[stPrastevil - 1]);
    f = fopen("results/.time", "r");
    if (f != NULL) napaka = fscanf(f, "%lu", &skupniCas);
    printf(STRING_TIME_PREVIOUS, d_h_m_s(skupniCas));
    if (strcmp(args[2], "-override")) {
      printf(STRING_CONTINUE);
      odgovor = getchar();
      if (odgovor == 'n' || odgovor == 'N') exit(0);
    }
  }
  else {
    fclose(f);
    if (strcmp(args[2], "--override")) {
      printf(STRING_FILE_CANNOT_OPEN);
      odgovor = getchar(); if (odgovor == 'n' || odgovor == 'N') exit(0);
    }
    eratosten[0] = 2;
    stPrastevil = 1;
    stZapisanih = 0;
  }

  printf(STRING_THREADS_CREATING); fflush(stdout);

  /* inicializiraj in zaženi nit za izpisovanje */
  time_z = time(NULL) - skupniCas;
  pthread_t izpisovalnik;
  pthread_create(&izpisovalnik, NULL, izpisi, NULL);

  /* računaj praštevila */
  kandidat = eratosten[stPrastevil - 1] % 2 == 0
             ? eratosten[stPrastevil - 1] + 1  // če je sodo
             : eratosten[stPrastevil - 1] + 2; // če je liho
  time_z = time(NULL) - skupniCas;
  while (stPrastevil < stPrastevilMax && !izhod) {
    imaDelitelje = false;
    // #pragma omp simd
    for (int i = 0; i < stPrastevil && !imaDelitelje; i++)
      if (jeDeljiv(kandidat,eratosten[i])) imaDelitelje = true;
    /* preveri, ali ima delitelje */
    if (!imaDelitelje) {
      eratosten[stPrastevil] = kandidat;
      lock(&stPrastevilLock);
      stPrastevil++;
      unlock(&stPrastevilLock);
    }
    /* preveri naslednjega kandidata */
    kandidat += 2;
  }

  /* ob končanem izvajanju */
  unsigned long time_k = time(NULL);
  izhod = true; pthread_join(izpisovalnik, NULL);
  printf(STRING_WRITING, eratosten[stPrastevil - 1], stPrastevil);
  fflush(stdout);
  /* zapiši na novo izračuana praštevila kot comma-separated values (CSV) */
  f = fopen(_FILENAME_PRIMES, "w");
  if (stZapisanih == 0) {
    fseek(f, 0, SEEK_END); fprintf(f, "primes = [");
  }
  else {
    fseek(f, -1, SEEK_END); fprintf(f, ",");
  }
  for (int i = stZapisanih; i < stPrastevil; i++) {
    fprintf(f, "%d,", eratosten[i]);
  }
  /* sprosti stPrastevilMax, zaključi in zapri datoteko */
  free(eratosten); fseek(f, -1, SEEK_END); fprintf(f, "]"); fclose(f);
  /* zapiši skupni porabljen čas v datoteko */
  f = fopen("results/.time", "a");
  if(time_k - time_z > 0) fprintf(f, "\r%lu", time_k - time_z);
  printf(STRING_FINISHED, d_h_m_s(time_k - time_z));
  return 0;
}
