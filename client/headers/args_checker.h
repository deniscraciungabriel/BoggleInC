#ifndef ARGS_CHECKER_H
#define ARGS_CHECKER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>

#include <macros.h>

#define DEFAULT_DURATION 180

void handle_args(int argc, char *argv[], char **serverName, int *serverPort);

#endif