#ifndef MINIREDIS_CONN_H
#define MINIREDIS_CONN_H

#include <poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define INITIAL_BUF_SIZE 1024

// Structure to hold connection state
struct connection {
    int fd;
    char *rbuf;
    size_t rbuf_size;
    size_t rbuf_used;
    char *wbuf;
    size_t wbuf_size;
    size_t wbuf_used;
    struct connection *next;
};

// Adds a file descriptor to the pollfd array
void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size);

// Removes a file descriptor from the pollfd array
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

// Creates a new connection object
struct connection *conn_create(int fd);

// Frees a connection object
void conn_free(struct connection *conn);

#endif
