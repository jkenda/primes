#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  int velikost = strtol(argv[1], NULL, 10);
  int stPrastevil = 0;
  FILE *f;
  f = fopen("Prastevila-izpis.txt", "w");
  int time_z = time(NULL);
  for (int i = 2; i <= velikost; i++) {
    int stDeliteljev = 0;
    for (int j = 1; j <= i / 2; j++) {
      if (stDeliteljev > 2) {
        break;
      }
      if (i % j == 0) {
        stDeliteljev += 1;
      }
    }
    if(stDeliteljev < 2) {
      fprintf(f, "%d, ", i);
      stPrastevil += 1;
    }
    printf("%d %%\r", (int) ((float) i / velikost * 100));
  }
  printf("\nPraštevil do %d je %d\n", velikost, stPrastevil);
  fprintf(f, "\nPraštevil do %d je %d\n", velikost, stPrastevil);
  fclose(f);
  printf("Končano. Čas: %ld s\n", time(NULL) - time_z);
}
