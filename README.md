# Command-Line Shell Implementation

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

### Key Features
1. **Prompt Display**
   - Shows a dynamic prompt with username and current working directory.

2. **Command Tokenization**
   - Supports tokenization of user input into arguments for `execvp`.

3. **Command Execution with Redirection**
   - Supports input (`<`) and output (`>`) redirection using `dup2`.

4. **Piping Between Commands**
   - The `execute_pipe` function manages pipes between commands.

5. **Memory Management**
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

### Key Features
1. **Basic Command Execution**
2. **Input and Output Redirection**
3. **Piping Between Commands**
4. **Running Commands in the Background**
5. **Handling Zombie Processes**

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

### Dependencies
- Requires headers for process control and signal handling.

### Usage
Compile with a C compiler and test in a Linux or macOS environment.

---

## Version 4

### Overview
Implements a shell supporting command execution, input/output redirection, piping, and command history.

### Features Implemented
1. **Command Execution**
2. **Input/Output Redirection**
3. **Piping**
4. **Command History**: Maintains the last 10 commands.

### Code Structure
- **Global Variables**: Stores command history.
- **Signal Handling**: Manages child processes.
- **Command Loop**: Processes commands and maintains history.

### Potential Improvements
- Implement built-in commands and advanced error handling.

### Usage
Compile and run the code for basic functionality and command history.

---

## Version 5

### Overview
A simple shell program that executes commands, manages background jobs, handles redirection, and maintains a command history.

### Features
1. **Command Execution**: Executes commands with arguments.
2. **Job Management**: Tracks and manages background jobs.
3. **Command History**: Remembers the last 10 commands.
4. **Pipes**: Supports piping between two commands.

### Built-in Commands
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

### Future Improvements
- Increase job limits and enhance built-in commands.

---

