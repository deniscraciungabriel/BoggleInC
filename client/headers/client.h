#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "macros.h"
#include "matrix_handler.h"

#define MSG_OK 'K'
#define MSG_ERR 'E'
#define MSG_REGISTRA_UTENTE 'R'
#define MSG_MATRICE 'M'
#define MSG_TEMPO_PARTITA 'T'
#define MSG_TEMPO_ATTESA 'A'
#define MSG_PAROLA 'W'
#define MSG_PUNTI_FINALI 'F'
#define MSG_PUNTI_PAROLA 'P'

typedef struct {
    // char server_ip[16];
    int port;
    int backlog;
} Config;

typedef struct {
    Cell (*matrix)[MATRIX_SIZE];
    int client_fd;
    int* score;
    char* client_input;
    char* server_response;
    char* terminal_message;
    int* time_left;
    pthread_mutex_t thread_mutex;
} Thread;

typedef struct {
    char type;
    unsigned int size;
    char* data;
} Message;

void init_client();

#endif
