/* Copyright (C) 2019 Jakob Kenda */

#if LANGUAGE == sl 
	#define STRING_ERR_FPE  "\r🙁 Prišlo je do preliva ali pa deljenja z 0.\n"
	#define STRING_ERR_ILL  "\r🙁 Neveljeven ukaz.\n"
	#define STRING_ERR_SEGV "\r🙁 Program je poskusil dostopati do podatkov, ki niso njegovi.\n"

	#define STRING_MEMORY_COUNTING   "Štejem, koliko spomina je na voljo."
	#define STRING_MEMORY_REMAINING  "\rV pomnilniku je prostor za %ld praštevil (%s).\n"
	#define STRING_THREADS_AVAILABLE "Za računanje %s na voljo %d niti.\n"
	#define STRING_FILE_OPENING      "Odpiram datoteko s praštevili ... "
	#define STRING_FILE_READING      "\b\b\b\b. Berem datoteko ... "
	#define STRING_FILE_READ         "\rDatoteka prebrana.\n"
	#define STRING_PRIME_NUMBER_OF   "%d praštevil je že izračunanih (do %d). "
	#define STRING_TIME_PREVIOUS     "Za prejšnje računanje ste potrebovali %s. "
	#define STRING_CONTINUE          "Nadaljujem? [Y/n] "
	#define STRING_FILE_CANNOT_OPEN  "Ne morem odpreti datoteke. Ustvarim novo? [Y/n] "
	#define STRING_THREADS_CREATING  "\rUstvarjam niti ..."
	#define STRING_WRITING           "\rPraštevil do %d je %d. Zapisujem ... \n"
	#define STRING_FINISHED          "\rKončano 😀. Čas: %s\n"
#else
	#define STRING_ERR_FPE  "\r🙁 Overflow or division by zero.\n"
	#define STRING_ERR_ILL  "\r🙁 Illegal set.\n"
	#define STRING_ERR_SEGV "\r🙁 Segmentation fault.\n"

	#define STRING_MEMORY_COUNTING   "Enquiring about available memory.                       "
	#define STRING_MEMORY_REMAINING  "\rThere is room for %ld prime numbers in memory (%s).\n"
	#define STRING_THREADS_AVAILABLE "There %s %d threads available.\n"
	#define STRING_FILE_OPENING      "Opening file with prime numbers ... "
	#define STRING_FILE_READING      "\b\b\b\b. Reading file ... "
	#define STRING_FILE_READ         "\bFile read.\n"
	#define STRING_PRIME_NUMBER_OF   "%d prime numbers have already been found (up to %d). "
	#define STRING_TIME_PREVIOUS     "For your last calculation you needed %s. "
	#define STRING_CONTINUE          "Continue? [Y/n] "
	#define STRING_FILE_CANNOT_OPEN  "Cannot open file. Create new? [Y/n] "
	#define STRING_THREADS_CREATING  "\rCreating threads ..."
	#define STRING_WRITING           "\rTo %d there are %d. Writing ... \n"
	#define STRING_FINISHED          "\rFinished 😀. Time: %s\n"
#endif