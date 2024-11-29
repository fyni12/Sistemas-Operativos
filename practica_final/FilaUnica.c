#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>

// TODO PREGUNTAR SI ESTO SE PUEDE
int n_clientes = 20;
int n_cajeros = 3;

char *logPath;
FILE *logFile;

pthread_cond_t condicion_reponedor;

pthread_mutex_t mutex_log, mutex_cola, mutex_reponedor;

typedef struct
{
    char id[20];
    int estado;
} cliente;
cliente *cola;

// lista de clientes y cajeros

void cleanLogger()
{
    pthread_mutex_lock(&mutex_log);

    logFile = fopen(logPath, "w");
    fclose(logFile);

    pthread_mutex_unlock(&mutex_log);
}

void writeLog(char *id, char *msg)
{
    time_t now = time(0);
    struct tm *tlocal = localtime(&now);
    char stnow[25];
    strftime(stnow, 25, " %d/ %m/ %y %H: %M: %S ", tlocal);

    pthread_mutex_lock(&mutex_log);

    logFile = fopen(logPath, "a");
    fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);
    fclose(logFile);

    pthread_mutex_unlock(&mutex_log);
}

int findClient(char *id) //! no tiene mutex
{
    int pos = -1;
    for (int i = 0; i < n_clientes; i++)
    {
        if (strcmp(cola[i].id, id) == 0)
        {
            pos = i;
            break;
        }
    }
    return pos;
}

void removeClient(int posicion) //! no tiene mutex
{

    for (int i = posicion; i < n_clientes - 1; i++)
    {

        strcpy(cola[i].id, cola[i + 1].id);

        cola[i].estado = cola[i + 1].estado;
    }

    strcpy(cola[n_clientes - 1].id, "vacio");

    cola[n_clientes - 1].estado = 0;
}

// void mostrarClientes()
// {
//     pthread_mutex_lock(&mutex_cola);

//     for (int i = 0; i < n_clientes; i++)
//     {
//         printf("%d: %s  %d\n", i, cola[i].id, cola[i].estado);
//     }
//     pthread_mutex_unlock(&mutex_cola);
// }

void mostrarClientes() {
    pthread_mutex_lock(&mutex_cola);

    for (int i = 0; i < n_clientes; i++) {
        if (cola[i].id != NULL) { // Verifica que id está inicializado
            printf("%d: %s  %d\n", i, cola[i].id, cola[i].estado);
        } else {
            printf("%d: Cliente no inicializado\n", i);
        }
    }

    pthread_mutex_unlock(&mutex_cola);
}


void *clientAlarm(void *id)
{

    sleep(rand() % 10 + 5); // duerme un tiempo random

    pthread_mutex_lock(&mutex_cola); // TODO  reajustar los mutex

    int pos = findClient(id);

    printf("se acabo la espera del cliente %s\n", (char *)id);
    if (pos != -1)
    {
        if (cola[pos].estado == 0)
        {
            printf("eliminando cliente %s\n", (char *)id);

            removeClient(pos);

            pthread_mutex_unlock(&mutex_cola);
            writeLog(id, "se canso de no ser atendido y se fue");
        }
    }
    else
    {

        pthread_mutex_unlock(&mutex_cola);
        writeLog("ERROR", "no se encontro el cliente que se queria eliminar");
        printf("ERROR AL ELIMINAR UN CLIENTE CON UNA ALARMA\n");
    }
}

int findEmptyPos() //!NO TIENE MUTEX
{
    int pos = -1;
    int i = 0;

    while (pos == -1 && i < n_clientes)
    {

        if (strcmp(cola[i].id, "vacio") == 0)
        {

            pos = i;
        }

        i++;
    }

    return pos;
}

void *createClient()
{
    static int numeroCliente = 0;

    printf("detectado la creacion de cliente\n");

    pthread_mutex_lock(&mutex_cola);
    int pos = findEmptyPos();

    if (strcmp(cola[pos].id, "vacio") == 0)
    {
        // TODO CUANTO TIEMPO ES EL MAX QUE TIENEN QUE ESPERAR?
        // TODO LANZAS UN HILO AL QUE LE PASAS EL NOMBRE DEL HIJO, Y YA ESTÁ, Y BASICAMENTE QUE ESE HILO SIMULE UNA ALARMA

        pthread_t hilo;
        char cli[20] = "Cliente ";
        char numero[20];
        sprintf(numero, "%d", numeroCliente);
        strcat(cli, numero);

        printf("creando cliente %d en la posicion %d\n", numeroCliente, pos);
        strcpy(cola[pos].id, cli);

        cola[pos].estado = 0;
        numeroCliente++;
        writeLog(cli, "ha entrado a la tienda");
        pthread_mutex_unlock(&mutex_cola);

        char *clidup = strdup(cli);

        pthread_create(&hilo, NULL, clientAlarm, (void *)clidup);
        pthread_detach(hilo);
    }
    else
    {

        pthread_mutex_unlock(&mutex_cola);
    }
}

void handlerClient()
{
    pthread_t hilo;

    pthread_create(&hilo, NULL, createClient, NULL);
    pthread_detach(hilo);
}

int clientAvaileable() //! NO TIENE MUTEX
{

    int pos = -1;
    for (int i = 0; i < n_clientes; i++)
    { // TODO CAMBIAR A WHILE

        if (strcmp(cola[i].id, "vacio") != 0 && cola[i].estado == 0)
        {

            pos = i;
            break;
        }
    }
    return pos;
}

