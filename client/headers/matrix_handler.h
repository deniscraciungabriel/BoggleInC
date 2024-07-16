#ifndef MATRIX_HANDLER_H
#define MATRIX_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MATRIX_SIZE 4
#define MATRIX_BYTES (MATRIX_SIZE * MATRIX_SIZE * sizeof(Cell))
#define MAX_WORD_LENGTH 16
#define MAX_SERVER_RESPONSE_LENGTH 100

typedef struct {
    char letter[3];
} Cell;

// Function prototypes
void init_empty_matrix(Cell matrix[MATRIX_SIZE][MATRIX_SIZE]);
void print_matrix(Cell matrix[MATRIX_SIZE][MATRIX_SIZE]);
// void cleanMatrix(Cell matrix[MATRIX_SIZE][MATRIX_SIZE]);

#endif
