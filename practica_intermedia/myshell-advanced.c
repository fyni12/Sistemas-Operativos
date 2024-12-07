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

#define MAX_TOKENS 100       // Maximum number of tokens to parse
#define MAX_TOKEN_LENGTH 100 // Maximum length of each token

int bucle = 1;
int intentoSalida = 0;

void handler();

void executeOne(char **tokens, int entrada, int salida);
int executeSecondPlane(char **tokens, int entrada, int salida);
char *trim(char *str);
void executePipes(char **tokens, int tokensLenght);

void genericTokenize(char *str, char **tokens, int *num_tokens, char *separator) // DEVUELVE UN ARRAY DE STRINGS CON LAS DISTINTAS PALABRAS TENIENDO COMO CRTITERIO DE SEPARACION EL SEPARADOR
{
    *num_tokens = 0;
    char *tok = strtok(str, separator);
    while (tok != NULL && *num_tokens < MAX_TOKENS)
    {
        int len = strlen(tok);
        if (len > MAX_TOKEN_LENGTH - 1)
        {
            printf("Error : token %s is too long .\n", tok);
            return;
        }
        for (int i = 0; i < len; i++)
        {
            tok[i] = tolower(tok[i]); // convert each token to lowercase
        }
        tokens[*num_tokens] = tok;
        (*num_tokens)++;
        tok = strtok(NULL, separator);
    }
    tokens[*num_tokens] = NULL;
}

int main()
{
    char str[1024]; // SE DEFINENE LOS ELEMENTOS DEL GENERIC TOKENIZE
    char *tokens[MAX_TOKENS];
    int num_tokens;
    int len;

    struct sigaction nombre; // SE DEFINE EL MANEJADOR DE LA SEÑAL

    nombre.sa_handler = handler;
    sigaction(SIGINT, &nombre, NULL);

    while (bucle)
    {
        fflush(stdout);
        7 printf(">>"); // SE LEE LA ENTRADA
        fflush(stdout);
        fgets(str, sizeof(str), stdin); // SE ALMACENA EN STR
        len = strlen(str);

        str[len - 1] = '\0'; // SE CAMBIA EL ULTIMO CARACTER

        trim(str); // SE ELIMINAN LOS ESPACION DEL PRINCIPIO Y DEL FINAL

        fflush(stdout);
        len = strlen(str);
        if (len != 0 && intentoSalida == 0) // SE COMPRUEBA QUE EL BUFFER NO ESTE VACIO Y QUE NO SE HAYA INTENTADO SALIR
        {

            genericTokenize(str, tokens, &num_tokens, "|"); // SE SEPARA EL STRING POR '|'

            executePipes(tokens, num_tokens); // SE MANDA LA EJECUCION DE LOS COMANDOS
        }
        intentoSalida = 0;
    }
    return 0;
}

void handler() // SE PREGUNTA SI SE QUIERE SALIR
{
    char eleccion;

    printf("\nsalir s/n:");
    fflush(stdin);
    scanf("%c", &eleccion);
    intentoSalida = 1;
    if (eleccion == 's')
    {
        printf("saliendo\n");
        bucle = 0;
    }
}

void executeOne(char **tokens, int entrada, int salida)
{
    // ESTE HILO LO QUE HACE ES LANZAR UN PROCESO HUJO AL QUE SE LE REDEFINE LA ENTRADA Y LA SALIDA A LOS DESCRIPTORES DE ARCHIVO, QUE DE NORMAL SON STDIN Y STDOUT Y ESPERA A QUE ACABE LA EJECUCION DE ESE HILO
    int result;
    int hijo = fork();

    if (hijo == 0)
    {

        dup2(entrada, STDIN_FILENO);

        dup2(salida, STDOUT_FILENO);

        if (execvp(tokens[0], tokens) == -1)
            printf(" Error al ejecutar el comando ' %s ': %s \n ", tokens[0], strerror(errno));
        exit(-1);
    }
    else
    {

        wait(&result);
    }
}

