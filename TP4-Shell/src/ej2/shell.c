#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>


#define MAX_COMMANDS 200

int count_args_with_quotes(char* command) {
    /* 
    Cuenta la cantidad de argumentos tomando a los que estan entre comillas como uno solo.
    */
    if (!command) return 0;
    
    int count = 0;
    int i = 0;
    int len = strlen(command);
    
    while (i < len) {                                                                               // avanza por argumentos
        while (i < len && (command[i] == ' ' || command[i] == '\t')) {                              // me salteo los espacios en blanco que puede haber al principio
            i++;
        }
        if (i >= len) break;                                                                        // se terminó el string -> no hay más argumentos
        
        count++;                                                                                    // si llega hasta aca entonces no se terminó el string
        int in_quotes = 0;                                                                          // "flag" para guardar si estoy en un argumento entre comillas o no
        char quote_char = '\0';                                                                     // tipo de comilla
        
        while (i < len) {                                                                           // busco donde termina el argumento actual
            if (!in_quotes && (command[i] == '"' || command[i] == '\'')) {                          // entra a un argumento entre comillas
                in_quotes = 1;
                quote_char = command[i];
                i++;
            } else if (in_quotes && command[i] == quote_char) {                                     // sale del argumento entre comillas (cierra comillas)
                in_quotes = 0;
                quote_char = '\0';
                i++;
            } else if (!in_quotes && (command[i] == ' ' || command[i] == '\t')) {                   // espacio
                break;
            } else {                                                                                // palabra normal
                i++;
            }
        }
    }
    return count;
}


char** parse_command_with_quotes(char* command, int* arg_count) {
    *arg_count = count_args_with_quotes(command);
    if (*arg_count == 0) return NULL;
    char **args = malloc((*arg_count + 1) * sizeof(char*));                                         // creo una lista para arg_count +1 para el NULL al final
    if (!args) {
        perror("malloc");
        return NULL;
    }
    int current_arg = 0;
    int i = 0;
    int len = strlen(command);
    while (i < len && current_arg < *arg_count) {
        while (i < len && (command[i] == ' ' || command[i] == '\t')) i++;                           // avanzo hasta que deje de haber espacios
        if (i >= len) break;
        int start = i;
        int in_quotes = 0;                                                                          // si estoy en un argumento entre comillas
        char quote_char = '\0';                                                                     // guardo si estoy entre comillas dobles o simples
        while (i < len) {                                                                           // avanzo hasta el próximo argumento
            if (!in_quotes && (command[i] == '"' || command[i] == '\'')) {
                in_quotes = 1;
                quote_char = command[i];
                i++;
            } else if (in_quotes && command[i] == quote_char) {
                in_quotes = 0;
                quote_char = '\0';
                i++;
            } else if (!in_quotes && (command[i] == ' ' || command[i] == '\t')) {
                break;
            } else if (in_quotes && i==len-1) {
                exit(EXIT_FAILURE);
            } else {
                i++;
            }
        }
        int arg_len = i - start;
        char *arg = malloc(arg_len + 1);                                                            // el argumento que voy a guardar
        if (!arg) {                                                                                 // si falla tengo que liberar todo lo que ya hice
            for (int j = 0; j < current_arg; j++) {
                free(args[j]);
            }
            free(args);
            return NULL;
        }
        int dest = 0;
        for (int src = start; src < i; src++) {                                                     // relleno el string del argumento
            if (command[src] != '"' && command[src] != '\'') {
                arg[dest++] = command[src];
            }
        }
        arg[dest] = '\0';                                                                           // caracter nulo
        args[current_arg++] = arg;
    }
    args[*arg_count] = NULL;                                                                        // NULL al final para pasarlo a execvp
    return args;
}

void free_args(char** args, int arg_count) {
    for (int i = 0; i < arg_count; i++) {
        free(args[i]);
    }
    free(args);
}



int main() {

    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) {
        if (isatty(STDIN_FILENO)) printf("Shell> ");
        
        /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
        fgets(command, sizeof(command), stdin);

        
        /* Removes the newline character (\n) from the end of the string stored in command, if present. 
           This is done by replacing the newline character with the null character ('\0').
           The strcspn() function returns the length of the initial segment of command that consists of 
           characters not in the string specified in the second argument ("\n" in this case). */
        command[strcspn(command, "\n")] = '\0';
        if (strcmp(command, "exit") == 0) {
           break;
        }
        /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
           Each resulting token is stored in the commands[] array. 
           The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
           In each iteration of the while loop, strtok() returns the next token found in command. 
           The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */
        char *token = strtok(command, "|");
        command_count=0;
        bool has_empty_command = false;
        while (token != NULL) {
            while (*token == ' ' || *token == '\t') token++;

            if (*token == '\0') {
                has_empty_command = true;
                break;
            }

            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        if (has_empty_command) {
            fprintf(stderr, "Error: comando vacío\n");
            continue;
        }

        /* You should start programming from here... */

        if (command_count == 0) { 
            fprintf(stderr, "Error: no se detectaron comandos válidos\n");
            continue;}
        int (*fds)[2] = malloc(sizeof(int[2]) * (command_count-1));
        if (!fds) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < command_count - 1; i++) {                                                               // crear los pipes
            if (pipe(fds[i]) < 0){
                free(fds);
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }
        pid_t child_pids[command_count]; 
        for (int i = 0; i < command_count; i++) {
            pid_t child_pid = fork(); 
            if (child_pid < 0) {
                free(fds);
                perror("fork");
                exit(EXIT_FAILURE);
            }
            child_pids[i] = child_pid;

            if (!child_pid) {                                                                                       // estoy en un proceso hijo                                                                         
                if (i) dup2(fds[i-1][0], STDIN_FILENO);                                                             // stdin -> pipe correspondiente
                if (i!=command_count-1) dup2(fds[i][1], STDOUT_FILENO);                                             // stdout -> pipe correspondiente
                for (int k = 0; k<command_count-1; k++) {                                                           // cierro pipes
                    close(fds[k][0]); close(fds[k][1]);
                }

                int arg_count;
                char **args = parse_command_with_quotes(commands[i], &arg_count);                                   // parseo los argumentos
                if (!args) {
                    exit(EXIT_FAILURE);
                }

                if (execvp(args[0], args) < 0) {                                                                    // ejecuto el comando y verifico que no falle                                                              
                    fprintf(stderr, "Error: no se encontró el comando %s\n", args[0]);
                    free_args(args, arg_count);
                    exit(EXIT_FAILURE);
                }
            }
        }
        for (int k = 0; k < command_count - 1; k++) {                                                               // cierro los pipes del padre
            close(fds[k][0]); close(fds[k][1]);
        }   

        int status;
        for (int p = 0; p < command_count; p++) {                                                                   // espero a los hijos desde el padre para liberar recursos
            if (waitpid(child_pids[p], &status, 0) < 0) {
                perror("waitpid");
            }
        }
        free(fds);
    }
    return 0;
}