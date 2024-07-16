#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For _exit

#define BOLD "\033[1m"
#define RESET "\033[0m"
#define BOX_HORIZONTAL "\u2500"
#define BOX_VERTICAL "\u2502"
#define BOX_TOP_LEFT "\u250C"
#define BOX_TOP_RIGHT "\u2510"
#define BOX_BOTTOM_LEFT "\u2514"
#define BOX_BOTTOM_RIGHT "\u2518"
#define BOX_T_DOWN "\u252C"
#define BOX_T_UP "\u2534"
#define BOX_T_RIGHT "\u251C"
#define BOX_T_LEFT "\u2524"
#define BOX_CROSS "\u253C"

// This is modified to increment the chance of finding italian words in a small 4x4 matrix
// I incremented the frequency of the top 5 most common letters in italian, and after playing around with the game, it seems much better
// https://www.sttmedia.com/characterfrequency-italian
#define ITALIAN_ALPHABET "AAAABCDEEEEFGHIIIILMNNNNOOOOPQRSTUVZ" 
#define ITALIAN_ALPHABET_SIZE (sizeof(ITALIAN_ALPHABET) - 1)

#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"

void trim_newline(char *str);
void handle_error(Error err);
int parse_positive_int(const char *str);
float parse_position_float(const char *str);
char get_random_letter();

#endif