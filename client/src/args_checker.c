#include "args_checker.h"
#include "client.h"
#include "utils.h"

Error check_port(int *port) {
    bool valid = true;

    // available ports: 1024 - 49151
    if (*port < 0 || *port > 65535) {
        valid = false;
    }

    // avoiding reserved ports: 0 - 1023
    if (*port < 1024) {
        valid = false;
    }

    if (!valid) {
        return PORT_ERROR;
    }

    return SUCCESS;
}

Error check_name(char *name) {
    if (name == NULL || strlen(name) == 0) {
        return SERVER_NAME_ERROR;
    }

    return SUCCESS;
}

void check_args(int argc, char **server_name, int *server_port) {
    if (argc < 3) {
        handle_error(WRONG_PARAMS_ERROR);
    }

    // Doing some checks on the server name
    Error err_name = check_name(*server_name);
    handle_error(err_name);

    // Doing some checks on the port
    Error err_port = check_port(server_port);
    handle_error(err_port);
}

void handle_args(int argc, char *argv[], char **server_name, int *server_port) {
    *server_name = argv[1];
    *server_port = atoi(argv[2]);
    check_args(argc, server_name, server_port);
}