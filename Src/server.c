#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

//this file contains code to setup a server socket that binds to port 3490

int main(){
    //server setup code
    struct sockaddr_storage incoming_addr; // socket address information about the incoming connection
    struct addrinfo hints, *res, *p;
    socklen_t addr_size;
    const int backlog = 10; // how many pending connections queue will hold
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
        break; // if we get here, we have successfully bound the socket
    }
    //check if we got bound to an address or no address worked
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    printf("server: bound to port 3490\n");
    freeaddrinfo(res); // got what we needed from getaddrinfo(), free the linked list

    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        return 1;
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
        char user_addr[INET6_ADDRSTRLEN]; // get the address of the client
        inet_ntop(incoming_addr.ss_family,
                  (incoming_addr.ss_family == AF_INET) ?
                  (void *)&((struct sockaddr_in *)&incoming_addr)->sin_addr :
                  (void *)&((struct sockaddr_in6 *)&incoming_addr)->sin6_addr,
                  user_addr, sizeof user_addr);
        printf("Client address: %s\n", user_addr);
        
        //fork a new process to handle the multiple clients
        if(!fork()) {
            close(sockfd); // child doesn't need the listener
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