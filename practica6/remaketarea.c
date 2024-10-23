#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

void launch(int val, pid_t index[5]);
void relaunch(pid_t pids[5]);
void handler();

int main(void)
{

    struct sigaction nombre;
    
    nombre.sa_handler=SIG_IGN;


    int exitestat, value;

    pid_t identifiers[5];
    sigaction(SIGINT, &nombre, NULL);

    for (int i = 1; i <= 5; i++)
    {
        launch(i, identifiers);
    }
   

    while (1)
    {
        printf("esperando...\n");
        pid_t cosa;
        cosa = wait(&exitestat);
        relaunch(identifiers);



    }

    return 0;
}

void launch(int val, pid_t index[5])
{

    // printf("proceso lanzado\n");
    if (val < 1 || val > 5)
    {
        return;
    }

    pid_t indent;
    indent = fork();

    if (indent == 0)
    {
        switch (val)
        {
        case 1:
            printf("lanzando xload\n");
            execl("/usr/bin/xload", "xload", (char *)0);
            exit(1);
            break;
        case 2:
            printf("lanzando xeyes\n");

            execl("/usr/bin/xeyes", "xeyes", (char *)0);
            exit(2);
            break;
        case 3:
            printf("lanzando xlogo\n");

            execl("/usr/bin/xlogo", "xlogo", (char *)0);
            exit(3);
            break;
        case 4:
            printf("lanzando xcalc\n");

            execl("/usr/bin/xcalc", "xcalc", (char *)0);
            exit(4);
            break;
        case 5:
            printf("lanzando xclock\n");
            
            execl("/usr/bin/xclock", "xclock", "-update", "1", (char *)0);
            exit(5);
            break;
        default:
            exit(-1);
        }
    }
    else
    {

        index[val - 1] = indent;
    }
}

void relaunch(pid_t pids[5])
{
    int i = 1;
    while (i <=5)
    {
        char comando[100] = "";
        char pid[10];
        sprintf(pid, "%d", pids[i-1]);

        strcat(comando, "./isProcessRunning ");
        strcat(comando, pid);
        int exitval = system(comando);

        if(exitval<1){
            
            launch(i, pids);
            i=6;
        }
        i++;
    }
}

void handler(){
    printf("que haces tonto??? >:( \n");
}
