# Compiler
CC = gcc
CFLAGS = -Wall -g

# Targets
all: client server

client: client.o
	$(CC) -o client client.o

server: server.o sha256.o
	$(CC) -o server server.o sha256.o

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

sha256.o: sha256.c sha256.h
	$(CC) $(CFLAGS) -c sha256.c

# Clean target to remove object files and executables
clean:
	rm -f *.o client server

.PHONY: all clean
