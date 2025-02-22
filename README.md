# Conway's Game of Life

This project is an implementation of the mathematical game: Conway's Game of Life. In this project we use threads to independently advance the state of the game, display the game state, and handle user input.



## Table of Contents

1. [Introduction](#introduction)
2. [Requirements](#requirements)
3. [Project Structure](#project-structure)
4. [Building the Project](#building-the-project)
5. [Running the Project](#running-the-project)
6. [Input Files](#input-files)
7. [Usage](#usage)
8. [Implementation Details](#implementation-details)
9. [Testing and Memory Management](#testing-and-memory-management)



## Introduction

Conway's Game of Life is a cellular automaton devised by mathematician John Conway. The game consists of a grid of cells that evolve over discrete time steps according to a set of rules based on the states of neighboring cells.



## Requirements

- C++17 or later
- A *NIX operating system (Linux, macOS, etc.)
- Makefile for building the project
- Valgrind for memory leak detection



## Project Structure

.
├── grid.h
├── grid.cpp
├── tui.h
├── tui.cpp
├── p3.cpp
├── Makefile

- `grid.h/grid.cpp`: Defines a Grid class which implements the business logic for evaluating a totalistic cellular automaton on a cartesian grid (e.g. Conway's Game of Life)
- `tui.h/tui.cpp`: Defines the namespace tui, containing the classes:
                        RGB : representing a 24-bit color, consisting of a red/green/blue triplet of 8-bit integer values
                        Tile : representing a (potentially colored) unicode symbols
                        Canvas : representing a grid of tiles that could be drawn to a terminal
                        Input : used to control terminal input modes
- `p3.cpp`: Main implementation file for the project.
- `Makefile`: Builds the project.



## Building The Project

To build the project, run the following command in the project directory:

```sh
make p3
```



## Running The Project

To run the project, use the following command:

```sh
./p3 <input_file>
```

Replace <input_file> with the path to a file that contains the initial grid state.



## Input files

Input files are text files where each line represents a row of the grid.
Non-space characters are 'alive' cells, and space characters are 'dead' cells.

Refer to files "glider.txt" and "acorn.txt" as examples of input files.

## Usage

Once the project is running, you can use the following key commands:
- q: Quit the program.
- f: Change the frame rate. Prompts the user to enter a new frame rate.
- u: Change the simulation update rate. Prompts the user to enter a new simulation rate.
- r: Change the rule. Prompts the user to enter a new rule value.



## Implementation Details

The project uses multithreading to handle different aspects of the simulation:
- Drawing the Grid: One thread is responsible for rendering the grid to the terminal.
- Updating the Grid: Multiple threads update the state of the grid in parallel, each handling a separate row.
- Handling User Input: Another thread listens for user input and pauses/resumes the simulation or changes settings based on user commands.



## Testing and Memory Management

Memory leaks can be tested with the following command:

```sh
valgrind -s --leak-check=full ./p3 <input_file>
```

This command will run the project under Valgrind and provide a summary of memory usage, helping you identify and fix any memory leaks.
