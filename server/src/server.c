#include "server.h"
#include "macros.h"
#include "utils.h"
#include "matrix_handler.h"
#include "player_handler.h"

#define MAX_CONF_LINE_LENGTH 64

// ---- GLOBAL VARS ----

// The game state is volatile to ensure it is always read from memory and not cached by the compiler.
volatile GameState game_state = WAITING_STATE;

// Initializing flags for game states and synchronization.
bool is_game_ended = false;
bool is_scores_list_ready = false;
bool is_csv_results_scoreboard_ready = false;
int match_duration; // This will store the duration of the game in seconds.
int game_iteration = 0; // Tracking the number of games played, used for matrix generation.
char* matrix_file_global; // This will hold the path to the file from which the matrix is generated.
char csv_result[MAX_CSV_LENGTH] = ""; // Buffer to store the CSV formatted final scores.

// Declaring condition variables and mutexes for synchronizing game state and player actions.
pthread_cond_t game_over_condition = PTHREAD_COND_INITIALIZER; 
pthread_cond_t scores_list_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t csv_results_condition = PTHREAD_COND_INITIALIZER; 
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;

ScoresList *scores_list = NULL; // Initializing the list of player scores.
TrieNode* dictionary_root = NULL; // This will point to the root of the Trie for dictionary lookups.
PlayerArray *players_array = NULL; // Array to keep track of players in the game.
// Defining the game matrix, which is a grid of letters used to form words.
Cell matrix[MATRIX_SIZE][MATRIX_SIZE];

// ---- FUNCTION DECLARATIONS ----

// This function is handling the alarm signal to switch the game state between waiting and playing.
static void alarm_handler() {
    pthread_mutex_lock(&state_mutex);

    if (game_state == WAITING_STATE) {
        transition_to_game_state(); // Switching to the game state.
    } else {
        transition_to_waiting_state(); // Switching to the waiting state.
    }

    pthread_mutex_unlock(&state_mutex);

    // Setting the next alarm based on the current game state.
    alarm(game_state == WAITING_STATE ? 20 : match_duration);
}

// This function is retrieving the remaining time left in the current game state.
unsigned int get_time_left() {
    pthread_mutex_lock(&time_mutex);
    unsigned int time_left = alarm(0); // Canceling the current alarm and getting the remaining time.
    if (time_left > 0) {
        alarm(time_left); // Resetting the alarm with the remaining time.
    }
    pthread_mutex_unlock(&time_mutex);
    return time_left;
}

// Serializing a Message structure into a buffer for network transmission.
int serialize_message(const Message *msg, char *buffer, size_t buffer_size) {
    if (!msg || !buffer || buffer_size < sizeof(char) + sizeof(int) + msg->size) {
        return -1; // Returning an error if the buffer size is insufficient.
    }

    int offset = 0;
    buffer[offset++] = msg->type; // Copying the message type.
    memcpy(buffer + offset, &msg->size, sizeof(int)); // Copying the message size.
    offset += sizeof(int);
    memcpy(buffer + offset, msg->data, msg->size); // Copying the message data.

    return offset + msg->size;
}

// Deserializing a buffer into a Message structure to understand the client's request.
Message* deserialize_message(const char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < sizeof(char) + sizeof(int)) {      
        return NULL; // Returning NULL if the buffer size is too small.
    }

    Message *msg = malloc(sizeof(Message));
    if (!msg) {
        return NULL;  // Returning NULL if memory allocation failed.
    }

    int offset = 0;
    msg->type = buffer[offset++]; // Reading the message type.
    memcpy(&msg->size, buffer + offset, sizeof(int)); // Reading the message size.
    offset += sizeof(int);

    if ((int)buffer_size < offset + msg->size) {
        free(msg);
        return NULL;  // Returning NULL if the buffer is smaller than expected.
    }

    msg->data = malloc(msg->size + 1);
    if (!msg->data) {
        free(msg);
        return NULL;  // Returning NULL if memory allocation failed.
    }

    memcpy(msg->data, buffer + offset, msg->size); // Reading the message data.
    msg->data[msg->size] = '\0';

    // Printing parsed message for debugging purposes.
    printf("Message received -> type: %c size: %d data: %s\n", msg->type, msg->size, msg->data);
    for (int i = 0; i < msg->size; i++) printf("%02x ", (unsigned char)msg->data[i]);
    printf("\n\n");

    return msg;
}

