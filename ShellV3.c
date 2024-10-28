#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>

#define MAXARGS 10
#define ARGLEN 30

int execute(char* arglist[], char* infile, char* outfile, int background);
void execute_pipe(char* arglist1[], char* arglist2[], char* infile, char* outfile);
char** tokenize(char* cmdline);
char* read_cmd(char*);

// Signal handler to avoid zombie processes
void sigchld_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    char *cmdline;
    char **arglist;

    // Set up signal handling for SIGCHLD to avoid zombie processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction failed");
        exit(1);
    }

    while (1) {
        // Get current username
        struct passwd *pw = getpwuid(getuid());
        const char *username = pw ? pw->pw_name : "unknown";

        // Get current working directory
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));

        // Calculate needed size for prompt
        int length_needed = snprintf(NULL, 0, "%s@%s$ ", username, cwd) + 1; // +1 for null terminator
        
        // Allocate memory for the prompt
        char *prompt = malloc(length_needed);
        if (prompt == NULL) {
            perror("Unable to allocate memory for prompt");
            exit(1);
        }

        // Construct the prompt
        snprintf(prompt, length_needed, "%s@%s$ ", username, cwd);

        // Read command from the user
        if ((cmdline = read_cmd(prompt)) == NULL) {
            free(prompt); // Free prompt memory before exiting
            break; // Exit loop if EOF or error
        }

        // Tokenize the command line
        if ((arglist = tokenize(cmdline)) != NULL) {
            char *infile = NULL;
            char *outfile = NULL;
            char *arglist2[MAXARGS + 1]; // For second command in case of pipe
            int i = 0, j = 0;
            int background = 0;

            // Check if the command should run in the background
            int last_arg = 0;
            while (arglist[last_arg] != NULL) {
                last_arg++;
            }
            last_arg--;
            if (strcmp(arglist[last_arg], "&") == 0) {
                background = 1;
                arglist[last_arg] = NULL; // Remove '&' from arglist
            }

            // Check for input and output redirection or pipes
            while (arglist[i] != NULL) {
                if (strcmp(arglist[i], "<") == 0) {
                    infile = arglist[i + 1];
                    arglist[i] = NULL; // Terminate arglist here
                }
                else if (strcmp(arglist[i], ">") == 0) {
                    outfile = arglist[i + 1];
                    arglist[i] = NULL; // Terminate arglist here
                }
                else if (strcmp(arglist[i], "|") == 0) {
                    arglist[i] = NULL; // Terminate the first arglist
                    // Fill second arglist with remaining tokens
                    while (arglist[i + 1] != NULL) {
                        arglist2[j++] = arglist[i + 1];
                        i++;
                    }
                    arglist2[j] = NULL;
                    execute_pipe(arglist, arglist2, infile, outfile);
                    break; // Exit loop after processing pipe
                }
                i++;
            }

            if (arglist[i] == NULL) {
                execute(arglist, infile, outfile, background);
            }

            // Free allocated memory for arglist
            for (int j = 0; j < MAXARGS + 1; j++) {
                free(arglist[j]);
            }
            free(arglist);
            free(cmdline);
        }

        free(prompt); // Free the prompt after each iteration
    }

    printf("\n");
    return 0;
}

int execute(char* arglist[], char* infile, char* outfile, int background) {
    int status;
    int cpid = fork();

    switch (cpid) {
        case -1:
            perror("fork failed");
            exit(1);
        case 0:
            // Handle input redirection
            if (infile != NULL) {
                int fd_in = open(infile, O_RDONLY);
                if (fd_in == -1) {
                    perror("Failed to open input file");
                    exit(1);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }

            // Handle output redirection
            if (outfile != NULL) {
                int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out == -1) {
                    perror("Failed to open output file");
                    exit(1);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }

            execvp(arglist[0], arglist);
            perror("Command not found...");
            exit(1);
        default:
            if (background) {
                printf("[%d] %d\n", 1, cpid); // Indicate background job
            } else {
                waitpid(cpid, &status, 0); // Wait for foreground process
                printf("Child exited with status %d\n", status >> 8);
            }
            return 0;
    }
}

void execute_pipe(char* arglist1[], char* arglist2[], char* infile, char* outfile) {
    int pipefd[2];
    pid_t cpid1, cpid2;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    cpid1 = fork();
    if (cpid1 == -1) {
        perror("fork failed");
        exit(1);
    }

    if (cpid1 == 0) { // First child process
        if (infile != NULL) {
            int fd_in = open(infile, O_RDONLY);
            if (fd_in == -1) {
                perror("Failed to open input file");
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        // Redirect stdout to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]); // Close unused read end
        close(pipefd[1]); // Close write end after duplicating

        execvp(arglist1[0], arglist1);
        perror("Command not found in first command...");
        exit(1);
    } else {
        cpid2 = fork();
        if (cpid2 == -1) {
            perror("fork failed");
            exit(1);
        }

        if (cpid2 == 0) { // Second child process
            // Redirect stdin from pipe
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[1]); // Close unused write end

            if (outfile != NULL) {
                int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_out == -1) {
                    perror("Failed to open output file");
                    exit(1);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }

            execvp(arglist2[0], arglist2);
            perror("Command not found in second command...");
            exit(1);
        } else {
            // Parent process
            close(pipefd[0]); // Close read end
            close(pipefd[1]); // Close write end
            waitpid(cpid1, NULL, 0); // Wait for first child
            waitpid(cpid2, NULL, 0); // Wait for second child
        }
    }
}

char** tokenize(char* cmdline) {
    char **arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int j = 0; j < MAXARGS + 1; j++) {
        arglist[j] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[j], ARGLEN);
    }

    if (cmdline[0] == '\0') {
        return NULL; // Return NULL if the user pressed enter without any input
    }

    int argnum = 0;
    char *cp = cmdline;
    char *start;
    int len;

    while (*cp != '\0') {
        while (*cp == ' ' || *cp == '\t') {
            cp++;
        }
        start = cp;
        len = 1;
        while (*++cp != '\0' && (*cp != ' ' && *cp != '\t')) {
            len++;
        }

        // Allocate memory for each argument
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0'; // Null-terminate the argument
        argnum++;

        // If we exceed the maximum number of arguments, return NULL
        if (argnum >= MAXARGS) {
            printf("Too many arguments!\n");
            return NULL;
        }

        // Skip over any trailing whitespace
        while (*cp == ' ' || *cp == '\t') {
            cp++;
        }
    }

    // Null-terminate the list of arguments
    arglist[argnum] = NULL;
    return arglist;
}

char* read_cmd(char* prompt) {
    printf("%s", prompt);
    char* cmdline = (char*)malloc(sizeof(char) * 1024);
    if (fgets(cmdline, 1024, stdin) == NULL) {
        free(cmdline);
        return NULL;
    }

    // Remove the newline character at the end, if present
    int length = strlen(cmdline);
    if (cmdline[length - 1] == '\n') {
        cmdline[length - 1] = '\0';
    }

    return cmdline;
}


