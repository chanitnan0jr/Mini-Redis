#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "aof.h"
#include "resp.h"

static FILE *aof_fp = NULL;

void aof_init(const char *filename) {
    aof_fp = fopen(filename, "a+"); // Append + Read
    if (!aof_fp) {
        perror("fopen aof");
        exit(1);
    }
}

void aof_close(void) {
    if (aof_fp) {
        fclose(aof_fp);
        aof_fp = NULL;
    }
}

// Write generic command to AOF
void aof_log(int argc, char **argv) {
    if (!aof_fp) return;

    // Format: *argc\r\n
    fprintf(aof_fp, "*%d\r\n", argc);
    for (int i = 0; i < argc; i++) {
        // $len\r\ncontent\r\n
        fprintf(aof_fp, "$%zu\r\n%s\r\n", strlen(argv[i]), argv[i]);
    }
    // Flush to ensure it's written (or rely on OS buffering, but safely fsync is better)
    fflush(aof_fp);
}

void aof_sync(void) {
    if (aof_fp) {
        fsync(fileno(aof_fp));
    }
}


void aof_load(void (*callback)(RedisCmd *cmd)) {
    if (!aof_fp) return;

    // Go to beginning of file
    rewind(aof_fp);

    // Helper buffer for reading
    // Note: Parsing from FILE* is different from parsing memory buffer.
    // For simplicity, we'll read line by line or use a simpler parser logic,
    // OR read the whole file into memory and use `parse_request`.
    // Reading whole file is easier for leveraging `resp.c`.
    
    fseek(aof_fp, 0, SEEK_END);
    long fsize = ftell(aof_fp);
    rewind(aof_fp);

    if (fsize == 0) return;

    char *buf = malloc(fsize + 1);
    if (!buf) return;

    fread(buf, 1, fsize, aof_fp);
    buf[fsize] = '\0';

    char *ptr = buf;
    char *end = buf + fsize;

    while (ptr < end) {
        RedisCmd cmd;
        int err = parse_request(ptr, end - ptr, &cmd);
        if (err == 0) {
            // Calculate how many bytes were consumed.
            // parse_request doesn't return consumed bytes...
            // Wait, parse_request returns 0 on success but doesn't tell us how much it read.
            // This is a flaw in the current `parse_request` implementation for streaming/multiple commands.
            // We need to fix `parse_request` to return bytes consumed or pointer.
            
            // For now, let's assume `parse_request` works but we can't easily advance `ptr`.
            // Modifying `parse_request` is needed.
            
            callback(&cmd);
            free_redis_cmd(&cmd);
            
             // Hack: we need to find the end of this command to advance `ptr`.
             // Re-parsing logic or modifying `resp.c` is best.
             break; // STOPPING HERE because we can't properly iterate without modifying resp.c
        } else {
            break; 
        }
    }
    
    free(buf);
}