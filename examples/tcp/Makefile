CC = gcc
CFLAGS = -Wall -I../../lib

LIB_SRC = ../../lib/network_lib.c
LIB_OBJ = ../../lib/network_lib.o

TARGETS = server client selectserver pollserver

all: $(TARGETS)

$(LIB_OBJ): $(LIB_SRC)
	$(CC) $(CFLAGS) -c $(LIB_SRC) -o $(LIB_OBJ)

server: server.c $(LIB_OBJ)
	$(CC) $(CFLAGS) server.c $(LIB_OBJ) -o server

client: client.c $(LIB_OBJ)
	$(CC) $(CFLAGS) client.c $(LIB_OBJ) -o client

selectserver: selectserver.c $(LIB_OBJ)
	$(CC) $(CFLAGS) selectserver.c $(LIB_OBJ) -o selectserver

pollserver: pollserver.c $(LIB_OBJ)
	$(CC) $(CFLAGS) pollserver.c $(LIB_OBJ) -o pollserver

clean:
	rm -f $(TARGETS) $(LIB_OBJ)
