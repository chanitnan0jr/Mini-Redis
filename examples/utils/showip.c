#include "network_lib.h"
/*this file contains code to lookup and display the all IP addresses for a given hostname
usage: ./showip hostname
example: ./showip www.google.com
*/
int main(int argc, char *argv[]){
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    /*
    argv[0] = "./showip" (ชื่อโปรแกรมจะอยู่ที่ช่อง 0 เสมอ)
    argv[1] = "www.google.com" (ข้อมูลที่เราอยากได้จริงๆ)
    argv[2] = NULL (จุดจบ บอกว่าหมดแล้ว)
    */

    if (argc != 2) {
        fprintf(stderr,"usage: showip hostname\n");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // Either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    /*
    Port is NULL because we only want the IP address not connect to the server
    */
    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    printf("IP addresses for %s:\n\n", argv[1]);

    //
    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            ipver = "IPv4";
        } else { // IPv6
            ipver = "IPv6";
        }

        addr = get_in_addr((struct sockaddr *)p->ai_addr);

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    }

    freeaddrinfo(res); // free the linked list

    return 0;
}