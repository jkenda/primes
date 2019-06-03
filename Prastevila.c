/* Copyright (C) 2019 Jakob Kenda */

#pragma GCC diagnostic ignored "-Wunused-result"

#include <stdio.h>       // I/0 (reading and writing on screen / to file)
#include <stdlib.h>      // malloc (allocating memory)
#include <time.h>        // measuring time consumption
#include <pthread.h>     // thread for info screen
#include <omp.h>         // multithreading
#ifdef _WIN32
  #include <windows.h>   // damn windows needs its own library
#endif
#include <unistd.h>      // sleep() and getting available memory
#include <signal.h>      // signal handling (ctrl+C, exceptions)
#include <stdbool.h>     // boolean library
#include <string.h>      // string library

#include "lib/funkcije.h"    // my functions
#include "lib/strings.h"     // translation

#define _FILENAME_PRIMES "results/primes.js"
#define _FILENAME_SPEED  "results/speed.js"
#define _FILENAME_TEMP   "/sys/bus/platform/devices/coretemp.0/hwmon/hwmon1/temp1_input"
#define _FILENAME_USAGE  "/proc/stat"

#define strmatch(str1, str2) (!strcmp(str1, str2))
typedef unsigned int prime_type;
typedef unsigned char byte;

bool finish = false, finished = false;
prime_type candidate, *primes, prime_counter, max_primes = 0;
unsigned long time_z;

void sigFPE() 
{
  fprintf(stderr, _STRING_ERR_FPE);
  finish = true;
}

void sigILL() {
  fprintf(stderr, _STRING_ERR_ILL);
  finish = true;
}

void sigSEGV() {
  fprintf(stderr, _STRING_ERR_SEGV);
  finish = true;
}

void ctrlC() {
  finish = true;
}

/* info screen thread */
void* izpisi(void* param) 
{ 
  /* file pointers */
  FILE *tempInput, *speedOutput, *usageInput;

  int prime_counter_local, prime_counter_local_old, speed, time_curr, temp, usage_curr, usage_last;
  prime_counter_local = 0; prime_counter_local_old = 2; speed = 0; usage_last = 0;
  speedOutput = fopen(_FILENAME_SPEED, "w"); fprintf(speedOutput, "speed = [");
  while (!finished) {
    prime_counter_local_old = prime_counter_local;
    prime_counter_local = prime_counter; 
    speed     = prime_counter_local - prime_counter_local_old; 
    time_curr = time(NULL) - time_z;
    tempInput  = fopen(_FILENAME_TEMP, "r");  fscanf(tempInput, "%d", &temp); fclose(tempInput);
    usageInput = fopen(_FILENAME_USAGE, "r"); fseek(usageInput, 4, SEEK_CUR); fscanf(usageInput, "%d", &usage_curr); fclose(usageInput);

    printf("\r%u (%u / 1000), %s [%d/s], CPU: %d °C, %d %% ", 
      prime_counter_local, (int) (prime_counter_local * 1000 / max_primes), 
      d_h_m_s(time_curr), speed, temp / 1000, (usage_curr - usage_last) / 10); 
    fflush(stdout);
    fprintf(speedOutput, "{x:%d,y:%d},", time_curr, speed);
    usage_last = usage_curr;
    sleep(1);
  }
  time_curr = time(NULL) - time_z;
  fprintf(speedOutput, "{x:%d,y:%d},", time_curr, speed);
  if (speed != 0) fseek(speedOutput, -1, SEEK_CUR);
  fprintf(speedOutput, "]"); fclose(speedOutput);
  return NULL;
}

bool is_prime(prime_type candidate) {
  bool has_denominators = false;
  for (int i = 0; i < prime_counter && !has_denominators; i++) {
    if (candidate % primes[i] == 0) has_denominators = true;
  }
  return !has_denominators;
}

prime_type get_primes(prime_type start, prime_type end) {
  #pragma omp parallel for ordered schedule(simd:runtime) shared(prime_counter)
  for (candidate = start; candidate <= end; candidate += 2) {
    if (is_prime(candidate)) {
      #pragma omp ordered
      {
        primes[prime_counter] = candidate;
        prime_counter++;
      }
    }
  }
  return end;
}

