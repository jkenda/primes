CFLAGS = -O2 -std=c99 -pedantic -Wall
LIBS = -lpthread

hello:
	@echo "=== Prevajam Prastevila za trenutni sistem ==="
	$(CC) Prastevila.c $(CFLAGS) $(LIBS) -o Prastevila
	$(CC) Prastevila.c $(CFLAGS) $(LIBS) -S -o Prastevila.s

arm:
	@echo "=== Prevajam Prastevila za ARM na GNU/linux ==="
	arm-linux-gnueabi-gcc Prastevila.c $(CFLAGS) $(LIBS) -o Prastevila-ARM
	arm-linux-gnueabi-gcc Prastevila.c $(CFLAGS) $(LIBS) -S -o Prastevila-ARM.s

test:
	@echo "=== Prevajam ==="
	$(CC) Prastevila.c $(CFLAGS) $(LIBS) -o Prastevila
	$(CC) Prastevila.c $(CFLAGS) $(LIBS) -S -o Prastevila.s
	@echo "=== Testiram ==="
	./Prastevila 10000
	-diff Prastevila-izpis.txt TEST > /tmp/prastevilaDiff # minus je za ignoriranje napake
	if [ $? == "0" ]; then																# pogleda, ali sta datoteki enaki
		echo "Program deluje pravilno."
	else
		echo "Program NE deluje pravilno."
	fi
