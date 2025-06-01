#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200

int count_words(const char *str);

int main() {

    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) 
    {
        printf("Shell> ");
        
        /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
        fgets(command, sizeof(command), stdin);

        
        /* Removes the newline character (\n) from the end of the string stored in command, if present. 
           This is done by replacing the newline character with the null character ('\0').
           The strcspn() function returns the length of the initial segment of command that consists of 
           characters not in the string specified in the second argument ("\n" in this case). */
        command[strcspn(command, "\n")] = '\0';

        /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
           Each resulting token is stored in the commands[] array. 
           The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
           In each iteration of the while loop, strtok() returns the next token found in command. 
           The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */
        char *token = strtok(command, "|");
        command_count=0;
        while (token != NULL) 
        {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }
        printf("%s\n", commands[0]);

        /* You should start programming from here... */

        // int fds[command_count-1][2];                                                                            // fds para crear los pipes
        int (*fds)[2] = malloc(sizeof(int[2]) * (command_count-1));
        if (!fds) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < command_count - 1; i++) {                                                               // crear los pipes
            pipe(fds[i]);
        }
        pid_t child_pids[command_count]; 
        for (int i = 0; i < command_count; i++) {
            printf("Command %d: %s\n", i, commands[i]);
            
            pid_t child_pid = fork(); 
            child_pids[i] = child_pid;
            
            if (!child_pid) {                                                                                   // estoy en un proceso hijo
                if (i) dup2(fds[i-1][0], STDIN_FILENO);                                                         // modifico stdin para que sea el pipe correspondiente
                if (i!=command_count-1) dup2(fds[i][1], STDOUT_FILENO);                                         // modifico stdout para que sea el pipe correspondiente
                for (int k = 0; k<command_count-1; k++) {                                                                    // cierro pipes
                    close(fds[k][0]); close(fds[k][1]);
                }


                int arg_count = count_words(commands[i]);                                                       // parseo de los argumentos
                char **args = malloc((arg_count + 1) * sizeof(char *));
                if (!args) {
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                char *arg = strtok(commands[i], " "); 
                int j = 0;
                while (arg) {
                    args[j++] = arg;
                    arg = strtok(NULL, " ");
                }
                args[j] = NULL;

                if (execvp(args[0], args) < 0) {                                                                // ejecuto el comando y verifico que no falle
                    perror("execvp");
                    free(args);
                    exit(EXIT_FAILURE);
                }
            }
        }   
        for (int k = 0; k<command_count-1; k++) {                                                               // cierro pipes
            close(fds[k][0]); close(fds[k][1]);
        } 
        int status;
        for (int p = 0; p < command_count; p++) {
            waitpid(child_pids[p], &status, 0);
            // CHEQUEOS
        }
        free(fds);
    }
    return 0;
}

int count_words(const char *str) {
    int count = 0;
    int in_word = 0;

    while (*str) {
        if (*str != ' ' && !in_word) {
            in_word = 1;
            count++;
        } else if (*str == ' ') {
            in_word = 0;
        }
        str++;
    }

    return count;
}