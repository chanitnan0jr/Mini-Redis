#include "server.h"
#include "net.h"
#include "conn.h"
#include "store.h"
#include "resp.h"
#include "aof.h" 
#include <ctype.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h> // for strcasecmp

#define PORT "3490" // Default port

// Global Key-Value Store
// Defined in store.c, access via store_get_db()

// Network variables
static int listener;
static struct pollfd *pfds;
static int fd_count = 0;
static int fd_size = 5;

// --- Helper Functions ---

// Send a simple string response (+OK\r\n)
void send_simple_string(int fd, const char *msg) {
    char buf[256];
    int len = snprintf(buf, sizeof(buf), "+%s\r\n", msg);
    sendall(fd, buf, &len);
}

// Send an error response (-ERR ...\r\n)
void send_error(int fd, const char *msg) {
    char buf[256];
    int len = snprintf(buf, sizeof(buf), "-%s\r\n", msg);
    sendall(fd, buf, &len);
}

// Send a bulk string response ($len\r\nstring\r\n)
void send_bulk_string(int fd, const char *str) {
    char buf[1024];
    if (!str) {
        // Null Bulk String for non-existent keys ($-1\r\n)
        int len = sprintf(buf, "$-1\r\n");
        sendall(fd, buf, &len);
        return;
    }
    int len = snprintf(buf, sizeof(buf), "$%zu\r\n%s\r\n", strlen(str), str);
    sendall(fd, buf, &len);
}

// Send an integer response (:num\r\n)
void send_integer(int fd, int val) {
    char buf[64];
    int len = sprintf(buf, ":%d\r\n", val);
    sendall(fd, buf, &len);
}

// --- AOF Replay Handler ---
// Replays commands from the AOF log when the server starts.
// Unlike process_command, this does NOT send responses to a socket.
void replay_command(RedisCmd *cmd) {
    if (!cmd->name) return;

    HMap *db = store_get_db();

    if (strcasecmp(cmd->name, "SET") == 0) {
        // Silent insert
        hmap_insert(db, cmd->argv[1], cmd->argv[2]);
    } else if (strcasecmp(cmd->name, "DEL") == 0) {
        // Silent delete
        hmap_delete(db, cmd->argv[1]);
    }
    // GET and PING are ignored during replay
}

// --- Main Command Processor ---
void process_command(int fd, RedisCmd *cmd) {
    if (cmd->argc == 0) return;
    if (!cmd->name) return;

    HMap *db = store_get_db();

    // --- SET Command ---
    if (strcasecmp(cmd->name, "SET") == 0) {
        if (cmd->argc != 3) {
            send_error(fd, "ERR wrong number of arguments for 'set' command");
            return;
        }
        
        // 1. Apply to in-memory database
        hmap_insert(db, cmd->argv[1], cmd->argv[2]);
        
        // 2. Persist to Disk (AOF)
        aof_log(cmd->argc, cmd->argv);
        aof_sync(); // Force fsync to ensure durability

        // 3. Send response to client
        send_simple_string(fd, "OK");

    // --- GET Command ---
    } else if (strcasecmp(cmd->name, "GET") == 0) {
        if (cmd->argc != 2) {
            send_error(fd, "ERR wrong number of arguments for 'get' command");
            return;
        }
        HNode *node = hmap_lookup(db, cmd->argv[1]);
        if (node) {
            send_bulk_string(fd, node->value);
        } else {
            send_bulk_string(fd, NULL);
        }

    // --- DEL Command ---
    } else if (strcasecmp(cmd->name, "DEL") == 0) {
        if (cmd->argc != 2) {
            send_error(fd, "ERR wrong number of arguments for 'del' command");
            return;
        }
        
        // 1. Apply to in-memory database
        int deleted = hmap_delete(db, cmd->argv[1]);
        
        // 2. Persist to Disk (AOF)
        if (deleted) {
             aof_log(cmd->argc, cmd->argv);
             aof_sync();
        }

        // 3. Send response to client
        send_integer(fd, deleted);

    // --- PING Command ---
    } else if (strcasecmp(cmd->name, "PING") == 0) {
        send_simple_string(fd, "PING A RAI KUB");

    } else {
        printf("Unknown command: %s\n", cmd->name);
        send_error(fd, "ERR unknown command");
    }
}

// Accept a new incoming connection
void handle_new_connection() {
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof remoteaddr;

    int newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

    if (newfd == -1) {
        perror("accept");
    } else {
        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);
        printf("New connection on socket %d\n", newfd);
    }
}

// Handle incoming data from an existing client
void handle_client_data(int i) {
    char buf[4096]; // 4KB Buffer
    int sender_fd = pfds[i].fd;

    int nbytes = recv(sender_fd, buf, sizeof(buf) - 1, 0);

    if (nbytes <= 0) {
        if (nbytes == 0) {
            printf("Socket %d hung up\n", sender_fd);
        } else {
            perror("recv");
        }
        close(sender_fd);
        del_from_pfds(pfds, i, &fd_count);
    } else {
        buf[nbytes] = '\0'; // Null-terminate string
        
        RedisCmd cmd;
        // Parse the RESP request
        int processed = parse_request(buf, nbytes, &cmd);
        
        // If parsing is successful, process the command
        if (processed >= 0) { 
            process_command(sender_fd, &cmd);
            free_redis_cmd(&cmd);
        } else {
             printf("Protocol error or incomplete data\n");
             // Optional: send_error(sender_fd, "ERR processing request"); 
        }
    }
}

void server_init(const char *port)
{
    // 1. Initialize the Key-Value Store
    store_init();
    
    // 2. Initialize AOF System & Recover Data
    printf("[AOF] Initializing AOF system...\n");
    aof_init("database.aof");
    
    printf("[AOF] Restoring data from disk...\n");
    aof_load(replay_command); // Replay log to restore state
    printf("[AOF] Data loaded successfully.\n");

    // 3. Setup Network Listener
    listener = get_listener_socket(port);
    if (listener == -1) {
        fprintf(stderr, "error getting listener socket\n");
        exit(1);
    }
    
    // Initialize pollfds array
    pfds = malloc(sizeof *pfds * fd_size);
    pfds[0].fd = listener;
    pfds[0].events = POLLIN; // Monitor listener for new connections
    fd_count = 1;

    printf("Server initialized on port %s\n", port);
}

void server_run() {
    printf("Server running...\n");
    for(;;) {
        int poll_count = poll(pfds, fd_count, -1);

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        for(int i = 0; i < fd_count; i++) {
            if (pfds[i].revents & POLLIN) {
                if (pfds[i].fd == listener) {
                    handle_new_connection();
                } else {
                    handle_client_data(i);
                }
            }
        }
    }
}