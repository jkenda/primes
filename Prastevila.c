/* Copyright (C) 2019 Jakob Kenda */

#include <stdio.h>   // I/0 (branje in izpisovanje na zaslon / v datoteko)
#include <stdlib.h>  // malloc (dodeljevanje pomnilnika)
#include <time.h>    // merjenje časa
#include <pthread.h> // multithreading
#include <unistd.h>  // spanje in pridobivanje velikosti preostalega pomnilnika
#include <signal.h>  // zaznavanje signalov (Ctrl+C)
#include <stdbool.h> // knjižnica za boolean (lahko nadomestiš z:
                     // #define bool int \n #define true 1 \n #define false 0)

bool izhod = false;
unsigned int *eratosten, i;
int stPrastevil, napaka;
unsigned long pomnilnik, time_z;
pthread_t thread; // tip za niti

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* nit za izpisovanje */
void *izpisi(void* param) {
  while (!izhod) {
    sleep(1);
    printf("%d (%d / 1000), %ld s          \r", i, (int) ((float) i / pomnilnik * 1000),
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

int main() {
  /* ctrl + C */
  signal(SIGINT, ctrlC);
  /* preveri, koliko pomnilnika je na voljo */
  printf("Štejem, koliko pomnilnika je na voljo ...\r"); fflush(stdout);
  pomnilnik = getTotalSystemMemory() / sizeof(unsigned int);
  pomnilnik -= pomnilnik / 10;

  printf("V pomnilniku je prostora za %ld praštevil (%f GB).\n", pomnilnik,
          (float) pomnilnik * sizeof(unsigned int) / 1000000000);
  /* dodeli vse razen 10 % Eratostenovenu rešetu */
  eratosten = (unsigned int*) malloc(pomnilnik * sizeof(unsigned int));
  /* preberi že izračunana praštevila iz datoteke */
  stPrastevil = 0;
  FILE *f;
  printf("Odpiram datoteko ...\r"); fflush(stdout);
  f = fopen("Prastevila-izpis.txt", "r");
  if (f != NULL) {
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
  printf("\b\bŠt. praštevil: %d (do %u)\n", stPrastevil, eratosten[stPrastevil - 1]); //debugging
  int stZapisanih = stPrastevil + 1;

  printf("Ali želite nadaljevati? [Y/n] ");
  char nadaljujem = getchar();
  if (nadaljujem == 'n' || nadaljujem == 'N') exit(0);

  /* zaženi nit za izpisovanje */
  time_z = time(NULL);
  printf("\rRačunam ...\r"); fflush(stdout);
  pthread_create(&thread, NULL, izpisi, NULL);
  /* začni računati praštevila */
  int stDeliteljev;
  i = stPrastevil + 1;
  while (stPrastevil <= pomnilnik && !izhod) {
    stDeliteljev = 0;
    for (int j = 0; j < stPrastevil; j++) {
      if (stDeliteljev >= 2) break;
      if (i % eratosten[j] == 0) stDeliteljev++;
    }
    if(stDeliteljev < 2) {
      eratosten[stPrastevil] = i;
      stPrastevil++;
    }
    pthread_mutex_lock(&mutex);
    i++;
    pthread_mutex_unlock(&mutex);
  }
  izhod = true; pthread_cancel(thread);
  printf("\rPraštevil do %d je %d\nZapisovanje ...\r", eratosten[stPrastevil - 1], stPrastevil);
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
