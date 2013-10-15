CFLAGS = -Wall -g0
LNKFLAGS = -lm

.PHONY: all
all: ltc2493

ltc2493: main.o ltc2493.o 
	gcc $(LNKFLAGS) -o ltc2493 ltc2493.o main.o

main.o: main.c
	gcc ${CFLAGS} -c main.c

ltc2493.o: ltc2493.c
	gcc ${CFLAGS} -c ltc2493.c


.PHONY: clean
clean:
	@rm -f ltc2493.o
	@rm -f main.o
	@rm -f ltc2493
