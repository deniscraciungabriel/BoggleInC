#ifndef MATRIX_HANDLER_H
#define MATRIX_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "macros.h"
#include "player_handler.h"
#include "utils.h"
#include "server.h"

#define MATRIX_SIZE 4
#define MATRIX_BYTES (MATRIX_SIZE * MATRIX_SIZE * sizeof(Cell))
#define MAX_WORD_LENGTH 16
#define ALPHABET_SIZE 22  // 21 Italian letters + 1 for "Qu"
#define BUFFER_SIZE 100

// Structure to represent a position in the matrix
typedef struct {
    int row;
    int col;
} Position;

// Structure to store all positions of a specific letter or "Qu"
typedef struct {
    Position positions[MATRIX_SIZE * MATRIX_SIZE];
    int count;
} LetterPositions;

typedef struct {
    char letter[3]; // Qu + null terminator
} Cell;

typedef struct TrieNode{
    struct TrieNode *children[ALPHABET_SIZE];
    bool end_of_word;
} TrieNode;

// Function prototypes
void init_matrix_from_file(Cell matrix[MATRIX_SIZE][MATRIX_SIZE], const char* fileName, int iteration);
void init_matrix_random(Cell matrix[MATRIX_SIZE][MATRIX_SIZE]);
void send_matrix_to_all(PlayerArray *players_array, Cell matrix[MATRIX_SIZE][MATRIX_SIZE]);
bool is_word_in_matrix(Cell matrix[MATRIX_SIZE][MATRIX_SIZE], char* word);
bool is_word_in_dictionary(TrieNode* dictionary_root, char* word);
bool form_word(const char* word, int index, int prev_row, int prev_col, LetterPositions* letter_hash, bool used[MATRIX_SIZE][MATRIX_SIZE]);
void print_matrix(Cell matrix[MATRIX_SIZE][MATRIX_SIZE]);
TrieNode* init_dictionary(const char *filename);

#endif /* MATRIX_HANDLER_H */
