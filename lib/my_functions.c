/* Copyright (C) 2019 Jakob Kenda */

#include <sys/time.h>

void 
debug(int NUM_THREADS, unsigned int *prime_on_thread_counter, unsigned int **primes_on_thread)
{
	putchar('\n');
	unsigned int counter_max = 0;
	for (int i = 0; i < NUM_THREADS; i++) {
		printf("%4u ", prime_on_thread_counter[i]); 
		if (prime_on_thread_counter[i] > counter_max) 
			counter_max = prime_on_thread_counter[i];
	}
	putchar('\n'); 
	for (unsigned int i = 0; i < counter_max; i++) {
		for (int j = 0; j < NUM_THREADS; j++) {
			if (prime_on_thread_counter[j] > i) printf("%3u ", primes_on_thread[j][i]);
			else printf("     ");
		}
		putchar('\n'); 
	}
	printf("----------"); putchar('\n');
}

unsigned long 
get_avail_mem() 
{
	long total_memory;
	#ifdef _WIN32
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatusEx(&status);
		total_memory = status.ullTotalPhys;
	#else
		long pages = sysconf(_SC_PHYS_PAGES);
		long page_size = sysconf(_SC_PAGE_SIZE);
		total_memory = pages * page_size;
	#endif
		return total_memory;
}

char * 
prettify_size(unsigned long bytes) 
{
	static char velikost_enota[10];
	float bytesFloat = (float) bytes;
	char enota[3] = "B";
	if (bytesFloat >= 1000) {
		bytesFloat /= 1000; strcpy(enota, "kB");
	}
	if (bytesFloat >= 1000) {
		bytesFloat /= 1000; strcpy(enota, "MB");
	}
	if (bytesFloat >= 1000) {
		bytesFloat /= 1000; strcpy(enota, "GB");
	}
	sprintf(velikost_enota, "%.2f %s", bytesFloat, enota);
	return velikost_enota;
}

char * 
grammar(unsigned char stevilo) 
{
	static char beseda[4];
	#if LANGUAGE == sl
		if (stevilo == 2) sprintf(beseda, "sta");
		else if (stevilo == 3 || stevilo == 4) sprintf(beseda, "so");
		else sprintf(beseda, "je");
	#else
		if (stevilo == 1) sprintf(beseda, "is");
		else sprintf(beseda, "are");
	#endif
	return beseda;
}

char * 
d_h_m_s(int s) 
{
	static char cas[23];
	int d = s / 86400; s %= 86400;
	int h = s / 3600; s %= 3600;
	int m = s / 60; s %= 60;
	if (d > 0)      sprintf(cas, "%dd %dh %dm %ds", d, h, m, s);
	else if (h > 0) sprintf(cas, "%dh %dm %ds", h, m, s);
	else if (m > 0) sprintf(cas, "%dm %ds", m, s);
	else            sprintf(cas, "%ds", s);
	return cas;
}

bool 
strmatch(char* str1, char* str2) 
{
	return !strcmp(str1, str2);
}

int 
get_cpu_temperature() 
{
	static int temp;
	FILE* temp_input = fopen(_FILENAME_TEMP, "r");
	if (temp_input == NULL)
		return -1024;
	fscanf(temp_input, "%d", &temp); 
	fclose(temp_input);
	return temp / 1000;
}

unsigned int 
get_cpu_usage() 
{
	static double raw[4], idle_prev = 0, idle, total_prev = 0, total;
	static int usage;
	FILE *usage_input = fopen(_FILENAME_USAGE, "r"); 
	fseek(usage_input, 4, SEEK_CUR); 
	fscanf(usage_input, "%*s %lf %lf %lf %lf", &raw[0], &raw[1], &raw[2], &raw[3]);
	fclose(usage_input);
	idle  = raw[0] + raw[1] + raw[2];
	total = idle + raw[3];
	usage = (int) ((idle - idle_prev) / (total - total_prev) * 100);
	idle_prev = idle;
	total_prev = total;
	return (usage > 100 ? 100 : usage);
}

struct 
timespec time_nanoseconds()
{
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	return tv;
}

struct 
timespec* subtract_nanoseconds(struct timespec first, struct timespec second)
{
	static struct timespec subtracted;
	subtracted.tv_sec = first.tv_sec - second.tv_sec;
	subtracted.tv_nsec = first.tv_nsec - second.tv_nsec;
	return &subtracted;
}

float 
subtract_nanoseconds_float(struct timespec first, struct timespec second)
{
	return (first.tv_sec - second.tv_sec) 
	       + (first.tv_nsec - second.tv_nsec) / 1000000000;
}
