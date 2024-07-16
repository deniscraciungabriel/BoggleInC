#include <stdio.h>

#include "client.h"
#include "macros.h"
#include "args_checker.h"

int main(int argc, char *argv[]) {
    char* server_name;
    int port;

    handle_args(argc, argv, &server_name, &port);
    init_client(server_name, port);
    
    return 0;
}