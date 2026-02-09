#ifndef AOF_H
#define AOF_H

#include "resp.h"

void aof_init(const char *filename);
void aof_close(void);
void aof_sync(void);
void aof_log(int argc, char **argv); // Log a command
void aof_load(void (*callback)(RedisCmd *cmd)); // Replay commands

#endif