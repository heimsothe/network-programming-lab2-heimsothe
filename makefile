CC = gcc
CFLAGS = -Wall -g

all: client server

client: client.c cJSON.c cJSON.h
	$(CC) $(CFLAGS) -o client client.c cJSON.c

server: server.c cJSON.c cJSON.h
	$(CC) $(CFLAGS) -o server server.c cJSON.c

clean:
	rm -f client server