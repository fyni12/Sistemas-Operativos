#include <unistd.h>
#include <stdio.h>
#include <signal.h>

void handler(){
    printf(":3\n");
}

int main(){

    signal(SIGALRM, handler);

    while(1){
        alarm(1);
        pause();
        
        printf("%d\n", time(NULL));
    }



    return 0;
}