# Version 1

This version implements a basic command-line shell that can execute user commands, tokenize inputs, and display a customizable prompt with the current username and working directory.

## Key Features

### 1. Prompt Display
   - The shell displays a dynamic prompt in the format:  
     ```
     <username>: <current_working_directory>$
     ```
   - This prompt is constructed by retrieving the current username and working directory at each iteration.

### 2. Command Tokenization
   - User input is processed through the `tokenize` function, which splits the command string into individual arguments. These arguments are then passed to the `execvp` function for execution.
   - The tokenization process supports up to `MAXARGS` arguments, with each argument being up to `ARGLEN` characters in length.

### 3. Command Execution
   - The `execute` function creates a new child process to run the user command. 
   - This function uses `fork` to create a new process and `execvp` to execute the command, allowing the shell to manage processes separately for each command.

### 4. Memory Management
   - Memory allocation and deallocation are handled carefully to avoid memory leaks.
   - All dynamically allocated memory, including command arguments and prompt strings, is freed after each command execution.

## File Structure

- `main()` - Initializes and manages the main loop, displays the prompt, and handles user input.
- `execute(char* arglist[])` - Executes a command by forking a child process.
- `tokenize(char* cmdline)` - Splits the user input into tokens for `execvp`.
- `read_cmd(char* prompt)` - Reads user input and returns it as a string.

## Usage

To run this shell, compile the code and execute. The shell will display a prompt based on the username and working directory. Enter commands as you would in any terminal.

# Version 2

This Version is an enhanced version of a command-line shell that supports basic command execution, input/output redirection, and piping between commands. It offers a customizable prompt and efficiently manages processes and memory.

## Key Features

### 1. Prompt Display
   - The shell dynamically displays a prompt with the username and current working directory in the format:
     ```
     <username>: <current_working_directory>$
     ```
   - The prompt is reconstructed each time the shell is run to reflect any changes in the working directory.

### 2. Command Tokenization
   - User input is tokenized into individual arguments through the `tokenize` function, enabling commands to be parsed and passed to `execvp`.
   - The tokenization process supports a maximum of `MAXARGS` arguments, with each argument up to `ARGLEN` characters long.

### 3. Command Execution with Redirection
   - The `execute` function forks a child process to execute the user's command.
   - Supports input (`<`) and output (`>`) redirection:
     - Input redirection opens a file and directs its content as input to the command.
     - Output redirection creates or overwrites a file with the output of the command.
   - Redirection is implemented by using `dup2` to replace the standard file descriptors (`STDIN_FILENO` and `STDOUT_FILENO`).

### 4. Piping Between Commands
   - The `execute_pipe` function supports piping between two commands using the `|` symbol.
   - When a pipe is detected, two child processes are created:
     - The first child redirects its output to the pipe, connecting its `stdout` to the pipe's write end.
     - The second child reads from the pipe, connecting its `stdin` to the pipe's read end.
   - This setup allows the output of the first command to be the input of the second.

### 5. Memory Management
   - The code carefully allocates and frees memory to prevent leaks.
   - Memory for the prompt, command arguments, and user input is freed after each command execution.

## File Structure

- **`main()`** - Manages the main loop, displaying the prompt, reading input, and initiating execution.
- **`execute(char* arglist[], char* infile, char* outfile)`** - Executes a command with optional input/output redirection.
- **`execute_pipe(char* arglist1[], char* arglist2[], char* infile, char* outfile)`** - Executes two commands connected by a pipe.
- **`tokenize(char* cmdline)`** - Splits the user input into arguments for `execvp`.
- **`read_cmd(char* prompt)`** - Reads user input and returns it as a string.

## Usage

To run this shell, compile the code and execute the binary. The shell will display a prompt and accept commands with options for redirection and piping.

# Version 3
### Overview
This C code implements a simple shell-like program that supports:
1. Basic command execution
2. Input and output redirection
3. Piping between two commands
4. Running commands in the background
5. Handling zombie processes using signal handling

