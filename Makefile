# Copyright (C) 2019 Jakob Kenda

CFLAGS = -O2 -std=c99 -pedantic -Wall
LIBS = -lpthread -fopenmp -lm
OMP_CANCELLATION = true
LANGUAGE = "en"
DEBUG = false

hello:
	@echo "Language: $(LANGUAGE)"
	@echo "=== Compiling Prastevila.c for current system ==="
	@$(CC) Prastevila.c -D LANGUAGE=$(LANGUAGE) -D DEBUG=$(DEBUG) $(CFLAGS) $(LIBS) -o bin/primes
	@$(CC) Prastevila.c -D LANGUAGE=$(LANGUAGE) -D DEBUG=$(DEBUG) $(CFLAGS) $(LIBS) -S -o assembly/primes.s
	@echo "=== Done ==="

arm:
	@echo "Language: $(LANGUAGE)"
	@echo "=== Compiling Prastevila.c for GNU/linux on ARM ==="
	@arm-linux-gnueabi-gcc Prastevila.c -D LANGUAGE=$(LANGUAGE) -D DEBUG=$(DEBUG) $(CFLAGS) $(LIBS) -o bin/primes-ARM
	@arm-linux-gnueabi-gcc Prastevila.c -D LANGUAGE=$(LANGUAGE) -D DEBUG=$(DEBUG) $(CFLAGS) $(LIBS) -S -o assembly/primes-ARM.s
	@echo "=== Done ==="

test:
	@echo "===  Compiling ==="
	@$(CC) Prastevila.c -D LANGUAGE=$(LANGUAGE) $(CFLAGS) $(LIBS) -o bin/primes
	@$(CC) Prastevila.c -D LANGUAGE=$(LANGUAGE) $(CFLAGS) $(LIBS) -S -o assembly/primes.s
	@echo "===  Compiled  ==="
	@echo "===  Testing   ==="
	@echo "===  Calculating 10.000 primes   ===\n"
	@bin/primes 10000 --override
	@echo "\n===      Comparing results       ==="
	@-diff results/primes.js results/TEST > results/error.diff # primerjaj prvih 10.000 praštevil s pravilno datoteko
	@case "$?" in \
		[0]) echo "===      PROGRAM WORKS FINE      ===" && rm results/error.diff && exit 0;; \
		*  ) echo "=== PROGRAM GIVES WRONG RESULT!  ===\nDifferences can be found in results/error.diff" && exit 1;; \
	esac
