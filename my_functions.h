/* Copyright (C) 2019 Jakob Kenda */

#ifndef LANGUAGE
	#define LANGUAGE en
#endif

#include <stdio.h>       // I/0 (reading and writing on screen / to file)
#include <stdlib.h>      // malloc (allocating memory)
#include <ctype.h>       // idigit()
#include <time.h>        // measuring time consumption
#include <sys/time.h>    // getting time in nanoseconds
#include <math.h>        // log2()
#include <pthread.h>     // thread for info screen
#ifdef _OPENMP /* if OpenMP library is provided */
	#include <omp.h>     // multithreading
#else          /* if OpenMP library is not provided */
	#define omp_get_max_threads() 1
	#define omp_get_thread_num() 0
#endif
#ifdef _WIN32 /* if it is compiled for Windows */
	#include <windows.h> // damn windows needs its own library
#endif
#include <unistd.h>      // sleep() and getting available memory
#include <signal.h>      // signal handling (ctrl+C, exceptions)
#include <stdbool.h>     // boolean library
#include <string.h>      // string library

#include "strings.h"      // translation

typedef unsigned int prime_type;
typedef unsigned char byte;


#define _FILENAME_USAGE "/proc/stat"
#define _FILENAME_TEMP  "/sys/bus/platform/devices/coretemp.0/hwmon/hwmon1/temp1_input"
#define _FILENAME_PRIMES "results/primes.js"
#define _FILENAME_SPEED  "results/speed.js"

/* just to make it more readable */
#define this_thread() omp_get_thread_num()
/* returns available memory */
unsigned long 
get_avail_mem();
/* returns kB, MB, GB from B given */
char *
prettify_size(unsigned long bytes);
/* returns "is" or "are" according to a number given */
char *
grammar(unsigned char stevilo);
/* returns number of days, hours minutes and seconds from seconds given */
char *
d_h_m_s(int s);
/* returns true if strings match */
bool 
strmatch(char* str1, char* str2);
/* returns CPU temperature */
int 
get_cpu_temp();
/* returns CPU usage */
unsigned int 
get_cpu_usage();
/* returns struct that contains seconds and nanoseconds since epoch */
struct timespec 
time_nanoseconds();
/* returns pointer to a difference of first and second time */
struct timespec * 
subtract_nanoseconds(struct timespec first, struct timespec second);
/* writes primes to file */

#include "lib/my_functions.c"
