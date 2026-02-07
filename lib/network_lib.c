#include "network_lib.h"

/*
 * Get sockaddr, IPv4 or IPv6:
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
 * Return a listening socket
 * Note: Modified to accept "port" as an argument
 */
int get_listener_socket(const char *port)
{
    struct addrinfo hints, *ai, *p;
    int yes=1;
    int rv;
    int listener;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, port, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    if (p == NULL) {
        return -1; // Let the caller handle the error
    }

    freeaddrinfo(ai); 

    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

/*
 * Ensure all data is sent
 */
int sendall(int s, char *buf, int *len)
{
    int total = 0;        
    int bytesleft = *len; 
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; 
    return n==-1?-1:0; 
}

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
        // printf("Foreign node: %s, Port: %s\n", host, service); // Optional logging
        return buf;
    } else {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
        return NULL;
    }
}

/*
 * Add new incoming connections to the pollfd array
 */
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

/*
 * Remove a file descriptor from the set
 */
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}

/*
 * Pack a floating point number into IEEE-754 format.
 */
uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;

    // -1 for sign bit
    unsigned significandbits = bits - expbits - 1;

    if (f == 0.0) return 0; // get this special case out of the way

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

/*
 * Unpack a floating point number from IEEE-754 format.
 */
long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    long double result;
    long long shift;
    unsigned bias;

    // -1 for sign bit
    unsigned significandbits = bits - expbits - 1;

    if (i == 0) return 0.0;

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}