// Sending a serialized message to a client.
void send_message_to_client(const Message *msg, int client_fd) {
    char buffer[MAX_BUFFER_SIZE];
    
    // Serializing the message and getting the size.
    int msg_size = serialize_message(msg, buffer + sizeof(int), MAX_BUFFER_SIZE - sizeof(int));
    if (msg_size < 0) {
        fprintf(stderr, "Error during message serialization\n");
        return;
    }

    // Prepending the message size to the buffer.
    int total_size = msg_size + sizeof(int);
    memcpy(buffer, &msg_size, sizeof(int));

    int bytes;
    printf("Sending message to client %d\n", client_fd);
    printf("Message size: %d\n", msg_size);
    printf("Message data: %s\n", msg->data);
    printf("Message type: %c\n", msg->type);
    SYSC(bytes, write(client_fd, buffer, total_size), "Error during message sending");
}

// Sending the game matrix to a client.
void send_matrix_to_client(int client_fd) {
    Message response;

    Player* player_searched = find_player(players_array, client_fd);

    if (game_state == GAME_STATE && player_searched != NULL) {
        Message response = {
            .type = MSG_MATRICE,
            .data = (char *)matrix,
            .size = sizeof(matrix)
        };

        send_message_to_client(&response, client_fd);
    } else if (player_searched == NULL) {
        response.type = MSG_ERR;
        strcpy(response.data, "You're not registered yet\n");
        response.size = strlen(response.data);
        send_message_to_client(&response, client_fd);
    } else {
        response.type = MSG_ERR;
        strcpy(response.data, "Game hasn't started yet\n");
        response.size = strlen(response.data);
        send_message_to_client(&response, client_fd);
    }
}

// Sending the remaining time to a client.
void send_time_left_to_client(int client_fd) {
    char time_left_string[20];
    sprintf(time_left_string, "%d", get_time_left());

    Message response = {
        .type = game_state == GAME_STATE ? MSG_TEMPO_PARTITA : MSG_TEMPO_ATTESA,
        .data = strdup(time_left_string),
        .size = strlen(time_left_string)
    };

    send_message_to_client(&response, client_fd);
    free(response.data);
}

// Sending the remaining time to all clients.
void send_time_left_to_all(PlayerArray *players_array) {
    for (int i = 0; i < players_array->size; i++) {
        send_time_left_to_client(players_array->players[i].fd);
    }
}

// Helper thread loop for each player to handle game-related tasks.
void *player_helper_thread_loop(void *player_arg) {
    Player *player = (Player *)player_arg;

    while (1) {
        pthread_mutex_lock(&state_mutex);

        // Waiting until the game ends.
        while (!is_game_ended) {
            printf("Player %s is waiting for game to end\n", player->username);
            pthread_cond_wait(&game_over_condition, &state_mutex);
        }

        // Handling player disconnection.
        if (player->fd < 0) {
            free(player->words);
            free(player);

            pthread_mutex_unlock(&state_mutex);
            pthread_exit(NULL);
        }

        // Adding player's score to the scores list.
        add_player_score(scores_list, player->username, player->score);
        printf("Player %s has scored %d points\n", player->username, player->score);

        // Signaling the scores list condition if all players have finished.
        if (scores_list->size == players_array->size) {
            is_scores_list_ready = 1;
            pthread_cond_signal(&scores_list_condition);
        }

        // Waiting until the CSV results are ready.
        while (!is_csv_results_scoreboard_ready) {
            pthread_cond_wait(&csv_results_condition, &state_mutex);
        }

        // Sending the final scores to the player.
        Message response = {
            .type = MSG_PUNTI_FINALI,
            .data = strdup(csv_result),
            .size = strlen(csv_result)
        };

        send_message_to_client(&response, player->fd);
        free(response.data);

        pthread_mutex_unlock(&state_mutex);
    }

    return NULL;
}

// Handling player registration.
void handle_registration(Player *player, char *username) {
    Message response;
    response.data = (char*)malloc(1024);

    Player *player_searched = find_player(players_array, player->fd);
    if (player_searched != NULL) {
        response.type = MSG_ERR;
        strcpy(response.data, "Player already registered");
    } else if (is_username_taken(players_array, username)) {
        response.type = MSG_ERR;
        strcpy(response.data, "Invalid username");
    } else {
        response.type = MSG_OK;
        strcpy(response.data, "Registration successful");
        Player *new_player = add_player(players_array, player->fd, pthread_self(), username);

        if (new_player == NULL) {
            response.type = MSG_ERR;
            response.size = strlen("Failed to register player: lobby is full :(\n");
            response.data = (char*)malloc(response.size);
            strcpy(response.data, "Failed to register player: lobby is full :(\n");
            send_message_to_client(&response, player->fd);
            return;
        }

        if (game_state == GAME_STATE) {
            send_matrix_to_client(player->fd);
        }

        send_time_left_to_client(player->fd);

        // Creating a helper thread for the player.
        pthread_create(&new_player->scorer_tid, NULL, player_helper_thread_loop, new_player);
    }

    response.size = strlen(response.data);
    send_message_to_client(&response, player->fd);
    free(response.data);
}

