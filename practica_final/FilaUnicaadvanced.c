#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>

int n_clientes = 20;
int n_cajeros = 3;

pthread_t *cajeros;

int cajeros_solicitar = 0;

int *clientesAtendidosTotal;

int fin = 1;

char *logPath;
FILE *logFile;

pthread_cond_t condicion_reponedor, condicion_cajero;

pthread_mutex_t mutex_log, mutex_cola, mutex_reponedor, mutex_cajero; // el mutex cajero es para gestionar la varible de la creacion de los propios cajeros

typedef struct
{
    char id[20];
    int estado;
} cliente;

cliente *cola;

void cleanLogger()
{
    pthread_mutex_lock(&mutex_log);

    logFile = fopen(logPath, "w");
    fclose(logFile);

    pthread_mutex_unlock(&mutex_log);
}

void writeLog(char *id, char *msg)
{

    // esto es para calcular la hora a la que se escribe el log
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
    int i = 0;

    while (i < n_clientes && pos == -1)
    {
        if (strcmp(cola[i].id, id) == 0)
        {
            pos = i;
        }
        i++;
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

void mostrarClientes()
{
    pthread_mutex_lock(&mutex_cola);

    for (int i = 0; i < n_clientes; i++)
    {
        if (cola[i].id != NULL)
        {
            printf("%d: %s  %d\n", i, cola[i].id, cola[i].estado);
        }
        else
        {
            printf("%d: Cliente no inicializado\n", i);
        }
    }

    pthread_mutex_unlock(&mutex_cola);
}

void *clientAlarm(void *id)
{

    sleep(10); // espera un tiempo random

    pthread_mutex_lock(&mutex_cola);

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
        else
        {
            pthread_mutex_unlock(&mutex_cola);
        }
    }
    else
    {
        pthread_mutex_unlock(&mutex_cola);
    }
}

int findEmptyPos() // encuentra una posicion de la cola vacia //! NO TIENE MUTEX
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

    if (pos != -1)
    {
        pthread_t hilo;

        // esto es para el logger
        char cli[20] = "Cliente ";
        char numero[20];

        sprintf(numero, "%d", numeroCliente);
        strcat(cli, numero);

        strcpy(cola[pos].id, cli);

        cola[pos].estado = 0;
        numeroCliente++;
        pthread_mutex_unlock(&mutex_cola);

        writeLog(cli, "ha entrado a la tienda");

        char *clidup = strdup(cli);
        if (rand() % 10 == 1)
        {
            // solo tiene un 10% de probabilidades de abandonar la fila
            pthread_create(&hilo, NULL, clientAlarm, (void *)clidup);
            pthread_detach(hilo);
        }
    }
    else
    {
        pthread_mutex_unlock(&mutex_cola);
    }
}

int clientAvaileable() // encuentra un cliente no atendido //! NO TIENE MUTEX
{
    int i = 0;
    int pos = -1;

    while (i < n_clientes && pos == -1)
    {
        if (strcmp(cola[i].id, "vacio") != 0 && cola[i].estado == 0)
        {
            pos = i;
        }
        i++;
    }

    return pos;
}

void *reponedor() // ejecuta la funcion del reponedor
{

    while (fin)
    {

        pthread_mutex_lock(&mutex_reponedor); // bloquea el mutex

        if (cajeros_solicitar == 0)
            pthread_cond_wait(&condicion_reponedor, &mutex_reponedor); // espera a que se lo requiera
        pthread_mutex_unlock(&mutex_reponedor);

        int tiempo = rand() % 5 + 1; // tarda entre 5 y 15 segundos
        sleep(tiempo);

        pthread_cond_signal(&condicion_cajero); // avisa de que ya hizo su funcion
    }
}

int ultimosNums(char *entrada)
{

    int n = strlen(entrada);
    int salida = 0;

    int j = 1;

    for (int i = n - 1; i > 0; i--)
    {

        if (entrada[i] == ' ')
        {
            break;
            ;
        }
        int temp = entrada[i] - '0';
        salida += j * temp;
        j *= 10;
    }

    return salida;
}

