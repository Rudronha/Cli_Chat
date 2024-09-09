# client/Makefile
CC = gcc
CFLAGS = -pthread -Wall -g
TARGET = client

all: $(TARGET)

$(TARGET): client.c
	$(CC) $(CFLAGS) -o $(TARGET) client.c

clean:
	rm -f $(TARGET)

