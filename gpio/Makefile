CFLAGS = -Wall -g0
LNKFLAGS = -lm

.PHONY: all
all: gpio

gpio: gpio.o main.o 
	gcc $(LNKFLAGS) -o gpio gpio.o main.o

gpio.o: gpio.c
	gcc ${CFLAGS} -c gpio.c

main.o: main.c
	gcc ${CFLAGS} -c main.c

.PHONY: clean
clean:
	@rm -f main.o
	@rm -f gpio.o
	@rm -f gpio
