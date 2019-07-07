/* Copyright (C) 2019 Jakob Kenda */

#pragma GCC diagnostic ignored "-Wunused-result"

#include "my_functions.h"     // translation

bool exit_flag = false;
prime_type candidate, *primes, **primes_on_thread;
unsigned int prime_counter, *prime_on_thread_counter, max_primes = 0, *max_primes_on_thread;
unsigned long primes_memory, primes_on_thread_memory, time_z;

void 
sigFPE() 
{
	fprintf(stderr, _STRING_ERR_FPE);
	exit_flag = true;
}

void 
sigILL() 
{
	fprintf(stderr, _STRING_ERR_ILL);
	exit_flag = true;
}

void 
sigSEGV() 
{
	fprintf(stderr, _STRING_ERR_SEGV);
	exit_flag = true;
}

void 
ctrlC() 
{
	exit_flag = true;
}

/* info screen thread */
void *
izpisi() 
{ 
	/* file pointers */
	FILE *speed_output;

	struct timespec loop_start, time_speed;
	unsigned int pc_local, pc_local_old, pc_estimate, speed, time_curr;
	pc_local = 2; pc_local_old = 0; pc_estimate = 2; 
	speed = 0; time_speed = time_nanoseconds();
	speed_output = fopen(_FILENAME_SPEED, "w"); fprintf(speed_output, "speed = [");
	while (!exit_flag) 
	{
		loop_start = time_nanoseconds(); loop_start.tv_sec -= 1;
		pc_local_old = pc_local;
		pc_local = prime_counter; 
		if (pc_local_old != pc_local) 
		{
			speed = (pc_local - pc_local_old) 
			        / subtract_nanoseconds_float(time_nanoseconds(), time_speed); 
			time_speed = time_nanoseconds();
		}
		else
		{
			pc_estimate = pc_local + subtract_nanoseconds_float(time_nanoseconds(), time_speed) * speed;
		}
		time_curr = time(NULL) - time_z;

		if (pc_estimate > pc_local)
			printf("\r%u (%u / 1000), %s [%u/s], CPU: %d °C, %u %% (estimated)", 
				pc_estimate, 
				(int) (pc_estimate * 1000 / max_primes), 
				d_h_m_s(time_curr), speed, get_cpu_temperature(), get_cpu_usage()); 
		else
			printf("\r%u (%u / 1000), %s [%u/s], CPU: %d °C, %u %%            ", 
				pc_local, 
				(int) (pc_local * 1000 / max_primes), 
				d_h_m_s(time_curr), speed, get_cpu_temperature(), get_cpu_usage()); 
		fflush(stdout);
		if (pc_local_old != pc_local) 
			fprintf(speed_output, "{x:%u,y:%u},", time_curr, speed);
		nanosleep(subtract_nanoseconds(time_nanoseconds(), loop_start), NULL);
	}
	fprintf(speed_output, "{x:%u,y:%u},", time_curr, speed);
	if (pc_local != 0) 
		fseek(speed_output, -1, SEEK_CUR);
	fprintf(speed_output, "]"); 
	fclose(speed_output);
	return NULL;
}

bool 
is_prime(prime_type candidate) 
{
	bool has_denominators = false;
	for (unsigned int i = 0; !has_denominators && i < prime_counter; i++) 
	{
		if (candidate % primes[i] == 0) 
			has_denominators = true;
	}
	return !has_denominators;
}

prime_type 
get_primes(prime_type start, prime_type end) 
{
	unsigned int local_prime_counter;
	//#pragma omp atomic read
	local_prime_counter = prime_counter;

	#pragma omp target
	#pragma omp teams distribute parallel for 
	for (candidate = start; candidate <= end; candidate += 2) 
	{
		if (local_prime_counter + prime_on_thread_counter[this_thread()] >= max_primes)
			exit_flag = true;
		if (exit_flag) 
			continue;
		if (is_prime(candidate)) 
		{
			/*
			if (prime_on_thread_counter[this_thread()] >= max_primes_on_thread[this_thread()])
			{
				max_primes_on_thread[this_thread()]++;
				realloc(primes_on_thread[this_thread()], max_primes_on_thread[this_thread()]);
			}
			*/
			primes_on_thread[omp_get_thread_num()]
			                [prime_on_thread_counter[omp_get_thread_num()]]  = candidate;
			prime_on_thread_counter[omp_get_thread_num()]++;
		}
	}
	return end;
}

void 
insert_primes_from_threads(int NUM_THREADS) 
{
	//#pragma omp for ordered schedule(simd:static)
	for (int i = 0; i < NUM_THREADS; i++) 
	{
		//#pragma omp ordered
		/*
		if (prime_counter + prime_on_thread_counter[i] >= max_primes)
			realloc(primes, (max_primes_on_thread[i] + prime_on_thread_counter[i]) * sizeof(prime_type));
		*/
		for (unsigned int j = 0; j < prime_on_thread_counter[i]; j++) 
		{
			primes[prime_counter + j] = primes_on_thread[i][j];
		}
		//#pragma omp atomic update
		prime_counter += prime_on_thread_counter[i];
		prime_on_thread_counter[i] = 0;
	}
}

