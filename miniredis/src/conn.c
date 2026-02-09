#include "conn.h"

void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size) {
        *fd_size *= 2; // Double it
        struct pollfd *temp = realloc(*pfds, sizeof(**pfds) * (*fd_size));

        if (temp != NULL) {
            *pfds = temp;
        } else {
            perror("realloc");
            exit(1);
        }
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read
    (*pfds)[*fd_count].revents = 0;

    (*fd_count)++;
}

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}

struct connection *conn_create(int fd)
{
    struct connection *conn = malloc(sizeof(struct connection));
    if (!conn) {
        return NULL;
    }
    conn->fd = fd;
    conn->rbuf = malloc(INITIAL_BUF_SIZE);
    conn->rbuf_size = INITIAL_BUF_SIZE;
    conn->rbuf_used = 0;
    conn->wbuf = malloc(INITIAL_BUF_SIZE);
    conn->wbuf_size = INITIAL_BUF_SIZE;
    conn->wbuf_used = 0;
    conn->next = NULL;
    return conn;
}

void conn_free(struct connection *conn)
{
    if (conn) {
        if (conn->rbuf) free(conn->rbuf);
        if (conn->wbuf) free(conn->wbuf);
        free(conn);
    }
}
