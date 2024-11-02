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