// Handling word submission by players.
void handle_word_submission(Player *player, const char *word) {
    Message response;
    response.data = malloc(MAX_MESSAGE_DATA_SIZE);
    
    printf("Player with username %s submitted word %s\n", player->username, word);

    char *word_lowercase = strdup(word);
    for (int i = 0; word_lowercase[i]; i++) {
        word_lowercase[i] = tolower(word_lowercase[i]);
    }

    Player *player_searched = find_player(players_array, player->fd);

    if (player_searched == NULL) {
        response.type = MSG_ERR;
        strcpy(response.data, "You're not registered yet");
    } else if (game_state == WAITING_STATE) {
        response.type = MSG_ERR;
        strcpy(response.data, "Waiting for match to start");
    } else if (!is_word_in_matrix(matrix, word_lowercase) || !is_word_in_dictionary(dictionary_root, word_lowercase)) {
        response.type = MSG_ERR;
        strcpy(response.data, "Invalid word");
    } else if (has_player_used_word(player_searched, word_lowercase)) {
        response.type = MSG_PUNTI_PAROLA;
        strcpy(response.data, "0");
    } else {
        response.type = MSG_PUNTI_PAROLA;
        int points_gained = strlen(word_lowercase);
        update_player_score(player_searched, points_gained);
        add_word_to_player(player_searched, word_lowercase);
        sprintf(response.data, "%d", points_gained);
    }

    response.size = strlen(response.data);
    send_message_to_client(&response, player->fd);
    free(response.data);
    free(word_lowercase);
}

// Main player handler function.
void* handle_player(void* player_arg) {
    Player *player = (Player *)player_arg;
    char* buffer = (char*)malloc(MAX_BUFFER_SIZE);
    int bytes_read;
    
    while ((bytes_read = read(player->fd, buffer, MAX_BUFFER_SIZE)) > 0) {
        Message *msg = deserialize_message(buffer, bytes_read);
        if (!msg) {
            fprintf(stderr, "Error deserializing message from client");
            continue;
        }

        switch (msg->type) {
            case MSG_REGISTRA_UTENTE:
                handle_registration(player, msg->data);
                break;
            case MSG_MATRICE:
                send_matrix_to_client(player->fd);
                send_time_left_to_client(player->fd);
                break;
            case MSG_PAROLA:
                handle_word_submission(player, msg->data);
                break;
            default:
                fprintf(stderr, "Unknown message type from client %d\n", player->fd);
                break;
        }

        free(msg->data);
        free(msg);
    }

    printf("Client disconnected\n");
    remove_player(players_array, player->fd);
    close(player->fd);
    pthread_exit(NULL);
}

// Resetting game variables for a new round.
static void reset_game_variables() {
    is_game_ended = false;
    is_scores_list_ready = false;
    is_csv_results_scoreboard_ready = false;
}

// Freeing the scores list memory.
static void free_scores_list() {
    if (scores_list) {
        free_player_score_list(scores_list);
        scores_list = NULL;
    }
}

// Transitioning the game to the active state.
static void transition_to_game_state() {
    game_state = GAME_STATE;
    printf("\n" BOLD RED "GAME IS ON!!\n\n" RESET);

    // Resetting player scores and words for a new game.
    for (int i = 0; i < players_array->size; i++) {
        players_array->players[i].score = 0;
        players_array->players[i].word_size = 0;
        players_array->players[i].words = malloc(INITIAL_WORD_CAPACITY * sizeof(char*));
    }

    // Generating a new matrix for the game.
    matrix_file_global ? init_matrix_from_file(matrix, matrix_file_global, game_iteration) : init_matrix_random(matrix);
    send_matrix_to_all(players_array, matrix);
    send_time_left_to_all(players_array);
    reset_game_variables();
    free_scores_list();
}

// Initializing the scores list for the new game.
static void initialize_scores_list() {
    scores_list = create_player_score_list(players_array->size);
}

// Notifying threads to send final results.
static void trigger_send_final_results() {
    is_game_ended = true;
    pthread_cond_broadcast(&game_over_condition);
}

// Transitioning the game to the waiting state.
static void transition_to_waiting_state() {
    game_state = WAITING_STATE;
    printf("\n" BOLD BLUE "TIME FOR A BREAK! SEE YOU IN 1 MIN\n\n" RESET);

    if (players_array != NULL && players_array->size > 0) {
        initialize_scores_list();
        trigger_send_final_results();
        send_time_left_to_all(players_array);
    }

    game_iteration++;
}

// Loading configuration from a file.
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

