#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <pthread.h>


char **strings = NULL;
int P, K, N, L, WT, nk;
char *filename, ST;
int occupied = 0;

void *manufacturer_action(void *args);
void *customer_action(void *args);


void end_program(int nr){
    printf("\nEnding program\n");
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


    // running manufacturers
    pthread_t *manufacturers = malloc(P * sizeof(pthread_t));
    int *man_nrs = malloc(P * sizeof(int));
    for(int i= 0; i < P; i++){
        man_nrs[i] = i+1;
        pthread_create(&manufacturers[i], NULL, manufacturer_action, &man_nrs[i]);
    }

    // running customers
    pthread_t *customers = malloc(K * sizeof(pthread_t));
    int *cust_nrs = malloc(K * sizeof(int));
    for(int i= 0; i < K; i++){
        cust_nrs[i] = i+1;
        pthread_create(&customers[i], NULL, customer_action, &cust_nrs[i]);
    }


    // joining manufacturers
    for(int i=0; i<P; i++){
        pthread_join(manufacturers[i], NULL);
    }

    // joining customers
    for(int i=0; i<K; i++){
        pthread_join(customers[i], NULL);
    }

    free(man_nrs);
    free(cust_nrs);
    free(customers);
    free(manufacturers);

//while(1){}
    return 0;
}


void *manufacturer_action(void *args){
    int nr = *(int *)args;
    printf("%d man dzialam\n", nr);

}

void *customer_action(void *args){
    int nr = *(int *)args;
    printf("%d cust dzialam\n", nr);

}
