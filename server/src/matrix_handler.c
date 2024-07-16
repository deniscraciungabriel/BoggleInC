#include "matrix_handler.h"
#include "macros.h"
#include "utils.h"


#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


// Helper function to get the index for a letter in our hash
// This function is case-insensitive and handles the special "Qu" case
int get_letter_index(const char *letter) {
    // Convert to uppercase for case-insensitivity
    char upper = toupper(letter[0]);
    
    // Special case for "Qu"
    if (upper == 'Q' && tolower(letter[1]) == 'u') return 21;  // Special index for "Qu"
    
    // Handle Italian alphabet (21 letters)
    // A B C D E F G H I L M N O P Q R S T U V Z
    static const char *italian_alphabet = "ABCDEFGHILMNOPQRSTUVZ";
    char *pos = strchr(italian_alphabet, upper);
    return (pos != NULL) ? (pos - italian_alphabet) : -1;
}

bool form_word(const char *word, int index, int prev_row, int prev_col, 
               LetterPositions *letter_hash, bool used[MATRIX_SIZE][MATRIX_SIZE]);

bool is_word_in_matrix(Cell matrix[MATRIX_SIZE][MATRIX_SIZE], char *word) {
    int word_len = strlen(word);
    
    // Quick check for word length validity
    if (word_len < 4 || word_len > MAX_WORD_LENGTH) return false;

    // Step 1: Create a hash of letter positions
    // This hash allows us to quickly look up all positions of a given letter
    LetterPositions letter_hash[ALPHABET_SIZE] = {0};
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            int index = get_letter_index(matrix[i][j].letter);
            if (index != -1) {
                letter_hash[index].positions[letter_hash[index].count].row = i;
                letter_hash[index].positions[letter_hash[index].count].col = j;
                letter_hash[index].count++;
            }
        }
    }

    // Step 2: Quick check if all letters of the word are present in the matrix
    // This allows us to quickly reject words that can't possibly be in the matrix
    for (int i = 0; i < word_len; i++) {
        int index;
        if (toupper(word[i]) == 'Q' && tolower(word[i+1]) == 'u') {
            index = 21;  // "Qu" case
            i++;  // Skip 'u'
        } else {
            index = get_letter_index(&word[i]);
        }
        if (index == -1 || letter_hash[index].count == 0) return false;
    }

    // Step 3: Try to form the word
    // If all letters are present, we now attempt to find a contiguous path
    bool used[MATRIX_SIZE][MATRIX_SIZE] = {{false}};
    return form_word(word, 0, -1, -1, letter_hash, used);
}

/**
 * Recursively attempts to form a word in the matrix.
 * 
 * This function is the core of the word-finding algorithm. It uses a depth-first
 * search approach to traverse the matrix and form words according to the game rules.
 * 
 * @param word The word we're trying to form in the matrix.
 * @param index Current index in the word we're processing.
 * @param prev_row Row of the previously used letter (-1 for start).
 * @param prev_col Column of the previously used letter (-1 for start).
 * @param letter_hash Hash table containing positions of each letter in the matrix.
 * @param used 2D array tracking which cells have been used in the current path.
 * 
 * @return true if the word can be formed, false otherwise.
 */
bool form_word(const char *word, int index, int prev_row, int prev_col, 
               LetterPositions *letter_hash, bool used[MATRIX_SIZE][MATRIX_SIZE]) {
    // Base case: we've successfully formed the entire word
    if (word[index] == '\0') return true;

    int letter_index;
    int skip = 1;  // How many characters to skip after processing current letter

    // Special handling for "Qu" digraph
    // This is crucial for games like "Boggle" where "Qu" is treated as a single unit
    if (toupper(word[index]) == 'Q' && tolower(word[index+1]) == 'u') {
        letter_index = 21;  // Arbitrary index chosen for "Qu" in our letter_hash
        skip = 2;  // Skip both 'Q' and 'u'
    } else {
        letter_index = get_letter_index(&word[index]);
    }

    // Iterate through all occurrences of the current letter in the matrix
    for (int i = 0; i < letter_hash[letter_index].count; i++) {
        int row = letter_hash[letter_index].positions[i].row;
        int col = letter_hash[letter_index].positions[i].col;

        // Check if this position is valid for use:
        // 1. The cell hasn't been used in our current word path
        // 2. It's either the first letter, or it's adjacent to the previous letter
        // Note: Adjacency allows only horizontal or vertical moves, not diagonal
        //       This is a key rule in our word-forming game
        if (!used[row][col] && 
            (index == 0 || 
             (abs(row - prev_row) <= 1 && abs(col - prev_col) <= 1 &&
              (row == prev_row || col == prev_col)))) {
            
            // Mark this cell as used in our current path
            // Critical: We treat "Qu" as occupying a single cell
            // This corrects a previous bug where "Qu" was incorrectly spread across two cells
            used[row][col] = true;
            
            // Recursive step: attempt to form the rest of the word
            // If successful, we've found a valid path and can return true
            if (form_word(word, index + skip, row, col, letter_hash, used)) {
                return true;  // Word successfully formed
            }
            
            // Backtracking step
            // If we couldn't form the word using this path, we unmark this cell
            // This allows us to explore other potential paths
            used[row][col] = false;
        }
    }

    // If we've exhausted all possibilities and haven't formed the word, it's not possible
    // This could be due to:
    // 1. The letter not being present in the matrix
    // 2. The letter being present, but not in a valid position to form the word
    return false;
}


