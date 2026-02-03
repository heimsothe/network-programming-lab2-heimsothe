CC = gcc
CFLAGS = -Wall -g

all: client server

client: client.c utils/utils.c cJSON.c cJSON.h utils/utils.h
	$(CC) $(CFLAGS) -o client client.c utils/utils.c cJSON.c

server: server.c utils/utils.c cJSON.c cJSON.h utils/utils.h
	$(CC) $(CFLAGS) -o server server.c utils/utils.c cJSON.c

clean:
	rm -f client server