int executeSecondPlane(char **tokens, int entrada, int salida)
{
    // ESTE HILO LO QUE HACE ES LANZAR UN PROCESO HUJO AL QUE SE LE REDEFINE LA ENTRADA Y LA SALIDA A LOS DESCRIPTORES DE ARCHIVO, QUE DE NORMAL SON STDIN Y STDOUT Y NO ESPERA A QUE ACABE LA EJECUCION DE ESE HILO

    int hijo = fork();
    if (hijo == 0)
    {
        dup2(entrada, STDIN_FILENO);
        dup2(salida, STDOUT_FILENO);

        if (execvp(tokens[0], tokens) == -1)
            printf(" Error al ejecutar el comando ' %s ': %s \n ", tokens[0], strerror(errno));
        exit(-1);
    }
    return hijo;
}

char *trim(char *str)
{
    int len = strlen(str);

    if (len != 0)
    {
        int espacios_incio = 0, espacios_final = 0;
        int i = 0;

        while (str[i] == ' ')
        {
            espacios_incio++;
            i++;
        }
        i = 1;

        while (str[len - i] == ' ')
        {
            espacios_final++;
            i++;
        }

        str[len - espacios_final] = '\0';

        for (i = espacios_incio; i <= len - espacios_final; i++)
        {
            str[i - espacios_incio] = str[i];
        }
    }
    return str;
}

void executePipes(char **tokens, int tokensLenght)
{
    char *subTokens[MAX_TOKENS];
    int subNum_tokens;

    if (tokensLenght == 1) //EN EL CASO DE QUE SOLO HAYA UN COMANDO 
    {
        genericTokenize(tokens[tokensLenght - 1], subTokens, &subNum_tokens, " ");

        if (subTokens[subNum_tokens - 1][strlen(subTokens[subNum_tokens - 1]) - 1] == '&') //SE COMPRUEBA SI EL ULTIMO CARACTER ES UN AMPERSON Y SE ELIMINA PARA EJECUTAR EL COMANDO EN SEGUNDO PLANO
        {
            subTokens[subNum_tokens - 1][strlen(subTokens[subNum_tokens - 1]) - 1] = '\0';
            executeSecondPlane(subTokens, STDIN_FILENO, STDOUT_FILENO);
        }
        else
        {
            executeOne(subTokens, STDIN_FILENO, STDOUT_FILENO);
        }
    }
    else
    {
        // reserva memoria para todas las tuberias
        int **totalPipes;
        totalPipes = (int **)malloc((tokensLenght - 1) * sizeof(int *));
        for (int i = 0; i < tokensLenght - 1; i++)
        {
            totalPipes[i] = (int *)malloc(2 * sizeof(int));
        } //SE CREANT TODAS LAS TUBERIAS NECESARIAS

        for (int i = 0; i < tokensLenght - 1; i++)
        {
            pipe(totalPipes[i]);
        }

        // primer ejecutar y ultimo tienen que estar definidos a mano

        for (int i = 0; i < tokensLenght; i++) //SE ITERA SOBRE TODOS LOS COMANDOS ASIGNANDOLES LAS TUBERIAS CORRESPONDIENTES
        {
            genericTokenize(tokens[i], subTokens, &subNum_tokens, " ");

            if (i == 0)
            {
                executeSecondPlane(subTokens, STDIN_FILENO, totalPipes[i][1]); // TODO PREGUNTAR SI ESTAS TIENEN QUE SER EN SEGUNDO PLANO
                close(totalPipes[i][1]);                                       // Cerramos el extremo de escritura que ya no se usa
            }
            else if (i == tokensLenght - 1)
            {
                if (subTokens[subNum_tokens - 1][strlen(subTokens[subNum_tokens - 1]) - 1] == '&')
                {
                    subTokens[subNum_tokens - 1][strlen(subTokens[subNum_tokens - 1]) - 1] = '\0';
                    executeSecondPlane(subTokens, totalPipes[i - 1][0], STDOUT_FILENO);
                }
                else
                {
                    executeOne(subTokens, totalPipes[i - 1][0], STDOUT_FILENO);
                }

                close(totalPipes[i - 1][0]); // Cerramos el extremo de lectura que ya no se usa
            }
            else
            {
                executeSecondPlane(subTokens, totalPipes[i - 1][0], totalPipes[i][1]);
                close(totalPipes[i - 1][0]); // Cerramos el extremo de lectura
                close(totalPipes[i][1]);     // Cerramos el extremo de escritura
            }
        }
    }
}
