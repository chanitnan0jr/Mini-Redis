CC = gcc
CFLAGS = -Wall -I../../lib

LIB_SRC = ../../lib/network_lib.c
LIB_OBJ = ../../lib/network_lib.o

TARGETS = showip pack_test

all: $(TARGETS)

$(LIB_OBJ): $(LIB_SRC)
	$(CC) $(CFLAGS) -c $(LIB_SRC) -o $(LIB_OBJ)

showip: showip.c $(LIB_OBJ)
	$(CC) $(CFLAGS) showip.c $(LIB_OBJ) -o showip

pack_test: pack_test.c $(LIB_OBJ)
	$(CC) $(CFLAGS) pack_test.c $(LIB_OBJ) -o pack_test

clean:
	rm -f $(TARGETS) $(LIB_OBJ)
