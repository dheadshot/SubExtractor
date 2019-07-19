subextractor: main.o
	gcc -o subextractor main.o
	chmod +x subextractor

main.o: main.c tstamp.h
	gcc ${CFLAGS} -c main.c


