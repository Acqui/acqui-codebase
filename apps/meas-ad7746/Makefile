CFLAGS = -Wall -g0
LNKFLAGS = -lm
DEV = ../../dev

.PHONY: all
all: meas-ad7746

meas-ad7746: meas-ad7746.o $(DEV)/ad7746.o
	gcc $(LNKFLAGS) -o meas-ad7746 meas-ad7746.o $(DEV)/ad7746.o

ad7746.o: $(DEV)/ad7746.c
	gcc ${CFLAGS} -c $(DEV)/ad7746.c

meas-ad7746.o: meas-ad7746.c
	gcc ${CFLAGS} -c meas-ad7746.c

.PHONY: clean
clean:
	@rm -f meas-ad7746.o
	@rm -f $(DEV)/ad7746.o

.PHONY: maintainer-clean
maintainer-clean: clean
	@rm -f meas-ad7746
