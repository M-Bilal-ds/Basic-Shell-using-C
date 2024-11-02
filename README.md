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

```sh
$ gcc -o shell shell.c
$ ./shell
username: /current_directory$


