#include "matrix_handler.h"
#include "macros.h"
#include "utils.h"


void print_matrix(Cell matrix[MATRIX_SIZE][MATRIX_SIZE]) {

    // Top part
    printf("   " BOX_TOP_LEFT);
    for (int i = 0; i < MATRIX_SIZE; i++) {
        printf(BOX_HORIZONTAL BOX_HORIZONTAL BOX_HORIZONTAL); // 3 is optimal number of spaces
        if (i < MATRIX_SIZE - 1) printf(BOX_T_DOWN); // leaving space for right border 
    }
    printf(BOX_TOP_RIGHT "\n");

    // Middle
    for (int i = 0; i < MATRIX_SIZE; i++) {
        printf(" %d " BOX_VERTICAL, i + 1); // n of row + initial vertical separator
        for (int j = 0; j < MATRIX_SIZE; j++) {
            // Number in cell (takes 1 space in horizontal, we are now in the middle since the top is already taken)
            if (strcmp(matrix[i][j].letter, "Qu") == 0) {
                printf(BOLD " %s" RESET, matrix[i][j].letter);
            } else {
                printf(BOLD " %s " RESET, matrix[i][j].letter);
            }
            printf(BOX_VERTICAL);
        }
        printf("\n");

        // bottom part of the middle: top part is taken, middle is number, third bottom is the bottom part of the cell
        if (i < MATRIX_SIZE - 1) {
            printf("   " BOX_T_RIGHT); // unite with top line, then go right
            for (int j = 0; j < MATRIX_SIZE; j++) {
                printf(BOX_HORIZONTAL BOX_HORIZONTAL BOX_HORIZONTAL); // usual 3 spaces
                if (j < MATRIX_SIZE - 1) printf(BOX_CROSS); // close all gaps
            }
            printf(BOX_T_LEFT "\n"); // unite with top line and close the gap on the left
        }
    }

    // Bottom
    printf("   " BOX_BOTTOM_LEFT);
    for (int i = 0; i < MATRIX_SIZE; i++) {
        printf(BOX_HORIZONTAL BOX_HORIZONTAL BOX_HORIZONTAL);
        if (i < MATRIX_SIZE - 1) printf(BOX_T_UP);
    }
    printf(BOX_BOTTOM_RIGHT "\n");

    // column numbers
    printf("\n    ");
    for (int i = 0; i < MATRIX_SIZE; i++) {
        printf(" %d  ", i + 1);
    }
    printf("\n\n");
}

void init_empty_matrix(Cell matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            strcpy(matrix[i][j].letter, " ");  // Set each cell to a single space
        }
    }
}