CFLAGS = -Wall -g0
LNKFLAGS = -lm
DEV = ../../dev
SYS = ../../sys

.PHONY: all
all: server

server: server.o $(SYS)/gpio.o $(SYS)/net.o $(DEV)/ad7746.o
	gcc -o server server.o $(SYS)/net.o $(SYS)/gpio.o $(DEV)/ad7746.o

gpio.o: $(SYS)/gpio.c
	gcc ${CFLAGS} -c $(SYS)/gpio.c

net.o: $(SYS)/net.c
	gcc ${CFLAGS} -c $(SYS)/net.c

ad7746.o: $(DEV)/ad7746.c
	gcc ${CFLAGS} -c $(DEV)/ad7746.c

server.o: server.c
	gcc ${CFLAGS} -c server.c

.PHONY: clean
clean:
	@rm -f server.o
	@rm -f $(SYS)/gpio.o
	@rm -f $(SYS)/net.o
	@rm -f $(DEV)/ad7746.o

.PHONY: maintainer-clean
maintainer-clean: clean
	@rm -f server
