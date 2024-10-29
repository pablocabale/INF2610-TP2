#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define MAX 100

int buffer[MAX]; 
int buffer_size;
int production_index = 0;
int consumption_index = 0;
int end_flag = 0;
int total_produced = 0;
int total_consumed = 0;
int sum_produced = 0;
int sum_consumed = 0;

sem_t full;  
sem_t empty;   
sem_t mutex;

void alarm_handler(int sig) {
    end_flag = 1;
}

void* producteur(void* pid) {
    int id = *((int*)pid);
    int number;
    srand(time(NULL) + id);

    while (1) {
        if (end_flag) break;  

        number = (rand() % 9) + 1; 

        sem_wait(&empty);  
        sem_wait(&mutex); 

        buffer[production_index] = number;
        production_index = (production_index + 1) % buffer_size;
        sum_produced += number;
        total_produced++;

        sem_post(&mutex); 
        sem_post(&full);  

        printf("Producteur %d a produit: %d\n", id, number);
    }

    pthread_exit(&sum_produced);  
}

void* consommateur(void* cid) {
    int id = *((int*)cid);
    int number;

    while (1) {
        sem_wait(&full); 
        sem_wait(&mutex);

        number = buffer[consumption_index];
        consumption_index = (consumption_index + 1) % buffer_size;

        sem_post(&mutex); 
        sem_post(&empty);

        if (number == 0) break; 

        sum_consumed += number;
        total_consumed++;

        printf("Consommateur %d a consommé: %d\n", id, number);
    }
    pthread_exit(&sum_consumed);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <nombre de producteurs> <nombre de consommateurs> <taille du tampon>\n", argv[0]);
        return 1;
    }

    int num_producers = atoi(argv[1]);
    int num_consumers = atoi(argv[2]);
    buffer_size = atoi(argv[3]);

    if (buffer_size > MAX) {
        printf("Buffer size too large! Limited to %d\n", MAX);
        return 1;
    }

    sem_init(&full, 0, 0);
    sem_init(&empty, 0, buffer_size);
    sem_init(&mutex, 0, 1);         


    signal(SIGALRM, alarm_handler);
    alarm(1);

    pthread_t producers[num_producers];
    pthread_t consumers[num_consumers];
    int pid[num_producers], cid[num_consumers];

    pthread_t producers[num_producers];
    pthread_t consumers[num_consumers];
    int pid[num_producers], cid[num_consumers];

    for (int i = 0; i < num_producers; i++) {
        pid[i] = i;
        pthread_create(&producers[i], NULL, producteur, &pid[i]);
    }

    for (int i = 0; i < num_consumers; i++) {
        cid[i] = i;
        pthread_create(&consumers[i], NULL, consommateur, &cid[i]);
    }
}
