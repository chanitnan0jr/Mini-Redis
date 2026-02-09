#ifndef RESP_H
#define RESP_H

#include <stddef.h> // size_t

typedef struct RedisCmd {
    int argc;
    char **argv;
    char *name; // Shortcut to argv[0], no need to free separate
} RedisCmd;

int parse_request(char *buf, size_t len, RedisCmd *cmd);
void free_redis_cmd(RedisCmd *cmd);

#endif
