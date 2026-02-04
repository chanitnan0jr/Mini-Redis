#include "network_lib.h"
//this file contains code to setup a server socket that binds to port 3490

void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int main(){
    //server setup code
    struct sockaddr_storage incoming_addr; // socket address information about the incoming connection
    struct addrinfo hints, *res, *p;
    struct sigaction sa;
    socklen_t addr_size;
    const int backlog = 10; // how many pending connections queue will hold
    char my_hostname[128];
    int new_fd;
    int status;
    int sockfd;
    int yes=1; //char yes='1'; // Solaris people use this

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // For wildcard IP address

    if ((status = getaddrinfo(NULL, "3490", &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 1;
    }else {
        printf("1: getaddrinfo success\n");
    }
    for (p = res; p != NULL; p = p->ai_next) {
        // try to create socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket"); //if socket fails, try next address
            continue;
        }
        printf("2: Socket created\n");
        // set socket options to reuse address lose the pesky "Address already in use" error message
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            return 1;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd); // close the socket if bind fails
            perror("3: server: bind"); 
            continue; //bind failed, try next address
        }
        if (gethostname(my_hostname, sizeof my_hostname) == -1) {
        perror("gethostname");
        }
        break; // if we get here, we have successfully bound the socket
    }
    //check if we got bound to an address or no address worked
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    printf("server: bound to port 3490 on hostname %s\n", my_hostname);
    freeaddrinfo(res); // got what we needed from getaddrinfo(), free the linked list

    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        return 1;
    }

    // reap all dead processes
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    printf("4: server: waiting for connections\n");

    //server would normally recv() and send() data here
    while(1) {
        addr_size = sizeof incoming_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&incoming_addr, &addr_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        printf("5: server: accepted connection on socket %d\n", new_fd);
        
        //fork a new process to handle the multiple clients
        if(!fork()) {
            close(sockfd); // child doesn't need the listener
            struct sockaddr_storage peer_addr;
            socklen_t peer_len = sizeof peer_addr;
            if (getpeername(new_fd, (struct sockaddr *)&peer_addr, &peer_len) == -1) {
                perror("getpeername");
            } else {
                char peer_addr_str[INET6_ADDRSTRLEN];
                inet_ntop(peer_addr.ss_family,
                          (peer_addr.ss_family == AF_INET) ?
                          (void *)&((struct sockaddr_in *)&peer_addr)->sin_addr :
                          (void *)&((struct sockaddr_in6 *)&peer_addr)->sin6_addr,
                          peer_addr_str, sizeof peer_addr_str);
                //or use get_in_addr(struct sockaddr *sa) function from Beej's guide
                /*
                get_in_addr function example:
                inet_ntop(peer_addr.ss_family,
                          get_in_addr((struct sockaddr *)&peer_addr),
                          peer_addr_str, sizeof peer_addr_str);
                */
                printf("Client address: %s\n", peer_addr_str);
            }
            const char *msg = "Hello from Sinu server!\n";
            if (send(new_fd, msg, strlen(msg), 0) == -1) {
                perror("send");
            }
            close(new_fd); // close child's socket
            exit(0);
        }

        close(new_fd); // Close parent's socket
    }
    close(sockfd); 
    /*
    Close the listening socket 
    (though we will never reach here) cause of the infinite loop
    and server socket will be closed when the program ends)
    */
    return 0;
}