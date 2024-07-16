#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <macros.h>
#include <utils.h>

// Get rid of newline character at the end of a string
void trim_newline(char *str) {
    char *pos;
    if ((pos = strchr(str, '\n')) != NULL) {
        *pos = '\0';
    }
}

void handle_error(Error err) {
    if (err.code != 0) {        // 0 is success
        fprintf(stderr, "Error [%d]: %s\n", err.code, err.message);
        exit(err.code);
    }
}

void print_title(char *title) {
    int title_length = strlen(title);
    int padding = 2; // padding on each side of the title
    int total_length = title_length + 2 * padding;

    // Top part
    printf("\n   " BOX_TOP_LEFT);
    for (int i = 0; i < total_length; i++) {
        printf(BOX_HORIZONTAL);
    }
    printf(BOX_TOP_RIGHT "\n");

    // Middle part
    printf("   " BOX_VERTICAL);
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
    printf(BOLD "%s" RESET, title);
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
    printf(BOX_VERTICAL "\n");

    // Bottom part
    printf("   " BOX_BOTTOM_LEFT);
    for (int i = 0; i < total_length; i++) {
        printf(BOX_HORIZONTAL);
    }
    printf(BOX_BOTTOM_RIGHT "\n\n\n");
}