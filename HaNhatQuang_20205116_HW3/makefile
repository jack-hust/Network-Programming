CC = gcc
CFLAGS = -Wall -g -I/usr/include/libxml2
LIBS = -L/usr/lib -lcurl -lxml2
TARGET = resolver

SRC = hw3.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)
