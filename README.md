# Linux Shell Implementation

## Overview
This document outlines the evolution of a simple command-line shell program implemented in C, detailing its features, code structure, and potential improvements across five versions.

---

## Version 1

### Key Features
1. **Prompt Display**
   - Displays a dynamic prompt in the format:  
     ```
     <username>: <current_working_directory>$
     ```

2. **Command Tokenization**
   - Tokenizes user input through the `tokenize` function, splitting it into individual arguments.

3. **Command Execution**
   - The `execute` function forks a child process to run the user command using `execvp`.

4. **Memory Management**
   - Manages memory allocation and deallocation to prevent leaks.

### File Structure
- **`main()`**: Initializes the main loop and handles user input.
- **`execute(char* arglist[])`**: Executes the command.
- **`tokenize(char* cmdline)`**: Tokenizes input for execution.
- **`read_cmd(char* prompt)`**: Reads user input.

### Usage
Compile and run to display a prompt for user commands.

---

## Version 2

### Key Features Added
1. **Command Execution with Redirection**
   - Supports input (`<`) and output (`>`) redirection using `dup2`.

2. **Piping Between Commands**
   - The `execute_pipe` function manages pipes between commands.

3. **Memory Management**
   - Allocates and frees memory to avoid leaks.

### File Structure
- **`main()`**: Manages the shell loop and input.
- **`execute(char* arglist[], char* infile, char* outfile)`**: Executes commands with redirection.
- **`execute_pipe(char* arglist1[], char* arglist2[])`**: Handles piped commands.
- **`tokenize(char* cmdline)`**: Tokenizes input.
- **`read_cmd(char* prompt)`**: Reads commands from the user.

### Usage
Compile and run for a shell with command execution, redirection, and piping.

---

## Version 3

### Key Features Added
1. **Running Commands in the Background**
2. **Handling Zombie Processes**

### Key Components
- **`execute` function**: Executes commands with redirection.
- **`execute_pipe` function**: Manages piping between commands.
- **`tokenize` function**: Tokenizes command line input.
- **`read_cmd` function**: Reads user commands with a dynamic prompt.
- **Signal Handling**: Manages child process termination.

### Limitations
- Supports only single pipes and has a maximum argument limit.

### Example Usage
- **Background Execution**: `sleep 10 &`
- **Input Redirection**: `command < input.txt`
- **Output Redirection**: `command > output.txt`
- **Piping**: `ls | grep 'pattern'`

### Usage
Compile with a C compiler and test in a Linux or macOS environment.

### Bugs Found
- Commands that require sudo privelages ain't woking well.
- The command terminal asks for password accepts northing.
- I tried to disable sudo privelages for the shell and successfully did so but it didn't execute perfectly. So background commands that don't require admin privelages are working alright.

---

## Version 4

### Overview
Implements a shell supporting command execution, input/output redirection, piping, and command history.

### Key Features Added
4. **Command History**: Maintains the last 10 commands.

### Code Structure
- **Global Variables**: Stores command history.
- **Signal Handling**: Manages child processes.
- **Command Loop**: Processes commands and maintains history.

### Usage
Compile and run the code for basic functionality and command history.

---

## Version 5

### Overview
A simple shell program that executes commands, manages background jobs, handles redirection, and maintains a command history.

### Key Features Added
- `cd <directory>`: Change directory.
- `exit`: Terminate the shell.
- `jobs`: List background jobs.
- `kill <job_num>`: Terminate a background job.
- `help`: Display help information.

### Code Structure
- **Main Loop**: Displays prompt and reads commands.
- **Signal Handling**: Manages child processes.
- **Tokenization**: Splits commands into tokens.

### Memory Management
- Frees allocated memory for commands and history.

### Limitations
- Supports a maximum of 10 background jobs and command history.

### Bugs Found
- When executing pipe in this version, it was causing issue with built-in commands.
- Execute was being called twice and thus it was executing command on either side of pipe.
- This was fixed using a variable 'mera' which doesn't allow both built-in and external commands to cross each other's way.

---

## Version 6

### Overview
A shell program that enhances command execution with variable management, background job handling, and advanced features like command piping and history tracking.

### Key Features Added
- **Variable Management**: Supports setting (`set`), exporting (`export`), unsetting (`unset`), and printing environment variables (`printenv`).
- **Enhanced Command Handling**: Added support for command piping and background job management.
  
### Code Structure
- **Main Loop**: Continuously reads commands and processes them based on user input.
- **Signal Handling**: Uses a signal handler for managing child process termination.
- **Tokenization**: Splits commands into manageable tokens for execution.
- **Job Management**: Maintains a list of background jobs and allows users to kill them.

### Memory Management
- Allocates and frees memory for command history and variable storage dynamically.

### Limitations
- Supports a maximum of 100 background jobs, 10 command history entries, and 100 user-defined variables.

