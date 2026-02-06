/*
** selectserver.c -- a cheezy multiperson chat server
*/
#include "network_lib.h"

#define PORT "9034"   // port we're listening on

/*
 * Convert socket to IP address string.
 * addr: struct sockaddr_in or struct sockaddr_in6
 */
const char *inet_ntop2(void *addr, char *buf, size_t size, socklen_t addrlen)
{
    // Standard constants: Ensures buffers fit any IPv6/Port.
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    // Using fixed size (sizeof sockaddr_storage) or actual 'socklen_t addrlen' as an argument.
    int s = getnameinfo((struct sockaddr *)addr, addrlen,
                        host, sizeof(host),
                        service, sizeof(service),
                        NI_NUMERICHOST | NI_NUMERICSERV); // FAST: No DNS lookup (returns raw IP/Port)

    if (s == 0) {
        snprintf(buf, size, "%s", host); // Safe copy to user buffer
        printf("Foreign node: %s, Port: %s\n", host, service);
        return buf;
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        return NULL;
    }
}

/*
 * Return a listening socket
 */
int get_listener_socket(void)
{
    struct addrinfo hints, *ai, *p;
    int yes=1;    // for setsockopt() SO_REUSEADDR, below
    int rv;
    int listener;

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    return listener;
}

/*
 * Add new incoming connections to the proper sets
 */
void handle_new_connection(int listener, fd_set *master, int *fdmax)
{
    socklen_t addrlen;
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address handles both IPv4 and IPv6
    char remoteIP[INET6_ADDRSTRLEN];

    addrlen = sizeof remoteaddr;
    newfd = accept(listener,
        (struct sockaddr *)&remoteaddr,
        &addrlen);
    // accept won't get blocked because we are using select() that only return when there is a new connection or data to be read
    
    if (newfd == -1) {
        perror("accept");
    } else {
        FD_SET(newfd, master); // add this new socket to master set
        if (newfd > *fdmax) {  // keep track of the max file descriptor number
            *fdmax = newfd;
        }
        printf("selectserver: new connection from %s on "
            "socket %d\n",
            inet_ntop2(&remoteaddr, remoteIP, sizeof remoteIP, addrlen),
            newfd);
    }
}

/*
 * Broadcast a message to all clients
 */
void broadcast(char *buf, int nbytes, int listener, int s,
               fd_set *master, int fdmax)
{
    // Iterate through all possible file descriptors
    for(int j = 0; j <= fdmax; j++) {
        
        // Check if the file descriptor is currently active in our master set
        if (FD_ISSET(j, master)) {
            
            // FILTERING LOGIC:
            // 1. Don't send to the listener socket (it handles connections, not data).
            // 2. Don't send back to the sender (avoids echo/duplicate messages).
            if (j != listener && j != s) {
                if (send(j, buf, nbytes, 0) == -1) {
                    perror("send");
                }
            }
        }
    }
}

/*
 * Handle client data and hangups
 */
void handle_client_data(int s, int listener, fd_set *master,
                        int fdmax)
{
    char buf[256];    // buffer for client data
    int nbytes;

    // handle data from a client
    if ((nbytes = recv(s, buf, sizeof buf, 0)) <= 0) {
        // got error or connection closed by client
        if (nbytes == 0) {
            // connection closed
            printf("selectserver: socket %d hung up\n", s);
        } else {
            perror("recv");
        }
        close(s); // close the socket cause the client disconnected
        FD_CLR(s, master); // remove from master set
    } else {
        // we got some data from a client
        broadcast(buf, nbytes, listener, s, master, fdmax);
    }
}

/*
 * Main
 */
int main(void)
{
    fd_set master;    // Master list: Keeps track of ALL active connections
    fd_set read_fds;  // Temp list: Used specifically for the select() call
    int fdmax;        // Max file descriptor number (needed for select sizing)

    int listener;     

    // Initialize the sets to be empty
    FD_ZERO(&master);    
    FD_ZERO(&read_fds);

    listener = get_listener_socket();

    // Add the listener to the master set so select() monitors for new connections
    FD_SET(listener, &master);

    // Currently, the listener is the socket with the highest ID
    fdmax = listener; 

    // Main Event Loop
    for(;;) {
        // Copy master to read_fds
        // select() changes the set you pass to it to indicate which sockets are ready.
        // If we didn't copy, we'd lose our list of connected clients after the first call.
        read_fds = master; 

        // Wait here until something happens on one of the sockets
        // fdmax+1: tells the kernel how many descriptors to check
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

            /*
            Using Bitwise AND to check if the file descriptor is in the set
            Cause select() will send back the set of file descriptors
            0 for not ready, 1 for socket that is ready to be read
            we need to iterate through all possible sockets to see which one woke up    
            by function FD_ISSET(file_descriptor, set)
            */
        for(int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                
                if (i == listener) {
                    // The listener is ready -> New incoming connection
                    handle_new_connection(i, &master, &fdmax);
                } else {
                    // A client socket is ready -> Receiving data (chat message)
                    handle_client_data(i, listener, &master, fdmax);
                }
            }
        }
    }

    return 0;
}