void *cajero(void *idCajero)
{

    int clientesAtendidos = 0;
    char *idCliente;
    int tiempoEspera, probcliente;

    int n = ultimosNums(idCajero);

    while (fin)
    {
        sleep(.2f);
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

            writeLog(idCajero, descripcionCajero);

            // atencion del cliente

            tiempoEspera = rand() % 5 + 1;
            probcliente = rand() % 101;

            sleep(tiempoEspera); //! HAY QUE HACER EL TIEMPO DE ESPERA

            if (probcliente > 70 && probcliente <= 95)
            {
                static int prioridad = 1;

                sprintf(descripcionCajero, "%s solicita consultar un precio", idCliente);
                writeLog("reponedor", descripcionCajero);
                pthread_mutex_lock(&mutex_reponedor);

                cajeros_solicitar++;

                pthread_mutex_unlock(&mutex_reponedor);

                pthread_cond_signal(&condicion_reponedor);

                pthread_mutex_lock(&mutex_reponedor);
                pthread_cond_wait(&condicion_cajero, &mutex_reponedor);

                cajeros_solicitar--;

                pthread_mutex_unlock(&mutex_reponedor);

                writeLog("reponedor", "acaba de consultar el precio");

                sprintf(descripcionCajero, "acabo de atender a %s,la compra tenia un precio de %d€", idCliente, rand() % 100 + 1);
                writeLog(idCajero, descripcionCajero);
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
                sprintf(descripcionCajero, "acabo de atender a %s,la compra tenia un precio de %d€", idCliente, rand() % 100 + 1);
                writeLog(idCajero, descripcionCajero);
            }

            pthread_mutex_lock(&mutex_cola);
            removeClient(findClient(idCliente));
            pthread_mutex_unlock(&mutex_cola);

            clientesAtendidos++;
            clientesAtendidosTotal[n - 1]++;
        }
        else
        {
            pthread_mutex_unlock(&mutex_cola);
        }

        if (clientesAtendidos >= 10)
        {
            writeLog(idCajero, "se tomará un descanso");
            clientesAtendidos = 0;
            sleep(20);
            writeLog(idCajero, "vuelve del descanso");
        }
    }
}

void *resizeQueue()
{
    pthread_mutex_lock(&mutex_cola);
    n_clientes++;
    cliente *temp = (cliente *)malloc(sizeof(cliente) * n_clientes);

    for (int i = 0; i < n_clientes - 1; i++)
    {
        strcpy(temp[i].id, cola[i].id);
        temp[i].estado = cola[i].estado;
    }

    strcpy(temp[n_clientes - 1].id, "vacio");
    temp[n_clientes - 1].estado = 0;
    free(cola);
    cola = temp;
    pthread_mutex_unlock(&mutex_cola);
    writeLog("SYSTEM", "Se ha aumentado la cola de clientes");
}

void *addCajero()
{
    static int nCaj = 0;

    pthread_mutex_lock(&mutex_cajero);
    nCaj++;

    pthread_t cajero_hilo;

    char cajeroid[20];
    sprintf(cajeroid, "cajero %d", nCaj);

    char *cajeroiddup = strdup(cajeroid);

    pthread_create(&cajero_hilo, NULL, cajero, cajeroiddup);

    cajeros[nCaj - 1] = cajero_hilo;

    pthread_mutex_unlock(&mutex_cajero);

    writeLog("SYSTEM", "Se ha añadido un cajero");
}

void *expandCajeros()
{

    pthread_mutex_lock(&mutex_cajero);
    n_cajeros++;
    pthread_t *temp = (pthread_t *)malloc(sizeof(pthread_t) * n_cajeros);
    int *temp2 = (int *)malloc(sizeof(int) * n_cajeros);

    for (int i = 0; i < n_cajeros - 1; i++)
    {
        temp[i] = cajeros[i];
        temp2[i] = clientesAtendidosTotal[i];
    }
    temp2[n_cajeros - 1] = 0;

    free(cajeros);
    free(clientesAtendidosTotal);

    cajeros = temp;
    clientesAtendidosTotal = temp2;

    pthread_mutex_unlock(&mutex_cajero);

    addCajero();
}

