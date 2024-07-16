#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For _exit

#define BOLD "\033[1m"
#define RESET "\033[0m"
#define CLEAR "\033[2J"
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

#define RED "\033[31m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define YELLOW "\033[33m"


void trim_newline(char *str);
void handle_error(Error err);
void print_title();

#endif