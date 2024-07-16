#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "player_handler.h"
#include "utils.h"
#include "macros.h"

ScoresList* create_player_score_list(int length) {
    ScoresList* list = malloc(sizeof(PlayerScore));

    list->players = malloc(length * sizeof(PlayerScore));
    list->size = 0;

    return list;
}

PlayerScore* add_player_score(ScoresList* score_list, char* username, int score) {
    
    // Initialize new player
    PlayerScore *newPlayer = (PlayerScore *)malloc(sizeof(Player));
    strcpy(newPlayer->username, username);
    newPlayer->score = score;

    // Add the new player to the list
    score_list->players[score_list->size++] = *newPlayer;

    // no need to free the memory allocated for the name because it's copied
    return newPlayer;
}

void free_player_score_list(ScoresList* list) {
    free(list->players);
    free(list);
}


PlayerArray* create_player_registry() {
    PlayerArray* registry = malloc(sizeof(PlayerArray));
    if (!registry) {
        fprintf(stderr, "Failed to allocate memory for PlayerArray\n");
        exit(EXIT_FAILURE);
    }
    registry->players = malloc(INITIAL_PLAYER_CAPACITY * sizeof(Player));
    if (!registry->players) {
        fprintf(stderr, "Failed to allocate memory for players array\n");
        free(registry);
        exit(EXIT_FAILURE);
    }
    registry->size = 0;
    registry->capacity = INITIAL_PLAYER_CAPACITY;
    return registry;
}

void expand_player_registry(PlayerArray* registry) {
    int new_capacity = registry->capacity * 2;
    Player* new_players = realloc(registry->players, new_capacity * sizeof(Player));
    if (!new_players) {
        fprintf(stderr, "Failed to expand player registry\n");
        exit(EXIT_FAILURE);
    }
    registry->players = new_players;
    registry->capacity = new_capacity;
}

Player* add_player(PlayerArray* registry, int fd, pthread_t tid, const char* username) {

    if (registry->size == MAX_PLAYERS) {
        fprintf(stderr, "Player tried to register - LIMIT REACHED\n");
        return NULL;
    }

    if (registry->size == registry->capacity) {
        expand_player_registry(registry);
    }
    
    Player* player = &registry->players[registry->size++];
    player->fd = fd;
    player->tid = tid;
    strncpy(player->username, username, MAX_USERNAME_LENGTH - 1);
    player->username[MAX_USERNAME_LENGTH - 1] = '\0';
    player->score = 0;
    player->words = malloc(INITIAL_WORD_CAPACITY * sizeof(char*));
    player->word_size = 0;
    player->word_capacity = INITIAL_WORD_CAPACITY;

    printf("Player %s registered with fd %d\n", player->username, player->fd);
    // print all the players in the registry
    for (int i = 0; i < registry->size; i++) {
        printf("Player %d: %s\n", i, registry->players[i].username);
    }
    
    return player;
}

void remove_player(PlayerArray* registry, int fd) {
    for (int i = 0; i < registry->size; i++) {
        if (registry->players[i].fd == fd) {
            for (int j = 0; j < registry->players[i].word_size; j++) {
                free(registry->players[i].words[j]);
            }
            free(registry->players[i].words);
            
            memmove(&registry->players[i], &registry->players[i+1], 
                    (registry->size - i - 1) * sizeof(Player));
            registry->size--;
            return;
        }
    }
}

Player* find_player(PlayerArray* registry, int fd) {
    for (int i = 0; i < registry->size; i++) {
        if (registry->players[i].fd == fd) {
            return &registry->players[i];
        }
    }
    return NULL;
}

void update_player_score(Player* player, int points_gained) {
    if (player) {
        player->score = player->score + points_gained;
    }
}

bool is_username_taken(PlayerArray* registry, const char* username) {
    for (int i = 0; i < registry->size; i++) {
        if (strcmp(registry->players[i].username, username) == 0) {
            return true;
        }
    }
    return false;
}

void add_word_to_player(Player* player, const char* word) {
    if (player->word_size == player->word_capacity) {
        player->word_capacity *= 2;
        player->words = realloc(player->words, player->word_capacity * sizeof(char*));
        if (!player->words) {
            fprintf(stderr, "Failed to expand word list for player\n");
            exit(EXIT_FAILURE);
        }
    }
    
    player->words[player->word_size] = strdup(word);
    if (!player->words[player->word_size]) {
        fprintf(stderr, "Failed to duplicate word\n");
        exit(EXIT_FAILURE);
    }
    player->word_size++;
}

bool has_player_used_word(Player* player, char* word) {
    for (int i = 0; i < player->word_size; i++) {
        if (strcmp(player->words[i], word) == 0) {
            return true;
        }
    }
    return false;
}

int sort_helper_players(const void* a, const void* b) {
    Player* player_a = (Player*)a;
    Player* player_b = (Player*)b;
    return player_b->score - player_a->score;
}

void sort_players_by_score(PlayerArray* registry) {
    qsort(registry->players, registry->size, sizeof(Player), sort_helper_players);
}