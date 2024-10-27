#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <limits.h>

#define MAXARGS 10
#define ARGLEN 30

int execute(char* arglist[]);
char** tokenize(char* cmdline);
char* read_cmd(char*);

int main() {
    char *cmdline;
    char **arglist;

    while (1) {
        // Get current username
        struct passwd *pw = getpwuid(getuid());
        const char *username = pw ? pw->pw_name : "unknown";

        // Get current working directory
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));

        // Calculate needed size for prompt
        int length_needed = snprintf(NULL, 0, "%s: %s$ ", username, cwd) + 1; // +1 for null terminator
        
        // Allocate memory for the prompt
        char *prompt = malloc(length_needed);
        if (prompt == NULL) {
            perror("Unable to allocate memory for prompt");
            exit(1);
        }

        // Construct the prompt
        snprintf(prompt, length_needed, "%s: %s$ ", username, cwd);

        // Read command from the user
        if ((cmdline = read_cmd(prompt)) == NULL) {
            free(prompt); // Free prompt memory before exiting
            break; // Exit loop if EOF or error
        }

        // Tokenize the command line
        if ((arglist = tokenize(cmdline)) != NULL) {
            execute(arglist);

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


int execute(char* arglist[]) {
    int status;
    int cpid = fork();
    switch (cpid) {
        case -1:
            perror("fork failed");
            exit(1);
        case 0:
            execvp(arglist[0], arglist);
            perror("Command not found...");
            exit(1);
        default:
            waitpid(cpid, &status, 0);
            printf("Child exited with status %d\n", status >> 8);
            return 0;
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
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }
    arglist[argnum] = NULL;
    return arglist;
}

char* read_cmd(char* prompt) {
    printf("%s", prompt);
    int c;
    int pos = 0;
    char *cmdline = (char*)malloc(sizeof(char) * 1024); // Use a fixed size for cmdline

    while ((c = getc(stdin)) != EOF) {
        if (c == '\n') {
            break;
        }
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline); // Free memory before returning NULL
        return NULL;
    }

    cmdline[pos] = '\0';
    return cmdline;
}
