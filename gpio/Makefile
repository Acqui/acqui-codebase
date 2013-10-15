CFLAGS = -Wall -g0
LNKFLAGS = -lm

.PHONY: all
all: gpio

gpio: gpio.o 
	gcc $(LNKFLAGS) -o gpio gpio.o

gpio.o: gpio.c
	gcc ${CFLAGS} -c gpio.c

.PHONY: clean
clean:
	@rm -f gpio.o
	@rm -f gpio
