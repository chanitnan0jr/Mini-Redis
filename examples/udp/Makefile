CC = gcc
CFLAGS = -Wall -I../../lib

LIB_SRC = ../../lib/network_lib.c
LIB_OBJ = ../../lib/network_lib.o

TARGETS = listener talker

all: $(TARGETS)

$(LIB_OBJ): $(LIB_SRC)
	$(CC) $(CFLAGS) -c $(LIB_SRC) -o $(LIB_OBJ)

listener: listener.c $(LIB_OBJ)
	$(CC) $(CFLAGS) listener.c $(LIB_OBJ) -o listener

talker: talker.c $(LIB_OBJ)
	$(CC) $(CFLAGS) talker.c $(LIB_OBJ) -o talker

clean:
	rm -f $(TARGETS) $(LIB_OBJ)
