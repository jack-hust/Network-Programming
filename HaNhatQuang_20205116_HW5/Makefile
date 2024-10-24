all: client server

client: client.o
	$(CC) -o client client.o $(LDFLAGS)

server: server.o
	$(CC) -o server server.o $(LDFLAGS)

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

clean:
	rm -f client server *.o