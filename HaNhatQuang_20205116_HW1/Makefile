CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: hw1

kplc: hw1.o
	${CC} hw1.o -o kplc

main.o: hw1.c
	${CC} ${CFLAGS} hw1.c


clean:
	rm -f *.o *~