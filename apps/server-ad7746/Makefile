CFLAGS = -Wall -g0
LNKFLAGS = -lm
DEV = ../../dev

.PHONY: all
all: server-dual server-single

server-dual: server-dual.o $(DEV)/ad7746.o
	gcc -o server-dual server-dual.o $(DEV)/ad7746.o

server-single: server-single.o $(DEV)/ad7746.o
	gcc -o server-single server-single.o $(DEV)/ad7746.o

ad7746.o: $(DEV)/ad7746.c
	gcc ${CFLAGS} -c $(DEV)/ad7746.c

server-dual.o: server-dual.c
	gcc ${CFLAGS} -c server-dual.c

server-single.o: server-single.c
	gcc ${CFLAGS} -c server-single.c

.PHONY: clean
clean:
	@rm -f server-dual.o
	@rm -f server-single.o
	@rm -f $(DEV)/ad7746.o

.PHONY: maintainer-clean
maintainer-clean: clean
	@rm -f server-dual
	@rm -f server-single