// Looping for the scorer thread to handle final scoring.
void* scorer_thread_loop() {
    int ret;

    while (1) {
        ret = pthread_mutex_lock(&state_mutex); // Locking mutex because of players_array.
        if (ret != 0) {
            fprintf(stderr, "Failed to lock mutex: %s\n", strerror(ret));
            continue;
        }

        while (!is_scores_list_ready) {
            ret = pthread_cond_wait(&scores_list_condition, &state_mutex);
            if (ret != 0) {
                fprintf(stderr, "Condition wait failed: %s\n", strerror(ret));
                pthread_mutex_unlock(&state_mutex);
                continue;
            }
        }

        is_game_ended = 0;
        is_scores_list_ready = 0;

        // Sorting scores in descending order.
        qsort(scores_list->players, scores_list->size, sizeof(Player), sort_helper_players);

        // Building the CSV message with final scores.
        int remaining_space = MAX_CSV_LENGTH;
        char* csv_ptr = csv_result;
        csv_result[0] = '\0';

        for (int i = 0; i < scores_list->size && remaining_space > 1; i++) {
            int written = snprintf(csv_ptr, remaining_space, "%s,%d,", 
                                   scores_list->players[i].username, 
                                   scores_list->players[i].score);
            
            if (written >= remaining_space) {
                // Truncating if we've run out of space.
                break;
            }
            
            csv_ptr += written;
            remaining_space -= written;
        }

        // Adding null terminator instead of last comma if we added any entries.
        if (csv_result[0] != '\0') {
            *(csv_ptr - 1) = '\0';
        }

        printf("Final results: %s\n", csv_result);

        is_csv_results_scoreboard_ready = true;
        ret = pthread_cond_broadcast(&csv_results_condition);
        if (ret != 0) {
            fprintf(stderr, "Broadcast failed: %s\n", strerror(ret));
        }

        ret = pthread_mutex_unlock(&state_mutex);
        if (ret != 0) {
            fprintf(stderr, "Failed to unlock mutex: %s\n", strerror(ret));
        }
    }

    return NULL;
}

// Initializing the server and starting to listen for connections.
void init_server(char *server_name, int server_port, unsigned int randomization_seed, int game_length, char *matrix_file, char *dictionary_file) {
    int server_socket_fd, client_fd, last_ret_value;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    match_duration = game_length; // Setting game duration.
    matrix_file_global = matrix_file;

    // Loading configuration file.
    Config config;
    Error err = load_config("config.txt", &config);
    handle_error(err);
    
    // Seeding the random number generator.
    srand(randomization_seed);

    players_array = create_player_registry();

    // Setting up server name in server_addr->sin_addr.
    if (strcmp(server_name, "localhost") == 0) {
        server_name = "127.0.0.1";
    }
    SYSC(last_ret_value, inet_pton(AF_INET, server_name, &server_addr.sin_addr), "Invalid address");

    if (last_ret_value <= 0) {
        handle_error(SERVER_NAME_ERROR);
    }

    // Creating the server socket.
    SYSC(server_socket_fd, socket(AF_INET, SOCK_STREAM, 0), "Socket creation failed");

    // Setting up the address.
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listening on all interfaces.
    server_addr.sin_port = htons(server_port);  // Converting to network byte order.

    // Binding the server socket to the address.
    if(bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        if (errno == EADDRINUSE) {
            handle_error(PORT_ERROR);
        } else {
            perror("Bind failed");
            exit(errno);
        }
    }

    // Initializing the dictionary.
    dictionary_root = init_dictionary(dictionary_file ? dictionary_file : "./data/dictionary_ita.txt");

    // matrix_file ? init_matrix_from_file(matrix, matrix_file, game_iteration) : init_matrix_random(matrix);

    // Starting to listen for incoming connections.
    SYSC(last_ret_value, listen(server_socket_fd, config.backlog), "Listen failed");

    printf("Server listening on port %d\n", server_port);

    // Setting up signal handler and initial alarm.
    signal(SIGALRM, alarm_handler);
    printf(BOLD GREEN "\nGet ready for the game! %d seconds of waiting... feel free to register or ask for help\n" RESET, PRE_GAME_DURATION);
    alarm(PRE_GAME_DURATION); // Giving a small pre-game time so users can register and get ready.

    // Starting the scoring thread.
    pthread_t scorer_thread;
    pthread_create(&scorer_thread, NULL, scorer_thread_loop, NULL);

    // Main loop to accept incoming connections.
    while (1) {
        client_addr_len = sizeof(client_addr);
        SYSC(client_fd, accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_addr_len), "Accepting client failed");
        printf("Client %d connected\n", client_fd);

        // Initializing the player.
        Player* player;
        player = malloc(sizeof(Player));
        player->fd = client_fd;

        pthread_create(&player->tid, NULL, handle_player, (void *)player);
    }

    // Closing the server socket.
    SYSC(last_ret_value, close(server_socket_fd), "Failed closing the socket");
}
