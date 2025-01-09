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

FILE *file_ptr;
FILE *write_file;
int NTHREADS;
int amount_of_palindromics;
char input_buffer[BUFFER_INDEX_SIZE*AMOUNT_OF_WORDS];

struct thread_indices {
	int start;
	int end;
	int t_id;
	int found;
};

struct thread_indices thread_indices_array[AMOUNT_OF_WORDS];
struct timespec t_start, t_stop;

void *find_palindromic(void *);
void read_file(char *);
void print_read_file();
int lookup(char *);
long nano_seconds(struct timespec*, struct timespec*);

int main(int argc, char *argv[]){
	if(argc > 3 || argc <= 2 || strcmp(argv[1], "-W") != 0){
		printf("Wrong amount of arguments. Please run the program with: $ ./palindromic -W <amount of workers>\n");
		exit(1);
	}
	NTHREADS = atoi(argv[2]);
	if(NTHREADS > AMOUNT_OF_WORDS){
		printf("just don't use this many threads, try again\n");
		exit(2);
	}
	printf("amount of threads: %d\n", NTHREADS);
	read_file("british-english");
	struct thread_indices *indices;
	int thread_loop_limit = AMOUNT_OF_WORDS / NTHREADS;
	write_file = fopen("results.txt", "w");

	pthread_t thread_id[NTHREADS];
	struct timeval start;
	struct timeval stop;
	printf("starting\n");
	clock_gettime(CLOCK_MONOTONIC, &t_start);	
	for(int i = 0; i < NTHREADS; i++){
		if(i == 0){
			thread_indices_array[i].start = i * thread_loop_limit;
		} else {
			thread_indices_array[i].start = i * thread_loop_limit + (AMOUNT_OF_WORDS % NTHREADS) + 1;
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
		pthread_join(thread_id[j], NULL);
	}
	clock_gettime(CLOCK_MONOTONIC, &t_stop);
	printf("stopped\n");
	long total_time = nano_seconds(&t_start, &t_stop);
	printf("total time: %0.2f seconds for %d threads\n", (double)total_time/NANO, NTHREADS);
	fprintf(write_file, "total time: %0.2f seconds for %d threads\n", (double)total_time / NANO, NTHREADS);
	fclose(write_file);
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
		if(strcmp(word, temp) == 0){
			return 0;	
		}
	}
	return 1;
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
	struct thread_indices *my_limits;
	my_limits = (struct thread_indices *) threadarg;
	int start = my_limits->start;
	int end = my_limits->end;
	printf("start: %d end: %d pid: %d\n", start, end, my_limits->t_id);
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
		if(res == 0){
			my_limits->found++;
		}
	}
	printf("A total of %d palindromics found by %d\n", my_limits->found, my_limits->t_id);
	fprintf(write_file, "A total of %d palindromics found by %d\n", my_limits->found, my_limits->t_id);
}
