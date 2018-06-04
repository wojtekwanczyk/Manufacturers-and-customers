#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <pthread.h>
#include <zconf.h>


char **strings = NULL;
int P, K, N, L, WT, nk;
char *filename, ST;
int man_nr = 0;
int cust_nr = 0;
FILE *fp;

pthread_mutex_t file_mutex;
pthread_mutex_t tab_mutex;

pthread_cond_t tab_not_full;
pthread_cond_t tab_not_empty;
pthread_cond_t end;


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

    fp = fopen(filename, "r");
    strings = malloc(N * sizeof(char*));
    //file_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init (&file_mutex, NULL);
    pthread_mutex_init (&tab_mutex, NULL);

    pthread_cond_init(&tab_not_full, NULL);
    pthread_cond_init(&tab_not_empty, NULL);
    pthread_cond_init(&end, NULL);


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

    pthread_mutex_destroy(&file_mutex);
    pthread_mutex_destroy(&tab_mutex);

    pthread_cond_destroy(&tab_not_full);
    pthread_cond_destroy(&tab_not_empty);
    pthread_cond_destroy(&end);

    free(man_nrs);
    free(cust_nrs);
    free(customers);
    free(manufacturers);

    fclose(fp);

//while(1){}
    return 0;
}


void *manufacturer_action(void *args){
    int nr = *(int *)args;

    if(WT){
        printf("Manufacturer nr %d started\n", nr);
    }


    char *line = NULL;
    size_t n = 0;
    ssize_t read;

    //pthread_mutex_lock(&file_mutex);
    read = getline(&line, &n, fp);
    //pthread_mutex_unlock(&file_mutex);

    while(read != -1){
        if(WT){
            printf("Manufacturer nr %d: put line: %s", nr, line);
            fflush(stdout);
        }

        pthread_mutex_lock(&tab_mutex);
        while((man_nr == cust_nr -1) || (cust_nr == 0 && man_nr == N-1)){
            printf("Manufacturer nr %d is waiting, buffer is full!!!\n", nr);
            pthread_cond_wait(&tab_not_full, &tab_mutex);
        }
        strings[man_nr] = line;
        //printf("MAN_NR: %d\n", man_nr);
        man_nr = (man_nr + 1) % N;
        pthread_mutex_unlock(&tab_mutex);

        pthread_cond_signal(&tab_not_empty);

        usleep(1);
        // make getline to allocate memory
        line = NULL;
        n = 0;
        //pthread_mutex_lock(&file_mutex);
        read = getline(&line, &n, fp);
        //pthread_mutex_unlock(&file_mutex);
    }

    if(line)
        free(line);
}

void *customer_action(void *args){
    int nr = *(int *)args;
    if(WT){
        printf("Customer nr %d started\n", nr);
    }
    char *line;
    int nrtab;




    while(1) {

        pthread_mutex_lock(&tab_mutex);
        while (man_nr == cust_nr) {
            printf("Customer nr %d is waiting, buffer is empty!!!\n", nr);
            pthread_cond_wait(&tab_not_empty, &tab_mutex);
        }
        line = strings[cust_nr];
        strings[cust_nr] = NULL;
        nrtab = cust_nr;
        //printf("CUST_NR: %d\n", cust_nr);
        cust_nr = (cust_nr + 1) % N;
        pthread_mutex_unlock(&tab_mutex);

        pthread_cond_signal(&tab_not_full);


        printf("Customer nr %d got line from index %d: %s", nr, nrtab, line);
        fflush(stdout);


        usleep(1);
    }
}
