#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define M 5 // Duración máxima de la tarea del proceso padre
#define N 7 // Duración máxima de la tarea del proceso hijo

// Manejador para las señales SIGUSR1 y SIGUSR2
void manejador(int sig) {
    //printf("señal envidad %d\n", sig);
}

int main(void) {
    int x, i = 0;
    pid_t pid;

    pid = fork();  // Creación del proceso hijo

    if (pid == -1) {
        perror("Error en la llamada a fork()");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {  // Proceso hijo
        signal(SIGUSR1, manejador);  // Asignar el manejador para SIGUSR1
        
        while (1)
        {
            pause();  // Espera a que el padre envíe una señal

            srand(time(NULL) ^ (getpid()<<16));  // Generar número aleatorio con una semilla

            x = 1 + rand() % N;  // Número aleatorio entre 1 y N
            printf("COMIENZO TAREA HIJO %d\n", i);
            sleep(x);  // Simula la duración de la tarea
            printf("FIN TAREA HIJO %d\n", i);

            i++;
            kill(getppid(), SIGUSR2);  // Enviar señal al padre para que continúe
            
        }
        
        
    } else {  // Proceso padre
        signal(SIGUSR2, manejador);  // Asignar el manejador para SIGUSR2

        while (1) {
            
            srand(time(NULL) ^ (getpid()<<16));  // Generar número aleatorio

            x = 1 + rand() % M;  // Número aleatorio entre 1 y M
            printf("COMIENZO TAREA PADRE %d\n", i);
            sleep(x);  // Simula la duración de la tarea
            printf("FIN TAREA PADRE %d\n", i);

            i++;
            kill(pid, SIGUSR1);  // Enviar señal al hijo para que continúe
            pause();  // Espera a que el hijo termine y envíe una señal
        }
    }

    return 0;
}
