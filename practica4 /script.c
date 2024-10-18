#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isValidDirectory(char *directorio);
void options();
int menu();

int main(int argc, char *argv[])
{

    char directory[100];

    switch (argc)
    {
    case 1:
        strcpy(directory, "./");
        break;
    case 2:
        if (isValidDirectory(argv[1]))
        {
            printf("directorio valido\n");
            strcpy(directory, argv[1]);
        }
        else
        {
            printf("directorio no valido\n");
        }

        break;

    default:
        printf("no es valido\n");
        return 0;
    }

    int option = menu();

    char comando[200];

    switch (option)
    {
    case 1:
        strcat(comando, "ls -1 ");
        strcat(comando, directory);
        strcat(comando, " | wc -l");
        printf("numero de archivos: ");
        system(comando);
        break;

    case 2:
        strcat(comando, "ls -1 ");
        strcat(comando, directory);
        strcat(comando, " | cut -c 1 |grep 'd' | wc -l");
        printf("numero de directorios: ");
        system(comando);
        break;

    case 3:
        strcat(comando, "ls -l ");
        strcat(comando, directory);
        strcat(comando, " | tr -s ' ' '\t' | cut -f5 | sort -n | tail -n 1");

        printf("el archivo que mas pesa: ");
        system(comando);

        break;

    case 4:
        strcat(comando, "ls -l ");
        strcat(comando, directory);
        strcat(comando, " | tr -s ' ' '\t' | cut -f5 | sort -r | tail -n 2");

        printf("el archivo que menos pesa: ");
        system(comando);
        printf("bytes\n");
        break;

    case 5:
        strcat(comando, "du -sh ");
        strcat(comando, directory);
        system(comando);
        break;

    case 6:
        strcat(comando, "ls -l ");
        strcat(comando, directory);
        strcat(comando, " | cut -c 2 |grep 'r' | wc -l");
        printf("numero de archivos con permisos de lectura para este usuario");
        system(comando);

        break;

    case 7:
        strcat(comando, "ls -l ");
        strcat(comando, directory);
        strcat(comando, " | cut -c 3 |grep 'w' | wc -l");
        printf("numero de archivos con permisos de escritura para este usuario");
        system(comando);

        break;

    case 8:
        strcat(comando, "ls -l ");
        strcat(comando, directory);
        strcat(comando, " | cut -c 4 |grep 'x' | wc -l");
        printf("numero de archivos con permisos de escritura para este usuario");
        system(comando);

        break;
    case 9:
        strcat(comando, "ls -l ");
        strcat(comando, directory);
        strcat(comando, " | cut -c 10 |grep 'x' | wc -l");
        printf("numero de archivos con permisos de escritura para todos los usuarios");
        system(comando);

        break;

    default:
        printf("has salido\n");
        break;
    }

    return 0;
}

int isValidDirectory(char *directorio)
{
    return fopen(directorio, "r") != NULL;
}

int menu()
{

    int a = 0;

    while (a <= 0 || a >= 11)
    {

        options();
        printf("opcion:");
        scanf("%d", &a);

        if (a <= 0 || a >= 11)
        {
            printf("OPCION INCORRECTA");
        }
    }
    return a;
}

void options()
{
    printf("1- numero de ficheros\n2- numero de subdirectorios\n3- fichero mas grande\n4- fichero mas pequeño\n5- espacio ocupado\n6- numero de ficheros con lectura\n7- numero de ficheros con escritura\n8- numero de ficheros con ejecucion \n9- numero de ficheros con ejecucion para todos\n10- salir\n\n");
}