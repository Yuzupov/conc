
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>

#define NANO 1000000000
#define CLOCKSTART clock_gettime(CLOCK_MONOTONIC, &t_start)
#define CLOCKSTOP clock_gettime(CLOCK_MONOTONIC, &t_stop)
#define AMOUNT_MEN 1
#define AMOUNT_WOMEN 1
#define WOMAN 1
#define MAN 2
#define BATHROOM_SIZE (AMOUNT_MEN + AMOUNT_WOMEN) / 2

int num_men = 0;
int num_women = 0;

sem_t b;
sem_t q;

struct timespec t_start, t_stop;
typedef struct bathroom {
	int bathroom_pointer;
	int *bathroom;
} bathroom;
struct thread_structs {
	int t_id;
	int gender;
};

void enter_male();
void enter_female();
void *do_work(void *);
void go_bathroom(void *, float);
bathroom* create_bathroom(int);
long nano_seconds(struct timespec *, struct timespec *);
//void enqueue(int *queue, int tail, int gender);
void *do_work(void *);


int main(int argc, char *argv[]){
	sem_init(&b, 0, 1);
	sem_init(&q, 0, 1);
	int NTHREADS = atoi(argv[1]);
	struct thread_structs struct_array[NTHREADS];
	pthread_t thread_id[NTHREADS];
	for(int i = 0; i < NTHREADS; i++){
		struct_array[i].t_id = i;
		if(i % 2 == 0)
			struct_array[i].gender = 0;
		else
			struct_array[i].gender = 1;

		pthread_create( &thread_id[i], NULL, do_work, (void *) &struct_array[i]);
		
	}
	for(int i = 0; i < NTHREADS; i++){
		pthread_join(thread_id[i], NULL);
	}

	return 0;
}

bathroom *create_bathroom(int size){
	int *bathroom_array = calloc(size, sizeof(int));
	bathroom *bth = malloc(sizeof(bathroom));
	bth->bathroom_pointer = 0;
	bth->bathroom = bathroom_array;
}

void *do_work(void *threadarg){
	struct thread_structs *thread_struct;
	thread_struct = (struct thread_structs *) threadarg;
	while(1){
		float temp = (float)rand()/(float)(RAND_MAX/5);
		sleep(temp);
		go_bathroom((void *) thread_struct, temp);
		

	}
}

void go_bathroom(void *threadarg, float time){
	int *men = &num_men;
	int *women = &num_women;
	struct thread_structs *thread_struct;
	thread_struct = (struct thread_structs *) threadarg;
	if(thread_struct->gender == 0){
		sem_wait(&b);
		while(*women > 0){
		}
		(*men)++;
		sem_post(&b);
		float temp = time/2;
		printf("amount of men: %d\t", *men);
		printf("amount of women: %d\n", *women);
		sleep(temp);
		sem_wait(&q);
		(*men)--;
		sem_post(&q);
		
	} else if(thread_struct->gender == 1) {
		sem_wait(&b);
		while(*men > 0){
		}
		(*women)++;
		sem_post(&b);
		float temp = time/2;
		printf("amount of men: %d\t", *men);
		printf("amount of women: %d\n", *women);
		sleep(temp);
		sem_wait(&q);
		(*women)--;
		sem_post(&q);
	}
	return;
}

long nano_seconds(struct timespec *t_start, struct timespec *t_stop){
	return (t_stop->tv_nsec - t_start->tv_nsec) +
		(t_stop->tv_sec - t_start->tv_sec)*NANO;
}
