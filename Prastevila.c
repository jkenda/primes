#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define ST_NITI 4

int  prastevilo_max, stPrastevil;
int  *eratosten;
int  i;
int  j[ST_NITI];
long time_z;
int  stDeliteljev;
int  lock;

pthread_mutex_t mutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t prestej = PTHREAD_COND_INITIALIZER;
pthread_cond_t nova    = PTHREAD_COND_INITIALIZER;

// nit za izpisovanje napredka
void *izpisi(void* param) {
  int s, h, m;
  while (i < prastevilo_max) {
    sleep(1);

    s = time(NULL) - time_z;
    h = s / 3600;
    s %= 3600;
    m = s / 60;
    s %= 60;
    printf("%d / 1000, %dh, %dm %ds\r", (int) ((float) i / prastevilo_max * 1000), h, m, s);
    fflush(stdout);
  }
  pthread_exit(0);
  return NULL;
}

void *racunaj(void *param) {
  int *nit = param;
  int check;
  while (i <= prastevilo_max) {
    check = 0;
    // printf("Thread racunaj: i = %d, j = %ls\r", i, j);
    // fflush(stdout);
    if (stDeliteljev != 0) pthread_cond_signal(&prestej);
    else if (i % eratosten[j[*nit]] == 0) {
      stDeliteljev += 1;
    }
    pthread_mutex_lock(&mutex);
    j[*nit] += ST_NITI;
    pthread_mutex_unlock(&mutex);
    sleep(1);
    if (j[*nit] >= stPrastevil) pthread_cond_wait(&nova, &mutex);
    for (int k = 0; k < ST_NITI; k++) if (j[k] >= stPrastevil) check++;
    if (check == ST_NITI) {
      // pojdi na preverjanje
      pthread_cond_signal(&prestej);
    }
  }
  pthread_exit(0);
  return NULL;
}

// glavna nit
int main(int argc, char *argv[]) {
  // ustvarimo spremeljivke za številke niti
  int idNiti[ST_NITI];
  for (int i = 0; i < ST_NITI; i++) idNiti[i] = i;
  // preberemo argument z največjim številom
  prastevilo_max = strtol(argv[1], NULL, 10);
  // dodelimo pomnilnik na kopici
  eratosten = (int*) malloc(prastevilo_max * sizeof(int));
  if (eratosten == NULL) {
    printf("Napaka: premalo pomnilnika.\n");
    exit(1);
  }
  //
  stPrastevil = ST_NITI;
  eratosten[0] = 2;
  eratosten[1] = 3;
  eratosten[2] = 5;
  eratosten[3] = 7;
  i = 8;
  for (int k = 0; k < ST_NITI; k++) j[k] = k;
  pthread_t izpisovanje_nit;
  pthread_t prastevila[ST_NITI];
  time_z = time(NULL);
  // začni nit za izpisovanje
  pthread_create(&izpisovanje_nit, NULL, izpisi, NULL);
  // začni niti za računanje
  for (int k = 0; k < ST_NITI; k++) pthread_create(&prastevila[k], NULL, racunaj, &idNiti[k]);
  // pojdi skozi vsa števila
  while (i <= prastevilo_max) {
    pthread_cond_wait(&prestej, &mutex);
    // pthread_mutex_lock(&mutex);
    if(stDeliteljev == 0) {
      eratosten[stPrastevil] = i;
      stPrastevil += 1;
    }
    // printf("Thread main: i = %d, j = %ls\r", i, j);
    i++;
    stDeliteljev = 0;
    for (int k = 0; k < ST_NITI; k++) j[k] = k;
    // pthread_mutex_unlock(&mutex);
    pthread_cond_broadcast(&nova);
  }
  // počakaj na konec niti
  pthread_join(izpisovanje_nit, NULL);
  for (int k = 0; k < ST_NITI; k++) pthread_join(prastevila[k], NULL);
  printf("Praštevil do %d je %d\nZapisovanje ...\n", prastevilo_max, stPrastevil);
  // zapiši rezultat v datoteko
  FILE *f;
  f = fopen("Prastevila-izpis.txt", "w");
  fprintf(f, "Praštevil do %d je %d.\n", prastevilo_max, stPrastevil);
  for (int i = 0; i < stPrastevil-1; i++) fprintf(f, "%d, ", eratosten[i]);
  fprintf(f, "%d\n", eratosten[stPrastevil-1]);
  free(eratosten);
  fclose(f);
  int s = time(NULL) - time_z;
  int h = s / 3600;
  s %= 3600;
  int m = s / 60;
  s %= 60;
  printf("Končano. Čas: %dh, %dm, %ds\n", h, m, s);
  return 0;
}
