#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  int velikost = strtol(argv[1], NULL, 10);
  int *eratosten;
  eratosten = (int*) malloc(velikost/10 * sizeof(int));
  if (eratosten == NULL) {
    printf("Napaka: premalo pomnilnika.\n");
    exit(1);
  }
  int stPrastevil = 1;
  eratosten[0] = 1;
  int time_z = time(NULL);
  for (int i = 2; i <= velikost; i++) {
    int stDeliteljev = 0;
    for (int j = 0; j < stPrastevil; j++) {
      if (stDeliteljev > 2) break;
      else if (i % eratosten[j] == 0) stDeliteljev += 1;
    }
    if(stDeliteljev < 2) {
      eratosten[stPrastevil] = i;
      stPrastevil += 1;
    }
    if ((int) ((float) i / velikost * 10000) % 10 == 0) {
      printf("%d / 1000\r", (int) ((float) i / velikost * 1000));
    }
  }
  printf("\nPraštevil do %d je %d\nZapisovanje ...\n", velikost, stPrastevil);

  FILE *f;
  f = fopen("Prastevila-izpis.txt", "w");
  fprintf(f, "Praštevil do %d je %d\n", velikost, stPrastevil);
  for (int i = 1; i < stPrastevil; i++) {
    fprintf(f, "%d ", eratosten[i]);
  }
  free(eratosten);
  fclose(f);
  printf("Končano. Čas: %ld s\n", time(NULL) - time_z);
  return 0;
}
