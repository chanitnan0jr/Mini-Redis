#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "resp.h"

/*
 * Helper Function: read_line
 * --------------------------
 * Finds the first occurrence of "\r\n" (CRLF).
 *
 * buf:      Pointer to the current position in the buffer.
 * line_len: Output pointer to store the length of the line (excluding CRLF).
 *
 * Returns:  Pointer to the beginning of the NEXT line (after \r\n),
 * or NULL if CRLF is not found.
 */
static const char *read_line(const char *buf, int *line_len) {
    const char *end = strstr(buf, "\r\n");
    if (!end) {
        return NULL; // CRLF not found, data might be incomplete
    }
    
    *line_len = end - buf; // Calculate length of the current segment
    return end + 2;        // Move pointer past "\r\n"
}

/*
 * Helper Function: parse_int
 * --------------------------
 * Reads a line and converts it to an integer.
 * Used for parsing array size (*3) or string length ($5).
 *
 * buf:    Pointer to the current position.
 * result: Output pointer to store the parsed integer.
 *
 * Returns: Pointer to the beginning of the NEXT line.
 */
static const char *parse_int(const char *buf, int *result) {
    int len;
    const char *next = read_line(buf, &len);
    if (!next) return NULL;

    // Create a temporary buffer to safely null-terminate the number string
    char num_buf[32];
    if ((size_t)len >= sizeof(num_buf)) return NULL; // Safety check for overflow
    
    memcpy(num_buf, buf, len);
    num_buf[len] = '\0';
    
    *result = atoi(num_buf);
    return next;
}

/*
 * Main Function: parse_request
 * ----------------------------
 * Parses a raw RESP buffer into a RedisCmd struct.
 * Expected format: Array of Bulk Strings (e.g., *3\r\n$3\r\nSET\r\n...)
 *
 * buf: Raw input buffer from the client.
 * len: Length of the buffer.
 * cmd: Pointer to the RedisCmd struct to populate.
 *
 * Returns: 0 on success, negative value on error.
 */
int parse_request(char *buf, size_t len, RedisCmd *cmd) {
    const char *ptr = buf;           // Cursor to traverse the buffer
    const char *end_buf = buf + len; // Boundary check

    // 1. Check if the buffer starts with the Array indicator '*'
    if (*ptr != '*') {
        return -1; // Invalid format (we only expect Arrays for commands)
    }
    ptr++; // Skip '*'

    // 2. Read the number of arguments (argc)
    int argc;
    ptr = parse_int(ptr, &argc);
    if (!ptr) {
        return -2; // Error parsing integer or incomplete data
    }

    cmd->argc = argc;
    
    // Allocate memory for the array of string pointers
    // Note: We haven't allocated the strings themselves yet, just the container.
    cmd->argv = malloc(sizeof(char*) * argc);
    if (cmd->argv == NULL) return -3; // Memory allocation failed

    cmd->name = NULL; 

    // 3. Loop to parse each argument (Bulk String)
    for (int i = 0; i < argc; i++) {
        // Every argument must start with '$' (Bulk String)
        if (*ptr != '$') {
            // TODO: call a cleanup function here before returning
            return -4; // Protocol error
        }
        ptr++; // Skip '$'

        // Read the length of the string
        int str_len;
        ptr = parse_int(ptr, &str_len);
        if (!ptr) {
            return -5;
        }

        // Safety Check: Ensure we have enough data in the buffer
        // We need: str_len bytes + 2 bytes for "\r\n"
        if (ptr + str_len + 2 > end_buf) {
            return -6; // Incomplete data
        }

        // Allocate memory for the string (+1 for Null Terminator)
        cmd->argv[i] = malloc(str_len + 1);
        if (cmd->argv[i] == NULL) return -7;

        // Copy data and null-terminate
        memcpy(cmd->argv[i], ptr, str_len);
        cmd->argv[i][str_len] = '\0';

        // Set the command name (usually the first argument, e.g., "SET", "GET")
        if (i == 0) {
            cmd->name = cmd->argv[0];
        }

        // Move cursor past the string data and the trailing "\r\n"
        ptr += str_len + 2;
    }

    return (ptr - buf); // Success
}

/*
 * Cleanup Function: free_redis_cmd
 * --------------------------------
 * Frees all memory allocated during parsing.
 */
void free_redis_cmd(RedisCmd *cmd) {
    if (cmd->argv) {
        for (int i = 0; i < cmd->argc; i++) {
            if (cmd->argv[i]) {
                free(cmd->argv[i]); // Free individual strings
            }
        }
        free(cmd->argv); // Free the array of pointers
    }
}
