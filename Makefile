CC = gcc
CFLAGS = -g -Wall -Wextra

client: client.c aux.c parson.c requests.c
	$(CC) $(CFLAGS) -o client client.c aux.c parson.c requests.c

run: client
	./client

clean:
	rm -f *.o client