int main(int argc, char **args) 
{
  /* ctrl + C, terminate */
  signal(SIGINT, ctrlC); signal(SIGTERM, ctrlC);
  /* error handling */
  signal(SIGFPE, sigFPE); signal(SIGILL, sigILL); signal(SIGSEGV, sigSEGV);

  bool override = false;
  for (int i = 1; i < argc; i++) {
    if (strmatch(args[i], "--override")) override = true;
    if (args[i][0] != '-') max_primes = strtol(args[i], NULL, 10);
  }
  if (max_primes == 0) {
    /* preveri, koliko pomnilnika je na voljo */
    printf(_STRING_MEMORY_COUNTING); fflush(stdout);
    max_primes = get_avail_mem();
    max_primes -= max_primes / 10;
  }

  /* get available threads */
  int NUM_THREADS;
  #pragma omp parallel
  NUM_THREADS = omp_get_max_threads();

  /* announce available memory and n. threads */
  printf(_STRING_MEMORY_REMAINING, max_primes, prettify_size(max_primes * sizeof(prime_type)));

  /* grammar 😃 */
  printf(_STRING_THREADS_AVAILABLE, grammar(NUM_THREADS), NUM_THREADS);

  /* allocate available memory for primes */
  primes = malloc(max_primes * sizeof(prime_type));

  unsigned int written_counter; char answer;

  /* read primes from file */
  int error = 0;
  unsigned long time_combined = 0;
  FILE *f;
  printf(_STRING_FILE_OPENING); fflush(stdout);
  f = fopen(_FILENAME_PRIMES, "r");
  if (f != NULL && !override) { // if file exists and is available, not --override
    printf(_STRING_FILE_READING); fflush(stdout);
    prime_counter = 0;
    fseek(f, strlen("primes = [ "), SEEK_SET); // skip "primes = ["
    while (error != EOF) {
      error = fscanf(f, "%u, ", &primes[prime_counter]);
      prime_counter++;
    }
    fclose(f);
    printf(_STRING_FILE_READ); fflush(stdout);
    prime_counter -= 2; // W/o EOF and ']'
    written_counter = prime_counter;
    printf(_STRING_PRIME_NUMBER_OF, prime_counter, primes[prime_counter - 1]);
    f = fopen("results/.time", "r");
    if (f != NULL) error = fscanf(f, "%lu", &time_combined);
    printf(_STRING_TIME_PREVIOUS, d_h_m_s(time_combined));
    printf(_STRING_CONTINUE);
    answer = getchar();
    if (answer == 'n' || answer == 'N') exit(0);
  }
  else {
    fclose(f);
    if (!override) {
      printf(_STRING_FILE_CANNOT_OPEN);
      answer = getchar(); if (answer == 'n' || answer == 'N') exit(0);
    }
    primes[0] = 2;
    primes[1] = 3;
    prime_counter = 2;
    written_counter = 0;
  }

  printf(_STRING_THREADS_CREATING); fflush(stdout);

  /* initialize and start info thread */
  time_z = time(NULL) - time_combined;
  pthread_t status_screen;
  pthread_create(&status_screen, NULL, izpisi, NULL);

  /* HEART OF THE PROGRAM */
  /* calculate primes */
  prime_type start = primes[prime_counter - 1] + 2;
  prime_type end   = start + primes[prime_counter - 1] - 1;
  while (prime_counter < max_primes && !finish) {
    start = get_primes(start, end) + 2;
    end = start + primes[prime_counter - 1] - 1;
  }

  /* when calculation ends */
  unsigned long time_k = time(NULL);
  finished = true; pthread_join(status_screen, NULL);
  printf(_STRING_WRITING, primes[prime_counter - 1], prime_counter);
  fflush(stdout);
  /* write newly calculated primes as a javascript file to be opened by results/index.html */
  f = fopen(_FILENAME_PRIMES, "w");
  if (written_counter == 0) {
    fseek(f, 0, SEEK_END); fprintf(f, "primes = [ ");
  }
  else {
    fseek(f, -2, SEEK_END); fprintf(f, ",");
  }
  for (int i = written_counter; i < prime_counter; i++) {
    fprintf(f, "%d,", primes[i]);
  }
  /* free max_primes, write ending and close file */
  free(primes); fseek(f, -1, SEEK_END); fprintf(f, " ]"); fclose(f);
  /* write time to file */
  f = fopen("results/.time", "w");
  if(time_k - time_z > 0) fprintf(f, "\r%lu", time_k - time_z);
  fclose(f);
  printf(_STRING_FINISHED, d_h_m_s(time_k - time_z));
  return 0;
}
