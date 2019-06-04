/* Copyright (C) 2019 Jakob Kenda */

#pragma GCC diagnostic ignored "-Wunused-result"

#include <stdio.h>       // I/0 (reading and writing on screen / to file)
#include <stdlib.h>      // malloc (allocating memory)
#include <time.h>        // measuring time consumption
#include <math.h>        // log2()
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

typedef unsigned int prime_type;
typedef unsigned char byte;

bool finish = false;
prime_type candidate, *primes, **primes_on_thread;
unsigned int prime_counter, *prime_on_thread_counter, max_primes = 0;
unsigned long time_z;

void sigFPE() 
{
  fprintf(stderr, _STRING_ERR_FPE);
  finish = true;
}

void sigILL() 
{
  fprintf(stderr, _STRING_ERR_ILL);
  finish = true;
}

void sigSEGV() 
{
  fprintf(stderr, _STRING_ERR_SEGV);
  finish = true;
}

void ctrlC() 
{
  finish = true;
}

/* info screen thread */
void* izpisi(void* param) 
{ 
  /* file pointers */
  FILE *speedOutput;

  struct timespec loop_start;
  int prime_counter_local, prime_counter_local_old, speed, time_curr;
  prime_counter_local = 0; prime_counter_local_old = 2; speed = 0;
  speedOutput = fopen(_FILENAME_SPEED, "w"); fprintf(speedOutput, "speed = [");
  while (!finish) {
    loop_start = time_nanoseconds(); loop_start.tv_sec -= 1;
    if (prime_counter_local_old != prime_counter_local) prime_counter_local_old = prime_counter_local;
    prime_counter_local = prime_counter; 
    if (prime_counter_local_old != prime_counter_local) speed = prime_counter_local - prime_counter_local_old; 
    time_curr = time(NULL) - time_z;

    printf("\r%u (%u / 1000), %s [%d/s], CPU: %d °C, %u %% ", 
      prime_counter_local, (int) (prime_counter_local * 1000 / max_primes), 
      d_h_m_s(time_curr), speed, get_cpu_temperature(), get_cpu_usage()); 
    fflush(stdout);
    if (prime_counter_local_old != prime_counter_local) fprintf(speedOutput, "{x:%d,y:%d},", time_curr, speed);
    nanosleep(subtract_nanoseconds(time_nanoseconds(), loop_start), NULL);
  }
  time_curr = time(NULL) - time_z;
  fprintf(speedOutput, "{x:%d,y:%d},", time_curr, speed);
  if (prime_counter_local != 0) fseek(speedOutput, -1, SEEK_CUR);
  fprintf(speedOutput, "]"); fclose(speedOutput);
  return NULL;
}

bool is_prime(prime_type candidate) 
{
  bool has_denominators = false;
  for (int i = 0; i < prime_counter && !has_denominators; i++) {
    if (candidate % primes[i] == 0) has_denominators = true;
  }
  return !has_denominators;
}

prime_type get_primes(prime_type start, prime_type end) 
{
  #pragma omp for ordered schedule(simd:static)
  for (candidate = start; candidate <= end; candidate += 2) {
    int local_prime_counter;
    #pragma omp atomic read
    local_prime_counter = prime_counter;
    if (local_prime_counter >= max_primes || finish) continue;
    if (is_prime(candidate)) {
      primes_on_thread[omp_get_thread_num()][prime_on_thread_counter[omp_get_thread_num()]] = candidate;
      prime_on_thread_counter[omp_get_thread_num()]++;
    }
  }
  return end;
}

void insert_primes_from_threads(int NUM_THREADS) 
{
  #pragma omp for ordered schedule(simd:static)
  for (int i = 0; i < NUM_THREADS; i++) {
    #pragma omp ordered
    for (int j = 0; j < prime_on_thread_counter[i]; j++) {
      primes[prime_counter + j] = primes_on_thread[i][j];
    }
    #pragma omp atomic update
    prime_counter += prime_on_thread_counter[i];
    prime_on_thread_counter[i] = 0;
  }
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
    if (args[i][0] != '-') max_primes = strtol(args[i], NULL, 10); // (-1)
  }
  if (!max_primes) {
    /* preveri, koliko pomnilnika je na voljo */
    printf(_STRING_MEMORY_COUNTING); fflush(stdout);
    max_primes = get_avail_mem();
    max_primes = max_primes * 0.66 - max_primes / 10;
  }

  unsigned int max_memory = get_avail_mem();

  /* get available threads */
  int NUM_THREADS;
  #pragma omp parallel
  NUM_THREADS = omp_get_max_threads();

  /* announce available memory and n. threads */
  printf(_STRING_MEMORY_REMAINING, max_primes, prettify_size(max_primes * sizeof(prime_type)));

  /* grammar 😃 */
  printf(_STRING_THREADS_AVAILABLE, grammar(NUM_THREADS), NUM_THREADS);

  /* allocate available memory for primes */
  primes = malloc(max_memory * sizeof(prime_type));
  primes_on_thread = malloc(NUM_THREADS * sizeof(prime_type*));
  prime_on_thread_counter = malloc(NUM_THREADS * sizeof(unsigned int*));
  for (int i = 0; i < NUM_THREADS; i++) {
    primes_on_thread[i] = malloc(max_memory / 2 / NUM_THREADS * sizeof(prime_type));
    prime_on_thread_counter[i] = 0;
  }

  unsigned int written_counter; char answer;

  /* read primes from file */
  int error = 0;
  unsigned long time_combined = 0;
  FILE *f;
  if (!override) {
    printf(_STRING_FILE_OPENING); fflush(stdout);
    f = fopen(_FILENAME_PRIMES, "r");
  }
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
    if (!override) {
      fclose(f);
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
  pthread_create(&status_screen, NULL, izpisi, &NUM_THREADS);

  /* HEART OF THE PROGRAM */
  /* calculate primes */
  prime_type start = primes[prime_counter - 1] + 2;
  prime_type end   = start + primes[prime_counter - 1] - 1;

  #pragma omp parallel shared(prime_counter)
  while (prime_counter < max_primes && !finish) {
    start = get_primes(start, end) + 2;
    end = start + primes[prime_counter - 1] - 1;

    #if DEBUG
    debug(NUM_THREADS, &prime_on_thread_counter[0], &primes_on_thread[0]);
    #endif

    insert_primes_from_threads(NUM_THREADS);
  }

  /* when calculation ends */
  unsigned long time_k = time(NULL);
  finish = true; pthread_join(status_screen, NULL);
  if (prime_counter > max_primes) prime_counter = max_primes;
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

  /* write ending and close file */
  for (int i = written_counter; i < prime_counter; i++) {
    fprintf(f, "%d,", primes[i]);
  }
  fseek(f, -1, SEEK_END); fprintf(f, " ]"); fclose(f);


  /* free primes, primes_on_thread and primes_on_thread_counter */
  free(primes); 
  for (int i = 0; i < NUM_THREADS; i++) {
    free(primes_on_thread[i]);
  }
  free(primes_on_thread);
  free(prime_on_thread_counter);

  /* write time to file */
  f = fopen("results/.time", "w");
  if(time_k - time_z > 0) fprintf(f, "%lu", time_k - time_z); 
  fclose(f);
  printf(_STRING_FINISHED, d_h_m_s(time_k - time_z));
  return 0;
}
