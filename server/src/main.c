#include <stdio.h>

#include "server.h"
#include "macros.h"
#include "args_checker.h"

void show_args(char *server_name, int server_port, char *matrix_file, float game_duration, unsigned int randomization_seed, char *dictionary_file) {
    printf("\nServer name: %s\n", server_name);
    printf("Server port: %d\n", server_port);
    printf("Random seed: %u\n", randomization_seed);
    printf("Game duration: %.f seconds\n", game_duration);
    printf("Pre game duration: %d secondss\n", PRE_GAME_DURATION);
    matrix_file ? printf("Matrix filename: %s\n", matrix_file) : printf("Matrix filename: not provided, will generate matrices randomly.\n");
    dictionary_file ? printf("New dictionary file: %s\n\n", dictionary_file) :printf("New dictionary file: not provided, using default dictionary_file\n\n");
}

int main(int argc, char *argv[]) {
    char *server_name;
    int server_port;
    unsigned int randomization_seed;
    float game_duration; // in minutes
    char *matrix_file;
    char *dictionary_file;

    handle_args(argc, argv, &server_name, &server_port, &randomization_seed, &game_duration, &matrix_file, &dictionary_file);
    show_args(server_name, server_port, matrix_file, game_duration , randomization_seed, dictionary_file);
    init_server(server_name, server_port, randomization_seed, game_duration, matrix_file, dictionary_file);
    
    return 0;
}