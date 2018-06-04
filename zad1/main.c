#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>


char **strings = NULL;
int P, K, N, L, WT, nk;
char *filename, ST;
int occupied = 0;

void end_program(int nr){
    printf("Ending program\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    if(argc != 9){
        printf("program must take 8 arguments\n"
               "./main P K N filename L search_type writing_type nk\n\n");
        exit(1);
    }
    if(signal(SIGINT, end_program) == SIG_ERR){
        printf("Error while setting sigint handler\n");
        exit(1);
    }
    P = (int)strtol(argv[1], NULL, 10);
    K = (int)strtol(argv[2], NULL, 10);
    N = (int)strtol(argv[3], NULL, 10);
    filename = argv[4];
    L = (int)strtol(argv[5], NULL, 10);
    ST = argv[6][0];
    if(strcmp("on", argv[7]) == 0){
        printf("Writing type turned on!\n");
        WT = 1;
    } else {
        WT = 0;
    }
    nk = (int)strtol(argv[8], NULL, 10);

    // check parsing
    printf("%d %d %d %s %d %c %d %d\n", P, K, N, filename, L, ST, WT, nk);




while(1){}
    return 0;
}


void manufacturer(void *args){
    int nr = *(int *)args;


}
