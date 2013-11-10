CFLAGS = -Wall -g0
LNKFLAGS = -lm
DEV = ../../dev
SYS = ../../sys

.PHONY: all
all: meas-ltc2493

meas-ltc2493: meas-ltc2493.o $(SYS)/gpio.o ${DEV}/ltc2493.o
	gcc $(LNKFLAGS) -o meas-ltc2493 meas-ltc2493.o $(SYS)/gpio.o ${DEV}/ltc2493.o

gpio.o: $(SYS)/gpio.c
	gcc $(CFLAGS) -c $(SYS)/gpio.c

ltc2493.o: ltc2493.c
	gcc ${CFLAGS} -c ltc2493.c

meas-ltc2493.o: meas-ltc2493.c
	gcc ${CFLAGS} -c meas-ltc2493.c

.PHONY: clean
clean:
	@rm -f meas-ltc2493.o
	@rm -f $(SYS)/gpio.o
	@rm -f ${DEV}/ltc2493.o

.PHONY: maintainer-clean
maintainer-clean: clean
	@rm -f meas-ltc2493
