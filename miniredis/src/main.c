#include <stdio.h>
#include "server.h"
#include "resp.h"
#include "aof.h"

int main() {
    printf("Mini-Redis Server starting\n");
    // Initialization is now handled inside server_init
    server_init("3490");
    server_run();
    aof_close();
    return 0;
}