void *reponedor()
{

    while (1)
    {

        pthread_mutex_lock(&mutex_reponedor);

        pthread_cond_wait(&condicion_reponedor, &mutex_reponedor);
        int tiempo = rand() % 10 + 5; // tarda entre 5 y 15 segundos
        //sleep(tiempo);

        sleep(1);

        pthread_cond_signal(&condicion_reponedor);

        pthread_mutex_unlock(&mutex_reponedor);
    }
}

void *cajero(void *idCajero)
{

    int clientesAtendidos = 0;
    char *idCliente;
    int tiempoEspera, probcliente;
    pthread_t dependiente;

    printf("Se creo el %s\n", (char *)idCajero);
    while (1)
    {
        pthread_mutex_lock(&mutex_cola);
        int posicionCliente = clientAvaileable();
        if (posicionCliente != -1)
        {

            cola[posicionCliente].estado = 1;
            char idCliente[20];
            strcpy(idCliente, cola[posicionCliente].id);

            pthread_mutex_unlock(&mutex_cola);

            char descripcionCajero[100];

            sprintf(descripcionCajero, "Atendiendo a  %s", idCliente);
            printf("%s\n", idCliente);
            writeLog(idCajero, descripcionCajero);

            // atencion del cliente

            tiempoEspera = rand() % 10 + 2;
            probcliente = rand() % 101;

            // sleep(tiempoEspera);              //!HAY QUE HACER EL TIEMPO DE ESPERA
            sleep(1);

            if (probcliente > 70 && probcliente <= 95)
            {
                static int prioridad = 1;

                pthread_mutex_lock(&mutex_reponedor);
                int var = prioridad;
                prioridad++;

                sprintf(descripcionCajero, "%s quiere consultar un precio", idCliente);
                writeLog("reponedor", descripcionCajero);

                    writeLog("yo", "bucle");
                do
                {
                    pthread_cond_signal(&condicion_reponedor);

                    pthread_cond_wait(&condicion_reponedor, &mutex_reponedor);

                    var--;

                } while (var > 0);

                prioridad--;

                writeLog("reponedor", "se acabo de consultar el precio");

                sprintf(descripcionCajero, "acabo de atender a %s", idCliente);
                writeLog(idCajero, descripcionCajero);

                pthread_mutex_unlock(&mutex_reponedor);
            }
            else if (probcliente > 95)
            {

                // fallo
                sprintf(descripcionCajero, "%s se fue porque hubo un fallo al realizar la compra", idCliente);
                writeLog(idCajero, descripcionCajero);
            }
            else
            {
                // todo bien
                sprintf(descripcionCajero, "acabo de atender a %s", idCliente);
                writeLog(idCajero, descripcionCajero);
            }

            pthread_mutex_lock(&mutex_cola);
            removeClient(findClient(idCliente));
            pthread_mutex_unlock(&mutex_cola);
            printf("!!se atendio a %s ¡¡\n ", idCliente);

            clientesAtendidos++;
        }
        else
        {
            pthread_mutex_unlock(&mutex_cola);
        }

        if (clientesAtendidos >= 10)
        {
            clientesAtendidos = 0;
            sleep(20);
        }
    }
}



int main()
{
    srand(time(NULL));

    // localizacion del archivo
    logPath = "./registroCaja.log";
    cleanLogger();
    // inicializar la fila de clientes
    cola = (cliente *)malloc(sizeof(cliente) * n_clientes);
    for (int i = 0; i < n_clientes; i++)
    {
        strcpy(cola[i].id, "vacio");
        cola[i].estado = 0;
    }

    printf("%d\n", findEmptyPos());

    // se redefine el comportamiento de sigur1
    struct sigaction sa;
    sa.sa_handler = handlerClient;
    sigaction(SIGUSR1, &sa, NULL);



    pthread_mutex_init(&mutex_log, NULL);
    pthread_mutex_init(&mutex_cola, NULL);
    pthread_mutex_init(&mutex_reponedor, NULL);
    pthread_cond_init(&condicion_reponedor, NULL);

    printf("%d\n", getpid());

    for (int i = 0; i < n_cajeros; i++)
    {
        pthread_t cajero_hilo;
        char cajeroid[20];
        sprintf(cajeroid, "cajero %d", i);

        char *cajeroiddup = strdup(cajeroid);

        pthread_create(&cajero_hilo, NULL, cajero, cajeroiddup);
        pthread_detach(cajero_hilo);
    }

    pthread_t reponedor_hilo;
    pthread_create(&reponedor_hilo, NULL, reponedor, NULL);
    pthread_detach(reponedor_hilo);



    while (1)
    {
        mostrarClientes();
        pause();

    }

    pthread_mutex_destroy(&mutex_log);
    pthread_mutex_destroy(&mutex_cola);
    pthread_mutex_destroy(&mutex_reponedor);
    pthread_cond_destroy(&condicion_reponedor);
    free(cola);

    return 0;
}

// TODO HACER QUE LOS CLIENTES CONSUMAN CAJEROS
// TODO hay que eliminar el hilo que crea a los clientes
// TODO HACER LA CAPTURA DEL CTRL C PARA PREGUNTAR SI QUIERES SALIR

//! importante
// TODO VOLVER A PONER LOS SLEEPS
