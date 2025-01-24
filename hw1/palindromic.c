#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>
#include <pthread.h>

#define NANO 1000000000
#define READ_MAX 16384
#define AMOUNT_OF_WORDS 101921 
#define BUFFER_INDEX_SIZE 100
#define MAX_ACTIVE_THREADS 1

FILE *file_ptr;
FILE *write_file;
int NTHREADS;
int amount_of_palindromics;
char input_buffer[BUFFER_INDEX_SIZE*AMOUNT_OF_WORDS];
int active_threads = 0;

struct thread_indices {
	int start;
	int end;
	int t_id;
	int found;
};

struct saved_indices_s {
	int found_indices[AMOUNT_OF_WORDS];
	int index;
};

struct timespec t_start, t_stop;
pthread_mutex_t mutexwrite = PTHREAD_MUTEX_INITIALIZER;

void *find_palindromic(void *);
void read_file(char *);
void print_read_file();
int lookup(char *);
long nano_seconds(struct timespec*, struct timespec*);

struct saved_indices_s saved_indices;

int main(int argc, char *argv[]){
	if(argc > 3 || argc <= 2 || strcmp(argv[1], "-W") != 0){
		printf("Wrong amount of arguments. Please run the program with: $ ./palindromic -W <amount of workers>\n");
		exit(1);
	}
	NTHREADS = atoi(argv[2]);
	if(NTHREADS > AMOUNT_OF_WORDS || NTHREADS >= 2147483647){
		printf("just don't use this many threads, try again\n");
		exit(2);
	}
	if(NTHREADS <= 0){
		printf("need at least 1 thread\n");
		exit(3);
	}
	printf("amount of threads: %d\n", NTHREADS);

	read_file("british-english");

	int thread_sum = 0;
	struct thread_indices thread_indices_array[NTHREADS];
	struct thread_indices *indices;
	int thread_loop_limit = AMOUNT_OF_WORDS / NTHREADS;
	write_file = fopen("results.txt", "w");
	int found_words_indices[AMOUNT_OF_WORDS];
	saved_indices.index = 0;
	pthread_t thread_id[NTHREADS];
	struct timeval start;
	struct timeval stop;

	clock_gettime(CLOCK_MONOTONIC, &t_start);	
	for(int i = 0; i < NTHREADS; i++){
		if(i == 0){
			thread_indices_array[i].start = i * thread_loop_limit;
		} else {
			thread_indices_array[i].start = i * thread_loop_limit + (AMOUNT_OF_WORDS % NTHREADS);
		}
		if(i != NTHREADS){
			thread_indices_array[i].end = (i + 1) * thread_loop_limit;
		}
		thread_indices_array[i].end = (i + 1) * thread_loop_limit + (AMOUNT_OF_WORDS % NTHREADS);
		thread_indices_array[i].t_id = i;
		thread_indices_array[i].found= 0;
		pthread_create( &thread_id[i], NULL, find_palindromic, (void *) &thread_indices_array[i]);
	}

	for(int j = 0; j < NTHREADS; j++){
		void *sum = NULL;
		pthread_join(thread_id[j], &sum);
		thread_sum += *(int *)sum;
	}
	printf("sum from threads: %d\n", thread_sum);

	printf("verify that threads return value and global variable, if == 0 it was successful. val: %d\n", amount_of_palindromics - thread_sum);

	for(int j = 0; j < NTHREADS; j++){
		fprintf(write_file, "A total of %d palindromics found by thread %d\n", thread_indices_array[j].found, thread_indices_array[j].t_id);
	}
	for(int i = 0; i < saved_indices.index; i++){
		char word[20]; 
		strcpy(word, &input_buffer[saved_indices.found_indices[i] * BUFFER_INDEX_SIZE]);
		//printf("index in the struct: %d\n", saved_indices.index);
		fprintf(write_file, "%s\n", word);
	}
	clock_gettime(CLOCK_MONOTONIC, &t_stop);
	long total_time = nano_seconds(&t_start, &t_stop);
	printf("total time: %0.2f seconds for %d threads\n", (double)total_time/NANO, NTHREADS);
	fprintf(write_file, "total time: %0.2f seconds for %d threads to find %d palindromics\n", (double)total_time / NANO, NTHREADS, thread_sum);
	fclose(write_file);
	pthread_mutex_destroy(&mutexwrite);
	return 0;
}

long nano_seconds(struct timespec *t_start, struct timespec *t_stop){
	return (t_stop->tv_nsec - t_start->tv_nsec) +
		(t_stop->tv_sec - t_start->tv_sec)*NANO;
}

int lookup(char *word){
	if(*word == '\0'){
		printf("not a word\n");
		return 1;
	}
	for(int i = 0; i < AMOUNT_OF_WORDS; i++){
		char *temp = &input_buffer[i * BUFFER_INDEX_SIZE];
		if(strcasecmp(word, temp) == 0){
			//printf("found: %s, \n", word);
			return 1;	
		}
	}
	return 0;
}

void print_read_file(){
	for(int i = 0; i < AMOUNT_OF_WORDS; i++){
		printf("word printed: %s", &input_buffer[i * BUFFER_INDEX_SIZE]); 
	}
}

void read_file(char *filename){
	file_ptr = fopen(filename, "r");
	if(file_ptr == NULL){
		printf("Unable to open file\n");
		exit(1);
	}
	int i = 0;
	char temp_read_buffer[READ_MAX];
	while(fgets(temp_read_buffer, READ_MAX, file_ptr)){
		temp_read_buffer[strcspn(temp_read_buffer, "\n")] = 0;
		strcpy(&input_buffer[i * BUFFER_INDEX_SIZE], temp_read_buffer);
		i++;
	}
	fclose(file_ptr);
}

void *find_palindromic(void *threadarg){
	int *thread_found = malloc(sizeof(int));
	struct thread_indices *my_limits;
	my_limits = (struct thread_indices *) threadarg;
	int start = my_limits->start;
	int end = my_limits->end;
	for(; start < end; start++){
		char word[20];
		strcpy(word, &input_buffer[start * BUFFER_INDEX_SIZE]);
		int l = 0;
		int r = strlen(word) - 1;
		char t;
		while(l < r){
			t = word[l];
			word[l] = word[r];
			word[r] = t;
			l++;
			r--;
		}
		int res = lookup(word);
		if(res == 1){
			pthread_mutex_lock(&mutexwrite);
			active_threads++;
			amount_of_palindromics++;
			saved_indices.found_indices[saved_indices.index] = start;
			saved_indices.index++;
			active_threads--;
			pthread_mutex_unlock(&mutexwrite);
			my_limits->found++;
			(*thread_found)++;
		}
	}
	printf("A total of %d palindromics found by %d\n", my_limits->found, my_limits->t_id);
	pthread_exit((void *) thread_found);
}
