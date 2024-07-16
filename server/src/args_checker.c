#include "args_checker.h"
#include "server.h"
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

void handle_args(int argc, char *argv[], char **server_name, int *server_port, unsigned int *randomization_seed, float *game_length, char **matrix_file, char **dictionary_file) {
    *server_name = argv[1];
    *server_port = atoi(argv[2]);
    check_args(argc, server_name, server_port);

    // Set default values
    *randomization_seed = (unsigned int)time(NULL);
    *game_length = GAME_DURATION;
    printf("new game length: %.f\n", *game_length);
    *matrix_file = NULL;
    *dictionary_file = NULL;

    int option;
    // Defining long options for getopt_long
    // Each option has: name, argument requirement, flag, and short option character
    // For ref and examples: https://stackoverflow.com/questions/9642732/parsing-command-line-arguments-in-c
    static struct option long_opts[] = {
        {"matrici", required_argument, NULL, 'm'},
        {"durata",  required_argument, NULL, 'd'},
        {"seed",    required_argument, NULL, 's'},
        {"diz",     required_argument, NULL, 'z'},
        {0, 0, 0, 0}  // Terminating element
    };

    // Process command line options
    while ((option = getopt_long(argc, argv, "m:d:s:z:", long_opts, NULL)) != -1) {
        switch (option) {
            case 'm':
                *matrix_file = optarg;
                break;
            case 'd':
                *game_length = parse_position_float(optarg) * 60; // convert minutes to seconds
                break;
            case 's':
                *randomization_seed = (unsigned int)parse_positive_int(optarg);
                break;
            case 'z':
                *dictionary_file = optarg;
                break;
            default:
                handle_error(WRONG_PARAMS_ERROR);
        }
    }
}