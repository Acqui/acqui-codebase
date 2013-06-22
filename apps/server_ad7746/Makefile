CFLAGS = -Wall -g0
LNKFLAGS = -lm
DEV = ../../dev

.PHONY: all
all: server

server: server.o $(DEV)/ad7746.o
	gcc -o server server.o $(DEV)/ad7746.o

ad7745.o: $(DEV)/ad7746.c
	gcc ${CFLAGS} -c $(DEV)/ad7746.c

server.o: server.c
	gcc ${CFLAGS} -c server.c

.PHONY: clean
clean:
	@rm -f server.o
	@rm -f $(DEV)/ad7746.o

.PHONY: maintainer-clean
maintainer-clean: clean
	@rm -f server
