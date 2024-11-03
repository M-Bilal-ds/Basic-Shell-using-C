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
#define HISTSIZE 10
#define MAXJOBS 100
#define MAXVARS 100  // Max number of variables

// Variable structure
struct var {
    char *str;   // name=value string
    int global;  // Boolean to indicate if it's global
};

struct var var_table[MAXVARS];  // Variable table
int var_count = 0;              // Current number of variables

char* command_history[HISTSIZE];
int hist_index = 0;
pid_t jobs[MAXJOBS];  // Array to store background job PIDs
int job_count = 0;     // Current number of jobs

// Function declarations
int execute(char* arglist[], char* infile, char* outfile, int background);
void execute_pipe(char* arglist1[], char* arglist2[], char* infile, char* outfile);
char** tokenize(char* cmdline);
char* read_cmd(char* prompt);
void add_to_history(const char* cmdline);
void repeat_command(int command_number);
void free_history();
void list_jobs();
void kill_job(int job_number);
void help();
// Function declarations for variables
void set_var(char *name, char *value, int global);
char* get_var(char *name);
void unset_var(char *name);
void print_vars();


void sigchld_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    char *cmdline;
    char **arglist;
    int mera = 0;

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction failed");
        exit(1);
    }

    while (1) {
        struct passwd *pw = getpwuid(getuid());
        const char *username = pw ? pw->pw_name : "unknown";
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        int length_needed = snprintf(NULL, 0, "%s@%s$ ", username, cwd) + 1;
        char *prompt = malloc(length_needed);
        if (prompt == NULL) {
            perror("Unable to allocate memory for prompt");
            exit(1);
        }
        snprintf(prompt, length_needed, "%s@%s$ ", username, cwd);


        if ((cmdline = read_cmd(prompt)) == NULL) {
            free(prompt);
            break;
        }

        // Check for command repetition
        if (cmdline[0] == '!') {
            int command_number = atoi(&cmdline[1]);
            if (command_number == -1) {
                command_number = hist_index - 1; // last command
            } else {
                command_number -= 1; // Adjust for zero-based index
            }
            repeat_command(command_number);
            free(cmdline);
            free(prompt);
            continue; // Skip the rest of the loop
        }

        // Add command to history
        add_to_history(cmdline);

        // Tokenize the command line
        if ((arglist = tokenize(cmdline)) != NULL) {
            char *infile = NULL;
            char *outfile = NULL;
            char *arglist2[MAXARGS + 1];
            int i = 0, j = 0;
            int background = 0;

            int last_arg = 0;
            while (arglist[last_arg] != NULL) {
                last_arg++;
            }
            last_arg--;
            if (strcmp(arglist[last_arg], "&") == 0) {
                background = 1;
                arglist[last_arg] = NULL;
            }

            if (strcmp(arglist[0], "set") == 0 && arglist[1] != NULL && arglist[2] != NULL) {
                // "set name value" command
                set_var(arglist[1], arglist[2], 0);  // 0 indicates local variable
            } else if (strcmp(arglist[0], "export") == 0 && arglist[1] != NULL) {
                // "export name" command
                char *value = get_var(arglist[1]);
                if (value != NULL) {
                    set_var(arglist[1], value, 1);  // Set as global
                    setenv(arglist[1], value, 1);  // Update the environment variable
                }
            } else if (strcmp(arglist[0], "unset") == 0 && arglist[1] != NULL) {
                // "unset name" command
                unset_var(arglist[1]);
            } else if (strcmp(arglist[0], "printenv") == 0) {
                // "printenv" command to list all variables
                print_vars();
            }

            while (arglist[i] != NULL) {
                if (strcmp(arglist[i], "<") == 0) {
                    infile = arglist[i + 1];
                    arglist[i] = NULL;
                } else if (strcmp(arglist[i], ">") == 0) {
                    outfile = arglist[i + 1];
                    arglist[i] = NULL;
                } else if (strcmp(arglist[i], "|") == 0) {
                    arglist[i] = NULL;
                    while (arglist[i + 1] != NULL) {
                        arglist2[j++] = arglist[i + 1];
                        i++;
                    }
                    arglist2[j] = NULL;
                    execute_pipe(arglist, arglist2, infile, outfile);
                    mera += 1;
                    break;
                }
                i++;
            }

            // Check for built-in commands
            if (arglist[0] != NULL) {
                if (strcmp(arglist[0], "cd") == 0) {
                    if (arglist[1] != NULL) {
                        if (chdir(arglist[1]) != 0) {
                            perror("cd failed");
                        }
                    } else {
                        fprintf(stderr, "cd: missing argument\n");
                    }
                } else if (strcmp(arglist[0], "exit") == 0) {
                    free(cmdline);
                    free(prompt);
                    free_history();
                    exit(0);
                } else if (strcmp(arglist[0], "jobs") == 0) {
                    list_jobs();
                } else if (strcmp(arglist[0], "kill") == 0) {
                    if (arglist[1] != NULL) {
                        kill_job(atoi(arglist[1]));
                    } else {
                        fprintf(stderr, "kill: missing job number\n");
                    }
                } else if (strcmp(arglist[0], "help") == 0) {
                    help();
                } else { if (mera == 0){
                    // Execute the command
                    execute(arglist, infile, outfile, background);}
                }
            }
            mera -= 1;

            for (int j = 0; j < MAXARGS + 1; j++) {
                free(arglist[j]);
            }
            free(arglist);
            free(cmdline);
        }

        free(prompt);
    }

    printf("\n");
    free_history();
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
            if (infile != NULL) {
                int fd_in = open(infile, O_RDONLY);
                if (fd_in == -1) {
                    perror("Failed to open input file");
                    exit(1);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }

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
                // Add the PID to the jobs list
                if (job_count < MAXJOBS) {
                    jobs[job_count++] = cpid;
                } else {
                    fprintf(stderr, "Too many background jobs.\n");
                }
                printf("[%d] %d\n", job_count, cpid);
            } else {
                waitpid(cpid, &status, 0);
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

    if (cpid1 == 0) {
        if (infile != NULL) {
            int fd_in = open(infile, O_RDONLY);
            if (fd_in == -1) {
                perror("Failed to open input file");
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(arglist1[0], arglist1);
        perror("Command not found in first command...");
        exit(1);
    } else {
        cpid2 = fork();
        if (cpid2 == -1) {
            perror("fork failed");
            exit(1);
        }

        if (cpid2 == 0) {
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[1]);
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
            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(cpid1, NULL, 0);
            waitpid(cpid2, NULL, 0);
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
        return NULL;
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

        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;

        if (argnum >= MAXARGS) {
            printf("Too many arguments!\n");
            return NULL;
        }

        while (*cp == ' ' || *cp == '\t') {
            cp++;
        }
    }

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

void add_to_history(const char* cmdline) {
    // Free the previous command if it exists
    if (command_history[hist_index] != NULL) {
        free(command_history[hist_index]);
    }
    // Store the new command in history
    command_history[hist_index] = strdup(cmdline);
    hist_index = (hist_index + 1) % HISTSIZE; // Wrap around
}

void repeat_command(int command_number) {
    if (command_number < 0 || command_number >= HISTSIZE || command_history[command_number] == NULL) {
        printf("No such command in history.\n");
        return;
    }
    printf("Repeating command: %s\n", command_history[command_number]);
    // Execute the repeated command
    char **arglist = tokenize(command_history[command_number]);
    if (arglist != NULL) {
        char *infile = NULL;
        char *outfile = NULL;
        int background = 0;

        // Execute the command as usual
        execute(arglist, infile, outfile, background);

        // Free the allocated memory for arglist
        for (int j = 0; j < MAXARGS + 1; j++) {
            free(arglist[j]);
        }
        free(arglist);
    }
}

void free_history() {
    for (int i = 0; i < HISTSIZE; i++) {
        if (command_history[i] != NULL) {
            free(command_history[i]);
        }
    }
}

void list_jobs() {
    printf("Background jobs:\n");
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %d\n", i + 1, jobs[i]);
    }
}

void kill_job(int job_number) {
    if (job_number < 1 || job_number > job_count) {
        fprintf(stderr, "kill: no such job\n");
        return;
    }
    pid_t pid = jobs[job_number - 1];
    if (kill(pid, SIGKILL) == 0) {
        printf("Killed job [%d] %d\n", job_number, pid);
        // Remove from jobs list
        for (int i = job_number - 1; i < job_count - 1; i++) {
            jobs[i] = jobs[i + 1];
        }
        job_count--;
    } else {
        perror("kill failed");
    }
}

void set_var(char *name, char *value, int global) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s=%s", name, value);

    // Check if variable already exists
    for (int i = 0; i < var_count; i++) {
        if (strncmp(var_table[i].str, name, strlen(name)) == 0 && var_table[i].str[strlen(name)] == '=') {
            free(var_table[i].str);
            var_table[i].str = strdup(buffer);
            var_table[i].global = global;
            return;
        }
    }

    // Add a new variable if it doesn't exist
    if (var_count < MAXVARS) {
        var_table[var_count].str = strdup(buffer);
        var_table[var_count].global = global;
        var_count++;
    } else {
        fprintf(stderr, "Error: Maximum variable limit reached.\n");
    }
}

char* get_var(char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strncmp(var_table[i].str, name, strlen(name)) == 0 && var_table[i].str[strlen(name)] == '=') {
            return strchr(var_table[i].str, '=') + 1;
        }
    }
    return NULL;
}

void unset_var(char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strncmp(var_table[i].str, name, strlen(name)) == 0 && var_table[i].str[strlen(name)] == '=') {
            free(var_table[i].str);
            var_table[i] = var_table[--var_count];  // Remove and shift last variable to current spot
            return;
        }
    }
}

void print_vars() {
    printf("User-defined variables:\n");
    for (int i = 0; i < var_count; i++) {
        if (!var_table[i].global) {
            printf("  %s\n", var_table[i].str);
        }
    }
    printf("Environment variables:\n");
    for (int i = 0; i < var_count; i++) {
        if (var_table[i].global) {
            printf("  %s\n", var_table[i].str);
        }
    }
}

void help() {
    printf("Built-in commands:\n");
    printf("  cd <directory>  Change the current working directory.\n");
    printf("  exit            Terminate the shell.\n");
    printf("  jobs            List background jobs.\n");
    printf("  kill <job_num>  Terminate a background job.\n");
    printf("  help            Display this help message.\n");
}