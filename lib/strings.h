/* Copyright (C) 2019 Jakob Kenda */

#if LANGUAGE == sl 
	#define _STRING_ERR_FPE  "\n🙁 Prišlo je do preliva ali pa deljenja z 0.\n"
	#define _STRING_ERR_ILL  "\n🙁 Neveljeven ukaz.\n"
	#define _STRING_ERR_SEGV "\n🙁 Program je poskusil dostopati do podatkov, ki niso njegovi.\n"

	#define _STRING_MEMORY_COUNTING   "Štejem, koliko spomina je na voljo."
	#define _STRING_MEMORY_REMAINING  "\rV pomnilniku je prostor za %u praštevil (%s).\n"
	#define _STRING_THREADS_AVAILABLE "Za računanje %s na voljo %d niti.\n"
	#define _STRING_FILE_OPENING      "Odpiram datoteko s praštevili... "
	#define _STRING_FILE_READING      "\b\b Berem datoteko... "
	#define _STRING_FILE_READ         "\rDatoteka prebrana.   \n"
	#define _STRING_PRIME_NUMBER_OF   "%u praštevil je že izračunanih (do %u). "
	#define _STRING_TIME_PREVIOUS     "Za prejšnje računanje ste potrebovali %s. "
	#define _STRING_CONTINUE          "Nadaljujem? [Y/n] "
	#define _STRING_FILE_CANNOT_OPEN  "Ne morem odpreti datoteke. Ustvarim novo? [Y/n] "
	#define _STRING_THREADS_CREATING  "\rUstvarjam niti..."
	#define _STRING_WRITING           "\rPraštevil do %u je %u. Zapisujem... \n"
	#define _STRING_FINISHED          "\rKončano 😀. Čas: %s\n"
#else
	#define _STRING_ERR_FPE  "\r🙁 Overflow or division by zero.\n"
	#define _STRING_ERR_ILL  "\r🙁 Illegal set.\n"
	#define _STRING_ERR_SEGV "\r🙁 Segmentation fault.\n"

	#define _STRING_MEMORY_COUNTING   "Enquiring about available memory.                       "
	#define _STRING_MEMORY_REMAINING  "\rThere is room for %u prime numbers in memory (%s).\n"
	#define _STRING_THREADS_AVAILABLE "There %s %d threads available.\n"
	#define _STRING_FILE_OPENING      "Opening file with prime numbers... "
	#define _STRING_FILE_READING      "\b\b Reading file... "
	#define _STRING_FILE_READ         "\bFile read.         \n"
	#define _STRING_PRIME_NUMBER_OF   "%d prime numbers have already been found (up to %d). "
	#define _STRING_TIME_PREVIOUS     "For your last calculation you needed %s. "
	#define _STRING_CONTINUE          "Continue? [Y/n] "
	#define _STRING_FILE_CANNOT_OPEN  "Cannot open file. Create new? [Y/n] "
	#define _STRING_THREADS_CREATING  "\rCreating threads..."
	#define _STRING_WRITING           "\rTo %d there are %d. Writing... \n"
	#define _STRING_FINISHED          "\rFinished 😀. Time: %s\n"
#endif

/* ANSI color escape sequences */

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
