#include "network_lib.h"
#define PORT "9034" 

/*
Helper function
Convert socket to IP address string.
*/
// (moved to network_lib.c)

/*
Return a listening socket.
*/
// (moved to network_lib.c)

/*
 * Add a new file descriptor to the set.
 */
// (moved to network_lib.c)

/*
 * Remove a file descriptor at a given index from the set.
 */
// (moved to network_lib.c)

/*
 Handle incoming connections.
 accept connection then add to pollfd array and print client address
 */
void handle_new_connection(int listener, int *fd_count,
        int *fd_size, struct pollfd **pfds)
{
    struct sockaddr_storage remoteaddr; // Client address (IP/Port) for ipv4 and ipv6
    socklen_t addrlen;
    int newfd;  // Newly accept()ed socket descriptor
    char remoteIP[INET6_ADDRSTRLEN]; //buffer for IP address string ex."192.168.1.5"

    addrlen = sizeof remoteaddr;
    newfd = accept(listener, (struct sockaddr *)&remoteaddr,&addrlen);

    if (newfd == -1) {
        perror("accept");
    } else {
        add_to_pfds(pfds, newfd, fd_count, fd_size);

        printf("pollserver: new connection from %s on socket %d\n",
                inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP, addrlen),
                newfd);
    }
}

/*
 * Handle regular client data or client hangups.
 */
void handle_client_data(int listener, int *fd_count, 
    struct pollfd *pfds, int *pfd_i)
{
    char buf[256];// Buffer for client data
    int nbytes = recv(pfds[*pfd_i].fd, buf, sizeof buf, 0);//receive data from client
    int sender_fd = pfds[*pfd_i].fd; //sender file descriptor

    if (nbytes <= 0) { // Got error or connection closed by client
        if (nbytes == 0) {
            // Connection closed if nbytes == 0
            printf("pollserver: socket %d hung up\n", sender_fd);
        } else {
            // Error if nbytes < 0
            perror("recv");
        }

        close(pfds[*pfd_i].fd); // Bye!
        del_from_pfds(pfds, *pfd_i, fd_count);
        /* 
        re-examine the slot we just deleted
        decrement i so that the next iteration of the for loop
        doesn't skip the element that moved into this slot
        */
        (*pfd_i)--;

    } else { 
        // We got some good data from a client
        printf("pollserver: recv from fd %d: %.*s", sender_fd,
                nbytes, buf);
        // broadcast the message to all other clients
        for(int j = 0; j < *fd_count; j++) {
            int dest_fd = pfds[j].fd;

            // Except the listener and ourselves
            if (dest_fd != listener && dest_fd != sender_fd) {
                if (send(dest_fd, buf, nbytes, 0) == -1) {
                    perror("send");
                }
            }
        }
    }
}

/*
 * Process all existing connections.
 */
void process_connections(int listener, int *fd_count, int *fd_size,
        struct pollfd **pfds)
{
    for(int i = 0; i < *fd_count; i++) {

        // Check if someone's ready to read using Bitwise OR
        if ((*pfds)[i].revents & (POLLIN | POLLHUP)) {
            /*
            POLLIN: Data is available to read or accept
            POLLHUP: Connection closed
            */
            if ((*pfds)[i].fd == listener) {
                // If we're the listener, it's a new connection
                handle_new_connection(listener, fd_count, fd_size,pfds);
            } else {
                // Otherwise we're just a regular client
                handle_client_data(listener, fd_count, *pfds, &i);
                /* 
                *** IMPORTANT ***
                If we remove the client from the pfds array, 
                we need to decrement i to avoid skipping the next 
                element in the array.
                */
            }
        }
    }
}

/*
 * Main: create a listener and connection set, loop forever
 * processing connections.
 */
int main(void)
{
    int listener;     // Listening socket descriptor

    // Start off with room for 5 connections
    // (We'll realloc as necessary)
    int fd_size = 5;
    int fd_count = 0;
    struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    // Set up and get a listening socket
    listener = get_listener_socket(PORT);

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    // Add the listener to set;
    // Report ready to read on incoming connection
    pfds[0].fd = listener;
    pfds[0].events = POLLIN;

    fd_count = 1; // For the listener

    puts("pollserver: waiting for connections...");

    // Main loop
    for(;;) {
        /*
        Wait infinitely(-1) until some one connect or send data or close the connection to the server
        or all the events(poll_count) are processed
        */
        int poll_count = poll(pfds, fd_count, -1);

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        // Run through connections looking for data to read
        process_connections(listener, &fd_count, &fd_size, &pfds);
    }
    /*
    Close the listening socket 
    (though we will never reach here) cause of the infinite loop
    and server socket will be closed when the program ends)
    */
    free(pfds);
}