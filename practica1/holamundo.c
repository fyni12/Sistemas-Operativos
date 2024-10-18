#include <stdio.h>
#include <unistd.h>

void espera(int time);

int main()
{

	printf("¡hola mundo!\n");
	espera(100);

	return 0;
}

void espera(int time)
{

	sleep(time);
	printf("han pasado %d\n", time);
}
