#ifndef PLAYER_HANDLER_H
#define PLAYER_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "macros.h"

#define MAX_USERNAME_LENGTH 10
#define MAX_PLAYERS 32
#define INITIAL_PLAYER_CAPACITY 5
#define INITIAL_WORD_CAPACITY 10

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    int score;
    char** words;
    int word_size;
    int word_capacity;
    int fd;
    pthread_t scorer_tid;
    pthread_t tid;
} Player;


typedef struct {
    char username[MAX_USERNAME_LENGTH];
    int score;
} PlayerScore;

typedef struct {
    PlayerScore* players;
    int size;
} ScoresList;


typedef struct {
    Player* players;
    int size;
    int capacity;
} PlayerArray;


ScoresList* create_player_score_list(int length);
void free_player_score_list(ScoresList* list);
int sort_helper_players(const void* a, const void* b);
void remove_player(PlayerArray* registry, int fd);
bool is_username_taken(PlayerArray* registry, const char* username);
bool has_player_used_word(Player* player, char* word);
void update_player_score(Player* player, int points_gained);
void add_word_to_player(Player* player, const char* word);
Player* add_player(PlayerArray* registry, int fd, pthread_t tid, const char* username);
Player* find_player(PlayerArray* registry, int fd);
PlayerScore* add_player_score(ScoresList* score_list, char* username, int score);
PlayerArray* create_player_registry();

#endif
