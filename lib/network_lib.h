#ifndef NETWORK_LIB_H
#define NETWORK_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h> // for uint64_t

// --- Add these Prototypes ---
void *get_in_addr(struct sockaddr *sa);
int get_listener_socket(const char *port);
int sendall(int s, char *buf, int *len);
const char *inet_ntop2(void *addr, char *buf, size_t size, socklen_t addrlen);
void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size);
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

// --- IEEE-754 Serialization Macros & Prototypes ---
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

uint64_t pack754(long double f, unsigned bits, unsigned expbits);
long double unpack754(uint64_t i, unsigned bits, unsigned expbits);

#endif