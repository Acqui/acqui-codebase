CFLAGS = -Wall -g0
LNKFLAGS = -lm
DEV = ../../dev

.PHONY: all
all: meas_ltc2493

meas_ltc2493: meas_ltc2493.o ${DEV}/ltc2493.o 
	gcc $(LNKFLAGS) -o meas_ltc2493 meas_ltc2493.o ${DEV}/ltc2493.o

meas_ltc2493.o: meas_ltc2493.c
	gcc ${CFLAGS} -c meas_ltc2493.c

ltc2493.o: ltc2493.c
	gcc ${CFLAGS} -c ltc2493.c

.PHONY: clean
clean:
	@rm -f meas_ltc2493.o
	@rm -f ${DEV}/ltc2493.o

.PHONY: maintainer-clean
maintainer-clean: clean
	@rm -f meas_ltc2493
