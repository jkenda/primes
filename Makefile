# Copyright (C) 2019 Jakob Kenda

CFLAGS = -O2 -std=c99 -pedantic -Wall
LIBS = -lpthread -fopenmp -lm
OMP_CANCELLATION = true
LANGUAGE = "en"

hello:
	@echo "Language: $(LANGUAGE)"
	@echo "=== Compiling Prastevila for current system ==="
	@$(CC) Prastevila.c -D LANGUAGE=$(LANGUAGE) $(CFLAGS) $(LIBS) -o bin/primes
	@$(CC) Prastevila.c -D LANGUAGE=$(LANGUAGE) $(CFLAGS) $(LIBS) -S -o assembly/primes.s

arm:
	@echo "Language: $(LANGUAGE)"
	@echo "=== Compiling Prastevila for GNU/linux on ARM ==="
	@arm-linux-gnueabi-gcc Prastevila.c -D LANGUAGE=$(LANGUAGE) $(CFLAGS) $(LIBS) -o bin/primes-ARM
	@arm-linux-gnueabi-gcc Prastevila.c -D LANGUAGE=$(LANGUAGE) $(CFLAGS) $(LIBS) -S -o assembly/primes-ARM.s

test:
	@echo "=== Compiling ==="
	@$(CC) Prastevila.c $(CFLAGS) $(LIBS) -o bin/primes
	@$(CC) Prastevila.c $(CFLAGS) $(LIBS) -S -o assembly/primes.s
	@echo "===  Testing  ==="
	@echo "Calculating 10.000 primes ..."
	@bin/primes 10000 --override > /dev/null 
	@echo "Comparing ..."
	@-diff results/primes.js results/TEST # primerjaj prvih 10.000 praštevil s pravilno datoteko
	@if [ $$? -eq "0" ]; then \
		echo "PROGRAM WORKS FINE."; \
		else echo "PROGRAM GIVES WRONG RESULT!"; \
	fi
