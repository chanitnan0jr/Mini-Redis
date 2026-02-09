#ifndef MINIREDIS_NET_H
#define MINIREDIS_NET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

// Opens a listening socket on the given port
int get_listener_socket(const char *port);

// Sets a socket to non-blocking mode
int set_nonblocking(int fd);

// Ensures all data in the buffer is sent
int sendall(int s, char *buf, int *len);

// Helper to get IPv4 or IPv6 address
void *get_in_addr(struct sockaddr *sa);

#endif
