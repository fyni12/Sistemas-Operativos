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

void executeOne(char **tokens);
char *trim(char *str);


void genericTokenize(char *str, char **tokens, int *num_tokens, char *separator)
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
    char str[1024];
    char *tokens[MAX_TOKENS];
    int num_tokens;
    int len;

    struct sigaction nombre;

    nombre.sa_handler = handler;
    sigaction(SIGINT, &nombre, NULL);

    while (bucle)
    {
        printf(">>");
        fflush(stdout);
        fgets(str, sizeof(str), stdin);
        len = strlen(str);

        str[strlen(str) - 1] = '\0';

        trim(str);

        fflush(stdout);
        len = strlen(str);
        if (len != 0 && intentoSalida == 0)
        {

            genericTokenize(str, tokens, &num_tokens, " ");

            executeOne(tokens);
        }
        intentoSalida = 0;
    }
    return 0;
}

void handler()
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

void executeOne(char **tokens)
{

    int result;
    int hijo = fork();

    if (hijo == 0)
    {


        if (execvp(tokens[0], tokens) == -1)
            printf(" Error al ejecutar el comando ' %s ': %s \n ", tokens[0], strerror(errno));
        exit(-1);
    }
    else
    {

        wait(&result);
    }
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

