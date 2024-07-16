#include "client.h"
#include "macros.h"
#include "utils.h"
#include "matrix_handler.h"

#define MAX_CONF_LINE_LENGTH 64

// Function to load configuration from a file.
Error load_config(const char *filename, Config *config) {
    FILE *file;
    SYSCN(file, fopen(filename, "r"), "Failed to open config file");

    char line[MAX_CONF_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        trim_newline(line);
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");

        if (strcmp(key, "socket_backlog") == 0) {
            if (value != NULL && strlen(value) > 0 && value[0] != '-')
                config->backlog = atoi(value);
            else {
                fclose(file);
                return CONFIG_ERROR_BACKLOG;
            }
        }
    }

    fclose(file);
    return SUCCESS;
}

// Function to interpret a message from the server.
static Message interpret_message(const char* buffer) {
    Message message = {0};
    message.type = buffer[0];
    memcpy(&message.size, buffer + sizeof(char), sizeof(int));
    message.data = malloc(message.size + 1);
    if (!message.data) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(message.data, buffer + sizeof(char) + sizeof(int), message.size);
    message.data[message.size] = '\0';
    return message;
}

// Resetting player's score, protected by a mutex.
void reset_player_score(Thread* thread) {
    pthread_mutex_lock(&thread->thread_mutex);
    *thread->score = 0;
    pthread_mutex_unlock(&thread->thread_mutex);
}