### Key Components
- **`execute` function**: Executes a single command, with optional redirection for input and output, and allows background execution.
- **`execute_pipe` function**: Supports piping between two commands by creating a pipe and forking two child processes for the commands.
- **`tokenize` function**: Tokenizes the command line input into an array of arguments.
- **`read_cmd` function**: Reads and returns a command line from the user, including dynamic prompt creation with username and current directory.
- **Signal Handling**: The `sigchld_handler` prevents zombie processes by handling `SIGCHLD`.

### Key Features
- **Background Execution**: Commands followed by `&` run in the background, and the shell continues accepting input without waiting for the command to complete.
- **Input and Output Redirection**: The shell supports `<` for input redirection and `>` for output redirection.
- **Piping**: The shell can handle a single pipe between two commands (e.g., `ls | grep`).
- **Dynamic Prompt**: Displays the username and current working directory in the prompt.

### Code Structure
1. **Main Loop**:
   - Sets up signal handling to avoid zombies.
   - Displays a dynamic prompt.
   - Reads user input, tokenizes it, and handles background execution, redirection, and pipes.

2. **Command Execution**:
   - `execute` function handles command execution and redirection.
   - `execute_pipe` handles the creation and execution of a pipe.

3. **Memory Management**:
   - Proper memory allocation and deallocation to avoid memory leaks.

### Limitations
- **Pipes**: Currently, only single pipes between two commands are supported.
- **Argument Limit**: Limited to `MAXARGS` (10 arguments) due to static allocation.
- **Error Handling**: Basic error handling; limited checks for specific command failures.

### Example Usage
- **Background Execution**: `sleep 10 &`
- **Input Redirection**: `command < input.txt`
- **Output Redirection**: `command > output.txt`
- **Piping**: `ls | grep 'pattern'`

### Possible Improvements
- Adding support for multiple pipes.
- Dynamic argument list resizing to handle more than `MAXARGS` arguments.
- Enhanced error handling and feedback for the user on command failures.

### Dependencies
- `<unistd.h>`, `<sys/types.h>`, `<sys/wait.h>`, `<pwd.h>`, `<fcntl.h>`, `<signal.h>`: Required for process control, I/O, and signal handling.

### Usage
Ensure to compile this code with a C compiler, such as `gcc`, and test in a compatible environment like Linux or macOS for proper functionality.

# Version 4

## Overview

The program is a simple command-line shell implemented in C, which supports executing commands, input/output redirection, piping, and maintaining command history.

## Features Implemented

- **Command Execution**: 
  - Supports execution of external commands using `execvp`.
  
- **Input/Output Redirection**: 
  - Handles redirection of input using `<` and output using `>`.
  
- **Piping**: 
  - Supports piping between two commands using `|`.
  
- **Command History**: 
  - Maintains the last 10 commands entered by the user.
  - Allows repeating commands using `!n` (where `n` is the command number).
  
- **Background Execution**: 
  - Supports running commands in the background with `&`.

## Code Structure

### Global Variables

- `char* command_history[HISTSIZE]`: Array to store command history.
- `int hist_index`: Index to track the current position in the command history.

### Functionality

- **Signal Handling**: 
  - Uses a signal handler (`sigchld_handler`) to handle child process termination.
  
- **Command Loop**: 
  - Continuously prompts for user input, processes commands, and manages command history.

- **Command Execution and Piping**: 
  - The `execute` function handles command execution and redirection.
  - The `execute_pipe` function manages execution of commands connected by a pipe.

- **Tokenization**: 
  - The `tokenize` function splits user input into command arguments.

- **Memory Management**: 
  - Allocates and frees memory for command history and argument lists.

## Potential Improvements

- Implementing built-in commands (e.g., `exit`, `cd`).
- Adding advanced error handling for various edge cases.
- Supporting more complex command structures.
- Improving user experience by enhancing command line editing capabilities.

## Usage

The code is functional and tested for basic features. It can be compiled and run without issues. Additional enhancements and features can be added as needed.
