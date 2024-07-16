#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <macros.h>
#include "utils.h"
#include "matrix_handler.h"

// Get rid of newline character at the end of a string
void trim_newline(char *str) {
    char *pos;
    if ((pos = strchr(str, '\n')) != NULL) {
        *pos = '\0';
    }
}

void handle_error(Error err) {
    if (err.code != 0) {        // 0 is success
        fprintf(stderr, "\n\nError [%d]: %s\n\n", err.code, err.message);
        exit(err.code);
    }
}

// Helper function to parse and validate positive integers
int parse_positive_int(const char *str) {
    char *endptr;
    long val = strtol(str, &endptr, 10);
    
    if (endptr == str || *endptr != '\0' || val <= 0) {
        handle_error(NEGATIVE_PARAM_ERROR);
    }
    
    return (int)val;
}

float parse_position_float(const char *str) {
    char *endptr;
    float val = strtof(str, &endptr);
    
    if (endptr == str || *endptr != '\0' || val < 0) {
        handle_error(NEGATIVE_PARAM_ERROR);
    }
    
    return val;
}

char get_random_letter() {
    return ITALIAN_ALPHABET[rand() % ITALIAN_ALPHABET_SIZE];
}
