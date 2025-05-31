#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>


// FALTAN CHEQUEOS

void write_int_pipe(int fd, const int *buff);
void read_int_pipe(int fd, int *buff);
void close_pipes(int fds[][2], int read, int write, int n);

int main(int argc, char **argv) {	
	int start, status, pid, n;
	int buffer[1];

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
    /* Parsing of arguments */
  	/* TO COMPLETE */
	n = atoi(argv[1]); 
	buffer[0] = atoi(argv[2]); 
	start = atoi(argv[3]);

    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
   	/* You should start programming from here... */				

	int fds[n+1][2];  														// Tengo que crear n+1 pipes para comunicar entre todos los procesos. 
	for (int i = 0; i <= n; i++) {
		pipe(fds[i]);														// creo los pipes para la comunicación entre los procesos.
	}
	
	pid_t child_pid[n]; 													// para guardar los pids de los hijos.
	int child_idx;
	for (int i = 0; i < n; i++) {
		pid = fork();
		child_idx = (i - start + n) % n;									// se suma n porque en C la operación % puede devolver negativos (?)
		child_pid[child_idx] = pid;
		if (!pid) break;													// soy un hijo, entonces tengo que salir del while para no crear más procesos.
	}

	if (pid) {																// soy el padre
		close_pipes(fds, n, 0, n+1);										// cierro los pipes que no voy a usar
		write_int_pipe(fds[0][1], buffer);									// paso el dato al hijo que corresponde

		int ret[1]; 
		read_int_pipe(fds[n][0], ret); 										// leo el dato que me devuelve el último hijo

		printf("%d\n", ret[0]);	

		for (int i = 0; i < n; i++) waitpid(child_pid[i], &status, 0);		// espero a los hijos para liberar recursos

	} else {																// soy hijo
		close_pipes(fds, child_idx, child_idx+1, n+1);						// cierro los pipes que no voy a usar

		int c[1];
		read_int_pipe(fds[child_idx][0], c);								// leo el dato que me pasa el hijo anterior

		c[0] = c[0] + 1;
		write_int_pipe(fds[child_idx+1][1], c);								// paso el dato incrementado al próximo hijo
	}
	return 0;
}


void close_pipes(int fds[][2], int read, int write, int n) {
	for (int i = 0; i<n; i++) {
		if (i==read) {
			close(fds[i][1]);
		} else if (i==write) {
			close(fds[i][0]);
		} else {
			close(fds[i][0]);
			close(fds[i][1]); 
		}
	}
}

void read_int_pipe(int fd, int *buff) {
	if (read(fd, buff, sizeof(int)) < 0) {
		perror("read"); 
		exit(EXIT_FAILURE); 
	}
	close(fd);
}

void write_int_pipe(int fd, const int *buff) {
	if (write(fd, buff, sizeof(int)) < 0) {
		perror("write"); 
		exit(EXIT_FAILURE); 
	}
	close(fd);
}