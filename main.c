#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include <pthread.h>
#include <zconf.h>
#include <sys/times.h>


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

    long persec = sysconf(_SC_CLK_TCK);
    struct tms t;
    clock_t start = times(&t);
    clock_t end = start + (nk * persec);

    if(nk == 0){
        if(signal(SIGINT, end_program) == SIG_ERR){
            printf("Error while setting sigint handler\n");
            exit(1);
        }
    }

    // check parsing
    printf("%d %d %d %s %d %c %d %d\n", P, K, N, filename, L, ST, WT, nk);

    fp = fopen(filename, "r");
    strings = malloc(N * sizeof(char*));
    //file_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init (&file_mutex, NULL);
    pthread_mutex_init (&tab_mutex, NULL);

    pthread_cond_init(&tab_not_full, NULL);
    pthread_cond_init(&tab_not_empty, NULL);


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


    if(nk != 0){
        while(start < end){
            start = times(&t);
        }
        printf("End of given time.\n");
        // cancelling manufacturers
        for(int i=0; i<P; i++){
            pthread_cancel(manufacturers[i]);
        }
        // cancelling customers
        for(int i=0; i<K; i++){
            pthread_cancel(customers[i]);
        }
    } else {

        // joining manufacturers
        for (int i = 0; i < P; i++) {
            pthread_join(manufacturers[i], NULL);
        }

        printf("End of work for manufacturers. Closing program\n");
        fflush(stdout);

        // cancelling customers
        for (int i = 0; i < K; i++) {
            pthread_cancel(customers[i]);
        }
    }

    fclose(fp);

    pthread_mutex_destroy(&file_mutex);
    pthread_mutex_destroy(&tab_mutex);

    pthread_cond_destroy(&tab_not_full);
    pthread_cond_destroy(&tab_not_empty);

    free(man_nrs);
    free(cust_nrs);
    free(customers);
    free(manufacturers);

    return 0;
}


void *manufacturer_action(void *args){
    int nr = *(int *)args;

    if(WT){
        printf("Manufacturer nr %d started\n", nr);
        fflush(stdout);
    }

    char *line = NULL;
    size_t n = 0;
    ssize_t read;

    //pthread_mutex_lock(&file_mutex);
    read = getline(&line, &n, fp);
    //pthread_mutex_unlock(&file_mutex);

    while(read != -1){


        pthread_mutex_lock(&tab_mutex);
        while((man_nr == cust_nr -1) || (cust_nr == 0 && man_nr == N-1)){
            if(WT){
                printf("Manufacturer nr %d is waiting, buffer is full!!!\n", nr);
                fflush(stdout);
            }
            pthread_cond_wait(&tab_not_full, &tab_mutex);
        }
        if(WT){
            printf("Manufacturer nr %d put line on index %d: %s", nr, man_nr, line);
            fflush(stdout);
        }
        strings[man_nr] = line;
        man_nr = (man_nr + 1) % N;

        pthread_mutex_unlock(&tab_mutex);

        pthread_cond_signal(&tab_not_empty);

        // make getline to allocate memory
        line = NULL;
        n = 0;
        // to see end of work in the give time (ex 1 s) ------------------------------------------
        //usleep(1000);
        //pthread_mutex_lock(&file_mutex);
        read = getline(&line, &n, fp);
        //pthread_mutex_unlock(&file_mutex);
    }

    return NULL;
}

void *customer_action(void *args){
    int nr = *(int *)args;
    if(WT){
        printf("Customer nr %d started\n", nr);
        fflush(stdout);
    }
    char *line;
    int nrtab;
    size_t len;

    while(1) {
        pthread_mutex_lock(&tab_mutex);
        while (man_nr == cust_nr) {
            if(WT){
                printf("Customer nr %d is waiting, buffer is empty!!!\n", nr);
                fflush(stdout);
            }
            pthread_cond_wait(&tab_not_empty, &tab_mutex);
        }
        line = strings[cust_nr];
        strings[cust_nr] = NULL;
        nrtab = cust_nr;

        if(WT)
            printf("Customer nr %d got line from index %d: %s", nr, nrtab, line);
        len = strlen(line);
        if((ST == '>' && len > L) || (ST == '=' && len == L)
           || (ST == '<' && len < L) ){
            printf("FOUND: Customer nr %d got matching line from index %d: %s", nr, nrtab, line);
            fflush(stdout);
        }
        cust_nr = (cust_nr + 1) % N;
        pthread_mutex_unlock(&tab_mutex);

        // free memory allocated by getline
        if(line)
            free(line);

        pthread_cond_signal(&tab_not_full);
    }
}
