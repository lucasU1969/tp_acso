#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>


int main(int argc, char **argv)
{	
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

	int fds[n][2];  									// Tengo que crear n pipes para comunicar entre todos los procesosos. 
	for (int i = 0; i <= n; i++) {
		pipe(fds[i]);									// creo los pipes para la comunicación entre los procesos.
	}
	
	pid_t child_pid[n]; 								// para guardar los pids de los hijos.
	int child_idx;
	for (int i = 0; i < n; i++) {
		child_pid[(i-start)%n] = fork(); 				// relleno el vector de pids de los hijos.
		int child_idx = (i-start)%n;
		if (!child_pid[i]) break;						// soy un hijo, entonces tengo que dejar de ejecutar el código porque no quiero seguir creando procesos.
	}

	if (child_pid[child_idx]) {							// soy el padre
		
		for (int i = 0; i < n; i++) {					// cerrar todos los pipes que no voy a usar.
			if (i==0) {
				close(fds[i][0]); 
			} else if (i==n-1) {
				close(fds[i][1]);
			} else {
				close(fds[i][0]); 
				close(fds[i][1]);
			}

		}

		write(fds[0][0], buffer, sizeof(buffer));		// esto puede dar error si es negativo -> hacer el chequeo

		// tengo que escribir c en el pipe que conecta con el hijo con child_idx = 0 

		// escuchar en el último pipe que es en el que voy a recibir el dato despues de que el último hijo lo incremente.

	}

	else {
		// calcular el índice del pipe en el que tengo que escuchar
		// cerrar todos los pipes de los que no voy a escuchar.
		// incrementar el dato en 1
		// escribir el dato en el pipe siguiente al usado para escuchar
	}



}