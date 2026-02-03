CC = gcc
CFLAGS = -Wall -g

all: client server

client: client.c utils.c cJSON.c cJSON.h utils.h
	$(CC) $(CFLAGS) -o client client.c utils.c cJSON.c

server: server.c utils.c cJSON.c cJSON.h utils.h
	$(CC) $(CFLAGS) -o server server.c utils.c cJSON.c

clean:
	rm -f client server