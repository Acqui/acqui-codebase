CFLAGS = -Wall -g0
LNKFLAGS = -lm
DEV = ../../dev

.PHONY: all
all: meas

meas: meas.o $(DEV)/ad7746.o
	gcc $(LNKFLAGS) -o meas meas.o $(DEV)/ad7746.o

server: server.o ad7746.o
	gcc -o server server.o $(DEV)/ad7746.o

ad7745.o: $(DEV)/ad7746.c
	gcc ${CFLAGS} -c $(DEV)/ad7746.c

meas.o: meas.c
	gcc ${CFLAGS} -c meas.c

.PHONY: clean
clean:
	@rm -f meas.o
	@rm -f $(DEV)/ad7746.o

.PHONY: maintainer-clean
maintainer-clean: clean
	@rm -f meas