// useful resources: 
// https://en.wikipedia.org/wiki/Box-drawing_characterss
// https://home.unicode.org/
// https://codereview.stackexchange.com/questions/276557/formatting-a-table-using-unicode-symbols-in-python
void print_matrix(Cell matrix[MATRIX_SIZE][MATRIX_SIZE]) {

    printf("\n" BOLD "  Paroliere Matrix:" RESET "\n\n");

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

void init_matrix_from_file(Cell matrix[MATRIX_SIZE][MATRIX_SIZE], const char* filename, int iteration) {
    printf("\nGenerating matrix...\n\n");

    FILE* file = fopen(filename, "r");
    if (!file) {
        handle_error(FILE_OPEN_ERROR);
    }

    char buffer[BUFFER_SIZE];
    int total_lines = 0;

    // Count total lines in the file
    while (fgets(buffer, sizeof(buffer), file)) {
        total_lines++;
    }

    // Calculate the starting line using the iteration and modulus operator
    int start_line = iteration % total_lines;

    // Reset file pointer to the beginning
    rewind(file);

    // Skip lines to reach the starting line
    for (int i = 0; i < start_line; i++) {
        if (!fgets(buffer, sizeof(buffer), file)) {
            handle_error(FILE_SIZE_ERROR);
        }
    }

    // Read the matrix data from the current line
    if (fgets(buffer, sizeof(buffer), file)) {
        int row = 0, col = 0;
        for (int i = 0; buffer[i] != '\0'; i++) {
            if (isalpha(buffer[i]) || (toupper(buffer[i]) == 'Q' && tolower(buffer[i+1]) == 'u')) {
                if (toupper(buffer[i]) == 'Q' && tolower(buffer[i+1]) == 'u') {
                    strncpy(matrix[row][col].letter, "Qu", 3);
                    i++;  // Skip u in Qu
                } else {
                    matrix[row][col].letter[0] = toupper(buffer[i]);
                    matrix[row][col].letter[1] = '\0';
                }
                col++;
                if (col == MATRIX_SIZE) {
                    row++;
                    col = 0;
                    if (row == MATRIX_SIZE) {
                        break;
                    }
                }
            }
        }
    } else {
        handle_error(FILE_SIZE_ERROR);
    }

    printf("Matrix generated from file '%s':\n", filename);
    print_matrix(matrix);

    fclose(file);
}


void init_matrix_random(Cell matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    printf("\nGenerating random matrix...\n\n");

    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            const char letter = get_random_letter();
            if (letter == 'Q'){
                strncpy(matrix[i][j].letter, "Qu", 3);
            } else {
                matrix[i][j].letter[0] = letter;
                matrix[i][j].letter[1] = '\0';
            }
        }
    }

    print_matrix(matrix); 
}

TrieNode* create_node(void) {
    TrieNode *node = calloc(1, sizeof(TrieNode));
    if (!node) {
       handle_error(MEMORY_ALLOCATION_ERROR);
    }
    return node;
}

void add_word(TrieNode *root, const char *word) {
    TrieNode* current = root;
    while (*word) {
        int index = tolower(*word) - 'a';
        if (index < 0 || index >= ALPHABET_SIZE) {
            word++;
            continue;  // Skip non-alphabetic characters
        }
        if (!current->children[index]) {
            current->children[index] = create_node();
        }
        current = current->children[index];
        word++;
    }
    current->end_of_word = true;
}

TrieNode* init_dictionary(const char *filename) {
    TrieNode *root = create_node();
    FILE *file = fopen(filename, "r");
    if (!file) {
        handle_error(FILE_OPEN_ERROR);
    }

    char word[256];
    size_t word_count = 0;

    printf("Loading dictionary...\n");
    while (fgets(word, sizeof(word), file)) {
        size_t len = strlen(word);
        if (len > 0 && word[len - 1] == '\n') {
            word[len - 1] = '\0';
        }
        add_word(root, word);
        word_count++;
    }

    fclose(file);
    printf("Dictionary loaded: %zu words\n", word_count);

    return root;
}

bool is_word_in_dictionary(TrieNode* dictionary_root, char* word) {
    TrieNode* current = dictionary_root;
    while (*word) {
        int index = tolower(*word) - 'a';
        if (index < 0 || index >= ALPHABET_SIZE) {
            word++;
            continue;  // Skip non-alphabetic characters
        }
        if (!current->children[index]) {
            return false;
        }
        current = current->children[index];
        word++;
    }
    return current && current->end_of_word;
}

void free_dictionary(TrieNode *node) {
    if (node) {
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            free_dictionary(node->children[i]);
        }
        free(node);
    }
}

void send_matrix_to_all(PlayerArray *players_array, Cell matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < players_array->size; i++) {
        send_matrix_to_client(players_array->players[i].fd);
    }
}


