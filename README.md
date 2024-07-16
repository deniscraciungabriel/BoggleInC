# BoggleInC
The Paroliere project is a digital implementation of the classic word game, developed as a client-server application in C. The game challenges participants to find as many words as possible in a 4x4 grid of letters within a time limit.

The server manages the game logic, including generating the letter matrix, verifying words proposed by players, and maintaining scores. It utilizes efficient data structures such as a Trie for the dictionary and optimized algorithms for word searching in the matrix.

Clients provide a text-based user interface through which players can interact with the game, submitting words and receiving real-time feedback. The system supports multiple simultaneous games, effectively handling concurrency and synchronization between players.

This project demonstrates the practical application of advanced C programming concepts, including network programming, multithreading, and the implementation of efficient data structures and algorithms.
