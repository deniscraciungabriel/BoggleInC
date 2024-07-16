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

#define PRE_GAME_DURATION 10 // seconds
#define GAME_DURATION 60 // seconds

#define MAX_CSV_LENGTH 1024
#define MAX_BUFFER_SIZE 1024
#define MAX_MESSAGE_DATA_SIZE 1024

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
    char type;
    int size;
    char* data;
} Message;

typedef enum {
    WAITING_STATE,
    GAME_STATE
} GameState;

void init_server(char *server_name, int server_port, unsigned int randomization_seed, int game_length, char *matrix_file, char *dictionary_file);
void send_matrix_to_client(int client_fd);
void send_message_to_client(const Message *msg, int client_fd);
static void transition_to_game_state();
static void free_scores_list();
static void transition_to_waiting_state();

#endif
