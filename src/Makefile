CFLAGS = -Wall -g0
LNKFLAGS = -lm

.PHONY: all
all: meas server

meas: meas.o ad7745.o
	gcc $(LNKFLAGS) -o meas meas.o ad7745.o

server: server.o ad7745.o
	gcc -o server server.o ad7745.o

ad7745.o: ad7745.c
	gcc ${CFLAGS} -c ad7745.c

meas.o: meas.c
	gcc ${CFLAGS} -c meas.c

server.o: server.c
	gcc ${CFLAGS} -c server.c

.PHONY: clean
clean:
	@rm -f *.o

.PHONY: maintainer-clean
maintainer-clean: clean
	@rm -f meas
	@rm -f server
