#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

int prastevilo_max;
int *eratosten;
int stPrastevil;
int i;
long time_z;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// nit za izpisovanje
void *izpisi(void* param) {
  while (i < prastevilo_max) {
    sleep(1);
    printf("%d / 1000, %ld s\r", (int) ((float) i / prastevilo_max * 1000), time(NULL) - time_z);
    fflush(stdout);
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  eratosten = (int*) malloc(prastevilo_max/10 * sizeof(int));
  if (eratosten == NULL) {
    printf("Napaka: premalo pomnilnika.\n");
    exit(1);
  }
  stPrastevil = 0;
  FILE *f;
  f = fopen("Prastevila-izpis.txt", "r");
  printf("Berem ...\r");
  do {
    fscanf(f, "%d, ");
  } while (c != EOF);
  fclose(f);
  printf("Končano.\r");

  prastevilo_max = strtol(argv[1], NULL, 10);
  pthread_t thread;
  time_z = time(NULL);
  pthread_create(&thread, NULL, izpisi, NULL);
  i = 2;
  while (i <= prastevilo_max) {
    int stDeliteljev = 0;
    for (int j = 0; j < stPrastevil; j++) {
      if (stDeliteljev > 2) break;
      else if (i % eratosten[j] == 0) stDeliteljev += 1;
    }
    if(stDeliteljev < 2) {
      eratosten[stPrastevil] = i;
      stPrastevil += 1;
    }
    pthread_mutex_lock(&mutex);
    i++;
    pthread_mutex_unlock(&mutex);
  }
  pthread_join(thread, NULL);
  printf("Praštevil do %d je %d\nZapisovanje ...\n", prastevilo_max, stPrastevil);

  f = fopen("Prastevila-izpis.txt", "w");
  fprintf(f, "Praštevil do %d je _%d.\n", prastevilo_max, stPrastevil);
  for (int i = 1; i < stPrastevil-1; i++) fprintf(f, "%d, ", eratosten[i]);
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