void handlerClient()
{

    // lanza un hilo que crea clientes y se desentiende de el
    pthread_t hilo;
    pthread_create(&hilo, NULL, createClient, NULL);
    pthread_detach(hilo);
}

void handlerCajero()
{

    pthread_t hilo;
    pthread_create(&hilo, NULL, expandCajeros, NULL);
    pthread_detach(hilo);
}

void handlerCola()
{

    pthread_t hilo;
    pthread_create(&hilo, NULL, resizeQueue, NULL);
    pthread_detach(hilo);
}

void handlerfin()
{
    writeLog("Supermercado", "se ha cerrado el supermercado");

    fin = 0;
}

void logCajerosResult()
{
    for (int i = 0; i < n_cajeros; i++)
    {
        char cajero[20];
        char msg[100];

        sprintf(cajero, "cajero %d", i + 1);
        sprintf(msg, "Atendio a %d clientes y se va", clientesAtendidosTotal[i]);
        writeLog(cajero, msg);
    }
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    // localizacion del archivo
    logPath = "./registroCaja.log";
    cleanLogger();

    if (argc == 3)
    {

        n_clientes = atoi(argv[1]);
        n_cajeros = atoi(argv[2]);
    }
    else
    {
        printf("usando valores por defecto\n");
    }

    // inicializar la fila de clientes
    cola = (cliente *)malloc(sizeof(cliente) * n_clientes);
    for (int i = 0; i < n_clientes; i++)
    {
        strcpy(cola[i].id, "vacio");
        cola[i].estado = 0;
    }

    clientesAtendidosTotal = (int *)malloc(sizeof(int) * n_cajeros);

    for (int i = 0; i < n_cajeros; i++)
    {
        clientesAtendidosTotal[i] = 0;
    }

    // se redefine el comportamiento de sigur1
    struct sigaction clientSignal;
    clientSignal.sa_handler = handlerClient;
    sigaction(SIGUSR1, &clientSignal, NULL);

    struct sigaction queuetSignal;
    queuetSignal.sa_handler = handlerCola;
    sigaction(SIGUSR2, &queuetSignal, NULL);

    struct sigaction cashierSignal;
    cashierSignal.sa_handler = handlerCajero;
    sigaction(SIGPIPE, &cashierSignal, NULL);

    struct sigaction finalsignal;
    finalsignal.sa_handler = handlerfin;
    sigaction(SIGINT, &finalsignal, NULL);

    pthread_mutex_init(&mutex_log, NULL);
    pthread_mutex_init(&mutex_cola, NULL);
    pthread_mutex_init(&mutex_reponedor, NULL);
    pthread_mutex_init(&mutex_cajero, NULL);
    pthread_cond_init(&condicion_reponedor, NULL);
    pthread_cond_init(&condicion_cajero, NULL);

    printf("pid: %d\n", getpid());

    cajeros = (pthread_t *)malloc(sizeof(pthread_t) * n_cajeros);

    for (int i = 0; i < n_cajeros; i++)
    {
        addCajero();
    }

    pthread_t reponedor_hilo;
    pthread_create(&reponedor_hilo, NULL, reponedor, NULL);
    pthread_detach(reponedor_hilo);

    while (fin)
    {
        pause();
    }

    for (int i = 0; i < n_cajeros; i++)
    {
        pthread_cancel(cajeros[i]);
    }
    pthread_cancel(reponedor_hilo);

    logCajerosResult();

    pthread_mutex_destroy(&mutex_log);
    pthread_mutex_destroy(&mutex_cola);
    pthread_mutex_destroy(&mutex_cajero);
    pthread_mutex_destroy(&mutex_reponedor);
    pthread_cond_destroy(&condicion_reponedor);
    free(cola);
    free(clientesAtendidosTotal);
    free(cajeros);
    return 0;
}