// Function to serialize a message for sending to the server.
static int pack_message(const Message* message, char** buffer) {
    int message_size = sizeof(char) + sizeof(int) + message->size;
    *buffer = malloc(message_size);
    if (!*buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    (*buffer)[0] = message->type;
    memcpy(*buffer + sizeof(char), &(message->size), sizeof(int));
    memcpy(*buffer + sizeof(char) + sizeof(int), message->data, message->size);
    return message_size;
}

// Function to interpret and store the scoreboard.
static void interpret_and_store_scoreboard(char* terminal_message, const char* message_data) {
    char* rest = strdup(message_data);
    if (!rest) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strcpy(terminal_message, "FINAL SCOREBOARD\n");
    char* token;
    int position = 1;

    // Parsing the message data.
    token = strtok(rest, ",");

    while (token != NULL) {
        char* player_username = token;
        token = strtok(NULL, ",");
        if (token == NULL) break;

        int score = atoi(token);

        char player_score[64];
        snprintf(player_score, sizeof(player_score), 
                 position == 1 ? YELLOW "%d° %s - %d\n" RESET : BLUE "%d° %s - %d\n" RESET, 
                 position, player_username, score);
        strcat(terminal_message, player_score);
        position++;

        token = strtok(NULL, ",");
    }
    
    free(rest);
}

// Function to handle a received message and update the client's state accordingly.
static void handle_received_message(Message* message, Thread* thread) {
    pthread_mutex_lock(&thread->thread_mutex);
    switch (message->type) {
        case MSG_MATRICE:
            memcpy(thread->matrix, message->data, message->size);
            thread->terminal_message[0] = '\0';
            break;
        case MSG_PUNTI_PAROLA:
            *thread->score += atoi(message->data);
            if (atoi(message->data) > 0) {
                sprintf(thread->terminal_message, GREEN "Nice! +%d" RESET, atoi(message->data));
            } else if (atoi(message->data) == 0) {
                strcpy(thread->terminal_message, GREEN "Already guessed! +0" RESET);
            } else {
                strcpy(thread->terminal_message, RED "Invalid word" RESET);
            }
            break;
        case MSG_TEMPO_PARTITA:
            *thread->time_left = atoi(message->data);
            break;
        case MSG_TEMPO_ATTESA:
            *thread->time_left = atoi(message->data);
            strcpy(thread->terminal_message, "Waiting for match to start");
            init_empty_matrix(thread->matrix);
            break;
        case MSG_PUNTI_FINALI:
            init_empty_matrix(thread->matrix);
            interpret_and_store_scoreboard(thread->terminal_message, message->data);
            reset_player_score(thread);
            break;
        default:
            strcpy(thread->terminal_message, message->data);
            break;
    }

    pthread_mutex_unlock(&thread->thread_mutex);
}

// Function to display the game GUI, showing the current game state.
static void show_game_GUI(const Thread* messages_thread) {
    if (!messages_thread) {
        fprintf(stderr, "Thread data is not available\n");
        return;
    }

    printf(CLEAR);
    print_title("PAROLIERE CLIENT");

    if (messages_thread->matrix) {
        print_matrix(messages_thread->matrix);
    }

    if (*messages_thread->time_left != 0) {
        printf(BOLD BLUE"Time left: %d"RESET, *messages_thread->time_left);
    } else {
        printf(BOLD BLUE"Check the time using the command 'matrice'"RESET);
    }

    if (messages_thread->score) {
        printf(BOLD GREEN"\nScore: %d\n"RESET, *messages_thread->score);
    }

    print_title("Type 'fine' to exit");

    if (messages_thread->terminal_message && messages_thread->terminal_message[0]) {
       printf(BLUE"\n%s\n"RESET, messages_thread->terminal_message);
    }

    printf(YELLOW "\nPAROLIERE CLIENT >> " RESET);
    fflush(stdout);
}

// Thread function to handle incoming messages from the server.
void* handle_messages_thread(void* arg) {
    Thread* messages_thread = (Thread*)arg;
    if (!messages_thread) {
        fprintf(stderr, "Thread data is not available\n");
        return NULL;
    }

    while (1) {
        // Read the message length first.
        int message_length;
        ssize_t bytes_read = read(messages_thread->client_fd, &message_length, sizeof(int));
        if (bytes_read <= 0) {
            fprintf(stderr, "Error reading message length from server or connection closed\n");
            handle_error(SERVER_CLOSED_ERROR);
            break;
        }

        // Reading the actual message based on the length.
        bytes_read = read(messages_thread->client_fd, messages_thread->server_response, message_length);
        if (bytes_read <= 0) {
            fprintf(stderr, "Error reading message from server or connection closed\n");
            handle_error(SERVER_CLOSED_ERROR);
            break;
        }

        // Ensuring null-termination if necessary.
        if (bytes_read < MAX_SERVER_RESPONSE_LENGTH) {
            messages_thread->server_response[bytes_read] = '\0';
        }

        // Interpret and handle the message.
        Message message = interpret_message(messages_thread->server_response);
        handle_received_message(&message, messages_thread);
        free(message.data);
        show_game_GUI(messages_thread);
    }

    return NULL;
}

// Function to process a command entered by the user.
static void execute_command(const char* command, const char* content, Thread* thread) {
    Message message = {0};
    char* buffer = NULL;

    if (strcmp(command, "aiuto") == 0) {
        strcpy(thread->terminal_message, "Comandi disponibili:\naiuto\nregistra_utente <username>\nmatrice\np <parola>\nfine\n");
        show_game_GUI(thread);
        return;
    }

    if (!content) {
        fprintf(stderr, "Content is null\n");
        return;
    }

    if (strcmp(command, "registra_utente") == 0) {
        message.type = MSG_REGISTRA_UTENTE;
    } else if (strcmp(command, "matrice") == 0) {
        message.type = MSG_MATRICE;
    } else if (strcmp(command, "p") == 0) {
        message.type = MSG_PAROLA;
    } else {
        strcpy(thread->terminal_message, "Comando non riconosciuto. Digita 'aiuto' per vedere i comandi disponibili.\n");
        show_game_GUI(thread);
        return;
    }

    message.size = strlen(content);
    message.data = (char*)content;
    for (int i = 0; message.data[i]; i++) {
        message.data[i] = tolower(message.data[i]);
    }
    int message_size = pack_message(&message, &buffer);

    if (write(thread->client_fd, buffer, message_size) != message_size) {
        fprintf(stderr, "Failed to send message to server\n");
    }
    free(buffer);
}

// Function to send a message based on user input.
void send_message(void* arg) {
    Thread* thread = (Thread*)arg;
    char command[32] = {0};
    char content[32] = {0};

    if (sscanf(thread->client_input, "%31s%*c%31[^\n]", command, content) < 1) {
        fprintf(stderr, "Failed to parse client input\n");
        return;
    }

    pthread_mutex_lock(&thread->thread_mutex);
    execute_command(command, content, thread);
    pthread_mutex_unlock(&thread->thread_mutex);
}

// Function to initialize the client and start the connection.
void init_client(char* server_name, int server_port) {
    // Load configuration file.
    Config config;
    Error err = load_config("config.txt", &config);
    handle_error(err);

    // Matrix will be stored here later on.
    Cell matrix[MATRIX_SIZE][MATRIX_SIZE];

    // Socket file descriptor and last return value (will be used in system calls).
    int client_socket_fd, last_ret_value, client_score = 0, time_left = 0;
    // Structs for server and client addresses.
    struct sockaddr_in server_addr;

    // This will be used to store the client input.
    char* client_input;
    NEW_MEMORY_ALLOCATION(client_input, 64, "Failed to allocate memory for client input message");
    memset(client_input, 0, 64);

    // This will be used to store the server response.
    char* server_response;
    NEW_MEMORY_ALLOCATION(server_response, 64, "Failed to allocate memory for server response");
    memset(server_response, 0, 64);

    // This will be used to show the server response.
    char* terminal_message;
    NEW_MEMORY_ALLOCATION(terminal_message, 64, "Failed to allocate memory for terminal message");
    memset(terminal_message, 0, 64);

    // For reference on sockets: https://beej.us/guide/bgnet/html//index.html#slightly-advanced-techniques.
    // Socket Creation.
    SYSC(client_socket_fd, socket(AF_INET, SOCK_STREAM, 0), "socket creation failed");

    // Address setup.
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_port = htons(server_port);  // Converting to network byte order.
    server_addr.sin_family = AF_INET;
    if (strcmp(server_name, "localhost") == 0) {
        server_name = "127.0.0.1";
    }
    SYSC(last_ret_value, inet_pton(AF_INET, server_name, &server_addr.sin_addr), "Invalid address");
    printf("Waiting for connection... on %s:%d\n", server_name, server_port);

    // Connecting to the server.
    SYSC(last_ret_value, connect(client_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)), "connect to server failed");

    printf(GREEN "Connected to the server\n" RESET);

    // Setting up the thread.
    pthread_t message_thread_id;
    pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
    Thread messages_thread = {
        .matrix = matrix,
        .client_fd = client_socket_fd,
        .score = &client_score,
        .client_input = client_input,
        .server_response = server_response,
        .terminal_message = terminal_message,
        .time_left = &time_left
    };

    if (pthread_create(&message_thread_id, NULL, handle_messages_thread, (void *)&messages_thread) != 0) {
        free(client_input);
        free(server_response);
        free(terminal_message);
        close(client_socket_fd);
        handle_error(THREAD_CREATION_ERROR);
    }

    // The matrix starts empty, it will be filled by the server later on.
    init_empty_matrix(matrix);

    // Displaying the shell GUI for the game. It will be updated by the thread making it look like a real-time game.
    show_game_GUI(&messages_thread);

    // Main loop.
    while (1) {
        // User Input.
        if (scanf("%63[^\n]%*c", client_input) != 1) continue;

        // User Exit.
        if (strcmp(client_input, "fine") == 0) break;

        send_message((void *)&messages_thread);
    }

    // Stopping and removing thread.
    pthread_cancel(message_thread_id);
    pthread_join(message_thread_id, NULL);
    pthread_mutex_destroy(&thread_mutex);

    // Freeing memory.
    free(client_input);
    free(server_response);
    free(terminal_message);

    // Closing the server socket.
    SYSC(last_ret_value, close(client_socket_fd), "Failed closing the socket");
}