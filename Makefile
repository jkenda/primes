CFLAGS = -O2 -std=c99 -pedantic -Wall
LIBS = -lpthread

hello:
	@echo "=== Prevajam Prastevila ==="
	$(CC) Prastevila.c $(CFLAGS) $(LIBS) -o Prastevila
	$(CC) Prastevila.c $(CFLAGS) $(LIBS) -S -o Prastevila.s

arm:
	@echo "Prevajam Prastevila za ARM na GNU/Linux"
	arm-linux-gnueabi-gcc Prastevila.c $(CFLAGS) $(LIBS) -o Prastevila-ARM
	arm-linux-gnueabi-gcc Prastevila.c $(CFLAGS) $(LIBS) -S -o Prastevila-ARM.s
