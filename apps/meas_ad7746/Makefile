CFLAGS = -Wall -g0
LNKFLAGS = -lm
DEV = ../../dev

.PHONY: all
all: meas-dual meas-single

meas-dual: meas-dual.o $(DEV)/ad7746.o
	gcc $(LNKFLAGS) -o meas-dual meas-dual.o $(DEV)/ad7746.o

meas-single: meas-single.o $(DEV)/ad7746.o
	gcc $(LNKFLAGS) -o meas-single meas-single.o $(DEV)/ad7746.o

ad7746.o: $(DEV)/ad7746.c
	gcc ${CFLAGS} -c $(DEV)/ad7746.c

meas-dual.o: meas-dual.c
	gcc ${CFLAGS} -c meas-dual.c

meas-single.o: meas-single.c
	gcc ${CFLAGS} -c meas-single.c

.PHONY: clean
clean:
	@rm -f meas-dual.o
	@rm -f meas-single.o
	@rm -f $(DEV)/ad7746.o

.PHONY: maintainer-clean
maintainer-clean: clean
	@rm -f meas-dual
	@rm -f meas-single
