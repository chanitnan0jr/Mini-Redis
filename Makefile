# 1. set up compiler and flags
CC = gcc
CFLAGS = -Wall -I./lib  # -I./lib tells the compiler to look for header files in the 'lib' directory

# 2. tell make where the source files are and what the targets are
SRC_DIR = Src
TARGETS = server client showip listener talker

# 3. Default target
all: $(TARGETS)

# 4. create rules for each target
server: $(SRC_DIR)/server.c
	$(CC) $(CFLAGS) $(SRC_DIR)/server.c -o $(SRC_DIR)/server

client: $(SRC_DIR)/client.c
	$(CC) $(CFLAGS) $(SRC_DIR)/client.c -o $(SRC_DIR)/client

listener: $(SRC_DIR)/listener.c
	$(CC) $(CFLAGS) $(SRC_DIR)/listener.c -o $(SRC_DIR)/listener

talker: $(SRC_DIR)/talker.c
	$(CC) $(CFLAGS) $(SRC_DIR)/talker.c -o $(SRC_DIR)/talker

showip: $(SRC_DIR)/showip.c
	$(CC) $(CFLAGS) $(SRC_DIR)/showip.c -o $(SRC_DIR)/showip

# 5. clean up compiled files
clean:
	rm -f $(SRC_DIR)/server $(SRC_DIR)/client $(SRC_DIR)/showip