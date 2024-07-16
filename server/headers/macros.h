#ifndef MACROS_H
#define MACROS_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// Check system call return value for -1 and handle error
#define SYSC(v, c, m) if ((v = c) == -1) { perror(m); exit(errno); }

// Check system call return value for NULL and handle error
#define SYSCN(v, c, m) if ((v = c) == NULL) { perror(m); exit(errno); }

// Custom error handling
#define SUCCESS (Error){0, "Success"}
#define PORT_ERROR (Error){1, "Port already in use or invalid"}
#define SERVER_NAME_ERROR (Error){2, "Invalid server name"}
#define NEGATIVE_PARAM_ERROR (Error){3, "Negative parameter passed - check your input"}
#define WRONG_PARAMS_ERROR (Error){4, "Wrong parameters passed\nUsage: ./paroliere_srv server_name server_port [--matrici matrix_file] [--durata game_duration] [--seed randomization_seed] [--diz dictionary_file]"}
#define CONFIG_ERROR_BACKLOG (Error){5, "Configuration file - socket_backlog not found or invalid"}
#define FILE_OPEN_ERROR (Error){6, "Error opening file"}
#define FILE_SIZE_ERROR (Error){7, "Error: Insufficient data in file"}
#define MEMORY_ALLOCATION_ERROR (Error){8, "Error: Memory allocation failed"}
#define LOCK_MUTEX_ERROR (Error){9, "Error: Mutex lock failed"}
#define MAX_PLAYERS_ERROR (Error){10, "Error: Maximum number of players reached"}

typedef struct {
    int code;
    const char *message;
} Error;

#endif