int 
main(int argc, char **args) 
{
	/* ctrl + C, terminate */
	signal(SIGINT, ctrlC); signal(SIGTERM, ctrlC);
	/* error handling */
	signal(SIGFPE, sigFPE); signal(SIGILL, sigILL); signal(SIGSEGV, sigSEGV);

	/* get available threads */
	int NUM_THREADS, NUM_DEVICES;
	#pragma omp parallel
	NUM_THREADS = omp_get_max_threads();
	NUM_DEVICES = omp_get_num_devices();

	printf(_STRING_MEMORY_COUNTING); fflush(stdout);
	unsigned long max_memory = get_avail_mem(); max_memory -= max_memory / 10;

	bool override = false;
	bool use_max_memory = true;
	for (int i = 1; i < argc; i++) 
	{
		if (isdigit(args[i][0])) 
		{
			use_max_memory = false;
			max_primes = strtol(args[i], NULL, 10); // (-1)
		}
		else 
		{
			if (strmatch(args[i], "--override")) 
				override = true;
		}
	}
	
	max_primes_on_thread = malloc(NUM_THREADS * sizeof(unsigned int*));
	if (!use_max_memory)
	{		
		primes_memory = max_primes * 2 * sizeof(prime_type);
		primes_on_thread_memory = max_primes * 2 * sizeof(unsigned int) / NUM_THREADS;
	}
	else
	{
		primes_memory = max_memory * 0.66;
		primes_on_thread_memory = (max_memory - primes_memory) / NUM_THREADS;

		max_primes = primes_memory / sizeof(prime_type);
	}

	for (int i = 0; i < NUM_THREADS; i++)
		max_primes_on_thread[i] = primes_on_thread_memory / sizeof(prime_type);

	/* announce available memory and n. threads */
	printf(_STRING_MEMORY_AVAILABLE, max_memory / sizeof(prime_type), prettify_size(max_memory),
	                                 max_primes, 
	                                 prettify_size(primes_memory));

	/* grammar 😃 */
	printf(_STRING_THREADS_AVAILABLE, grammar(NUM_THREADS), NUM_THREADS, NUM_DEVICES);

	/* allocate available memory for primes */
	primes = malloc(primes_memory - (primes_memory % sizeof(prime_type)));
	primes_on_thread = malloc(NUM_THREADS * sizeof(prime_type*));
	prime_on_thread_counter = malloc(NUM_THREADS * sizeof(unsigned int*));
	for (int i = 0; i < NUM_THREADS; i++) 
	{
		primes_on_thread[i] = malloc(primes_on_thread_memory);
		prime_on_thread_counter[i] = 0;
	}

	unsigned int written_counter; 
	char answer;

	/* read primes from file */
	unsigned long time_combined = 0;
	FILE *f;
	if (!override) // override argument is not present
	{
		printf(_STRING_FILE_OPENING); fflush(stdout);
		f = fopen(_FILENAME_PRIMES, "r");
		if (f != NULL) // file exists
		{
			printf(_STRING_FILE_READING); fflush(stdout);
			fseek(f, strlen("primes = ["), SEEK_SET); // skip "primes = ["

			prime_counter = 0;
			primes[prime_counter] = 0;
			int c;
			while ((c = fgetc(f)) != EOF)
			{
				if (isdigit(c))
					primes[prime_counter] = 10 * primes[prime_counter] + (c - 48);
				else if (c == ',')
				{
					prime_counter++;
					primes[prime_counter] = 0;
				}
				else if (c == ']')
					prime_counter++;
			}
			fclose(f);
			printf(_STRING_FILE_READ);
			written_counter = prime_counter;
			printf(_STRING_PRIME_NUMBER_OF, prime_counter, primes[prime_counter - 1]);
		}
		else
		{
			fclose(f);
			printf(_STRING_FILE_CANNOT_OPEN);
			primes[0] = 2;
			primes[1] = 3;
			prime_counter = 2;
			written_counter = 0;
		}
	}
	else 
	{
		primes[0] = 2;
		primes[1] = 3;
		prime_counter = 2;
		written_counter = 0;
	}

	printf(_STRING_CONTINUE);
	answer = getchar();
	if (answer == 'n' || answer == 'N') 
		exit(0);

	printf(_STRING_THREADS_CREATING); fflush(stdout);

	/* initialize and start info thread */
	time_z = time(NULL) - time_combined;
	pthread_t status_screen;
	pthread_create(&status_screen, NULL, izpisi, &NUM_THREADS);

	/* HEART OF THE PROGRAM */
	/* calculate primes */
	prime_type start = primes[prime_counter - 1] + 2;
	prime_type end   = start + primes[prime_counter - 1] - 1;

	while (prime_counter < max_primes && !exit_flag) 
	{
		start = get_primes(start, end) + 2;
		end = start + primes[prime_counter - 1] - 1;

		#if DEBUG
		debug(NUM_THREADS, &prime_on_thread_counter[0], &primes_on_thread[0]);
		#endif

		insert_primes_from_threads(NUM_THREADS);
	}

	/* when calculation ends */
	unsigned long time_k = time(NULL);
	exit_flag = true; 
	pthread_join(status_screen, NULL);
	
	if (prime_counter > max_primes) 
		prime_counter = max_primes;
	
	printf(_STRING_WRITING, primes[prime_counter - 1], prime_counter);
	fflush(stdout);

	/* write newly calculated primes as a javascript file to be opened by results/index.html */
	if (written_counter == 0) 
	{
		f = fopen(_FILENAME_PRIMES, "w");
		fprintf(f, "primes = [");
	}
	else 
	{
		f = fopen(_FILENAME_PRIMES, "r+");
		fseek(f, -1L, SEEK_END); 
		fprintf(f, ",");
	}

	/* write ending and close file */
	for (unsigned int i = written_counter; i < prime_counter; i++) 
		fprintf(f, "%u,", primes[i]);
	fseek(f, -1L, SEEK_END); 
	fprintf(f, "]"); 
	fclose(f);

	/* free primes, primes_on_thread and primes_on_thread_counter */
	free(primes); 
	for (int i = 0; i < NUM_THREADS; i++) 
		free(primes_on_thread[i]);
	free(primes_on_thread);
	free(prime_on_thread_counter);

	printf(_STRING_FINISHED, d_h_m_s(time_k - time_z));
	return errno;
}
