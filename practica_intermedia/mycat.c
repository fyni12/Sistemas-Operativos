#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define Kb_size 1024

void leer(int descriptor, char *buffer);

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("No such file or directory");
    }
    else
    {
        int descriptor;

        descriptor = open(argv[1], O_RDONLY);

        if (descriptor == -1)
        {

            printf("No such file or directory");
        }
        else
        {
            char buffer[Kb_size*4]; // alamcena hasta 4kb de info
            leer(descriptor, buffer);
            printf("%s", buffer);
        }
        close(descriptor);
    }

    return 0;
}

void leer(int descriptor, char *buffer)
{
    size_t B_leidos = read(descriptor, buffer, Kb_size*4); //lee como maximo 4kb
    buffer[B_leidos] = '\0'; // añade el final de linea
}
