# Copyright (C) 2019 Jakob Kenda

CFLAGS = -g -O3 -mtune=native -std=c99 -pedantic -Wall -Wextra
LIBS = -lpthread -fopenmp -lm
OMP_CANCELLATION = true
LANGUAGE = "en"
DEBUG = false

hello:
	@echo "Language: $(LANGUAGE)"
	@echo "=== Compiling main.c for current system ==="
	@$(CC) main.c -D LANGUAGE=$(LANGUAGE) -D DEBUG=$(DEBUG) $(CFLAGS) $(LIBS) -o bin/primes
	@$(CC) main.c -D LANGUAGE=$(LANGUAGE) -D DEBUG=$(DEBUG) $(CFLAGS) $(LIBS) -S -o assembler/primes.s
	@echo "=== Done ==="

arm:
	@echo "Language: $(LANGUAGE)"
	@echo "=== Compiling main.c for GNU/linux on ARM ==="
	@arm-linux-gnueabi-gcc main.c -D LANGUAGE=$(LANGUAGE) -D DEBUG=$(DEBUG) $(CFLAGS) $(LIBS) -o bin/primes-ARM
	@arm-linux-gnueabi-gcc main.c -D LANGUAGE=$(LANGUAGE) -D DEBUG=$(DEBUG) $(CFLAGS) $(LIBS) -S -o assembler/primes-ARM.s
	@echo "=== Done ==="

test:
	@echo "===  Compiling ==="
	@$(CC) main.c -D LANGUAGE=$(LANGUAGE) $(CFLAGS) $(LIBS) -o bin/primes
	@$(CC) main.c -D LANGUAGE=$(LANGUAGE) $(CFLAGS) $(LIBS) -S -o assembler/primes.s
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
