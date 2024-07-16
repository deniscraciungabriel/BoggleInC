# Boggle

Il Paroliere is a digital implementation of the classic word game, developed as a client-server application in C. This project was created for the Laboratory II course (corso A) by Denis Gabriel Craciun.

## Project Overview

The game challenges participants to find as many words as possible in a 4x4 grid of letters within a time limit. It demonstrates the practical application of advanced C programming concepts, including network programming, multithreading, and the implementation of efficient data structures and algorithms.

### Key Features

- Client-server architecture
- Concurrent handling of multiple players
- Efficient word searching using a Trie data structure
- Real-time game updates and scoring
- Text-based GUI for the client

## Structure

The project is divided into two main components: the client and the server. Both share a similar directory structure:

- `/src`: Contains source files (.c)
- `/headers`: Contains header files (.h)
- `/executables`: Contains the compiled executables
- `/objects`: Contains object files generated during compilation

The server has an additional `/data` directory for game-related data files.

### Main Components

- `main.c`: Entry point for both client and server
- `args_checker.c`: Handles command-line argument parsing
- `matrix_handler.c`: Manages game matrix operations
- `utils.c`: Contains general utility functions
- `client.c`/`server.c`: Main logic for client and server respectively
- `player_handler.c`: (Server only) Manages player-related operations

## Algorithms

The project implements several key algorithms, including:

- Efficient word search in the game matrix
- Trie-based dictionary lookup
- Concurrent client handling
- Dynamic player management

## Building and Running

### Prerequisites

- GCC compiler
- Make

### Compilation

1. Navigate to either the client or server directory.
2. Run `make clean` to ensure a clean build environment.
3. Execute `make all` to compile the project.

### Execution

1. Start the server:
./executables/server <server_name> <port> [options]
Options include paths for matrix and dictionary files.

2. Start the client:
3. ./executables/client <server_name> <port>
**Note**: Ensure the server is running before starting any clients.

## Testing

While no formal test suite is included, the project has been tested using Valgrind for memory leaks, deadlocks, and race conditions.

## Future Improvements

- Implementation of a formal test suite
- Further modularization of the codebase
- Enhanced GUI features

## Author

Denis Gabriel Craciun

## Acknowledgments

This project was developed as part of the Laboratory II course at [Your University Name].
