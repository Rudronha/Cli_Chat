# server/Makefile
CC = gcc
CFLAGS = -pthread -Wall -g
TARGET = server

all: $(TARGET)

$(TARGET): server.c
	$(CC) $(CFLAGS) -o $(TARGET) server.c

clean:
	rm -f $(TARGET)

