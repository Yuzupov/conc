#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>
#include <pthread.h>
//defining some constants
#define NANO 1000000000
#define READ_MAX 99
#define AMOUNT_OF_WORDS 20000000
#define BUFFER_INDEX_SIZE 100
#define MAX_ACTIVE_THREADS 1
//declaring some variables
FILE *file_ptr;
FILE *write_file;
int NTHREADS;
int amount_of_palindromics;
char input_buffer[BUFFER_INDEX_SIZE*AMOUNT_OF_WORDS];
int amount_rows_in_file;
//declaring a struct that each thread gets passed when created
struct thread_indices {
	int start;
	int end;
	int t_id;
	int found;
};
//declaring a struct that saves each word that a thread founds
struct found_words_s {
	int found_indices[AMOUNT_OF_WORDS];
	int index;
};
//declaring the above struct as global so that it does not need to be passed to each thread 
struct found_words_s found_words;
//declaring timespec structs for performance measurement
struct timespec t_start, t_stop;
//initalizing the mutex for writing to shared variables
pthread_mutex_t mutexwrite = PTHREAD_MUTEX_INITIALIZER;
//declaring functions
void *find_palindromic(void *);
void read_file(char *);
void print_read_file();
int lookup(char *);
void count_rows(char *);
long nano_seconds(struct timespec*, struct timespec*);


int main(int argc, char *argv[]){
	//checks for argument validity
	if(argc > 4 || argc <= 2 || strcmp(argv[1], "-W") != 0){
		printf("Wrong amount of arguments. Please run the program with: $ ./palindromic -W <amount of workers>\n");
		exit(1);
	}
	//checks for amount of thread validity
	NTHREADS = atoi(argv[2]);
	if(NTHREADS > AMOUNT_OF_WORDS || NTHREADS >= 2147483647){
		printf("just don't use this many threads, try again\n");
		exit(2);
	}
	//corner case for integer overflow
	if(NTHREADS <= 0){
		printf("need at least 1 thread\n");
		exit(3);
	}
	printf("amount of threads: %d\n", NTHREADS);
	//read the file, if there is a filename in the CLI after the threads argument it will read from that file 
	argc == 4 ? read_file(argv[3]) : read_file("words.txt");
	argc == 4 ? count_rows(argv[3]) : count_rows("words.txt");
	printf("amount rows: %d\n", amount_rows_in_file);
	//declare an array to hold the individual structs for each thread	
	struct thread_indices thread_indices_array[NTHREADS];
	//declaring how many "blocks" the threads will have to read
	int thread_loop_limit = amount_rows_in_file / NTHREADS;
	//setting the saved words struct starting index to 0
	found_words.index = 0;
	//declare the pthreads thread_id array
	pthread_t thread_id[NTHREADS];
	//declare the timveal structs
	struct timeval start;
	struct timeval stop;
	//start the performance measurement 
	clock_gettime(CLOCK_MONOTONIC, &t_start);	
	//the loop to create threads
	for(int i = 0; i < NTHREADS; i++){
		//if it's the first thread it's just 0
		if(i == 0){
			thread_indices_array[i].start = i * thread_loop_limit;
		} else {
			//if it isn't the first then give it the index of the block + any potential residual 
			thread_indices_array[i].start = i * thread_loop_limit + (amount_rows_in_file % NTHREADS);
		}
		//give it the end of the limit
		if(i != NTHREADS){
			thread_indices_array[i].end = (i + 1) * thread_loop_limit;
		}
		thread_indices_array[i].end = (i + 1) * thread_loop_limit + (amount_rows_in_file % NTHREADS);
		//in short the above function is strict on the ending indices, but adjusts where a thread starts
		//for an amount of threads that don't evenly divide the size of the input file, with the 
		//exception of the final thread, which has to take that into consideration
		//
		//below are just the rest of the initializers for the id and how many it has found 
		thread_indices_array[i].t_id = i;
		thread_indices_array[i].found= 0;
		//create the thread with the thread function and the struct it should use
		pthread_create( &thread_id[i], NULL, &find_palindromic, (void *) &thread_indices_array[i]);
	}
	//variable for checking how many words the threads found in total
	int thread_sum = 0;
	for(int j = 0; j < NTHREADS; j++){
		//temporary pointer to catch the return value of each thread
		void *thread_return_ptr = NULL;
		//barrier that waits for each thread to finish, sums the return value in thread_sum
		pthread_join(thread_id[j], &thread_return_ptr);
		thread_sum += *(int *)thread_return_ptr;
	}
	clock_gettime(CLOCK_MONOTONIC, &t_stop);
	printf("sum from threads: %d\n", thread_sum);
	//sanity check print to validate that the returned value and the global variable they access are the same
	printf("verify that threads return value and global variable, if == 0 it was successful. val: %d\n", amount_of_palindromics - thread_sum);
	//open a file in write mode to the variable write_file
	write_file = fopen("results.txt", "w");
	//write sequentially how many words each individual thread found
	for(int j = 0; j < NTHREADS; j++){
		fprintf(write_file, "A total of %d palindromics found by thread %d\n", thread_indices_array[j].found, thread_indices_array[j].t_id);
	}
	//write the actual words to the file
	for(int i = 0; i < found_words.index; i++){
		char word[20]; 
		strcpy(word, &input_buffer[found_words.found_indices[i] * BUFFER_INDEX_SIZE]);
		//printf("index in the struct: %d\n", found_words.index);
		fprintf(write_file, "%s\n", word);
	}
	//convert the time taken to nanoseconds
	long total_time = nano_seconds(&t_start, &t_stop);
	printf("total time: %0.2f seconds for %d threads\n", (double)total_time/NANO, NTHREADS);
	fprintf(write_file, "total time: %0.2f seconds for %d threads to find %d palindromics\n", (double)total_time / NANO, NTHREADS, thread_sum);
	//close the file pointer 
	fclose(write_file);
	//destroy the mutex
	pthread_mutex_destroy(&mutexwrite);
	return 0;
}

//function for converting the values in the 
long nano_seconds(struct timespec *t_start, struct timespec *t_stop){
	return (t_stop->tv_nsec - t_start->tv_nsec) +
		(t_stop->tv_sec - t_start->tv_sec)*NANO;
}
//lookup function with a standard linear search
int lookup(char *word){
	//checks if the line read is a null terminator
	if(*word == '\0'){
		return 0;
	}
	for(int i = 0; i < amount_rows_in_file; i++){
		//loads the word into a temp buffer
		char *dictionary_word = &input_buffer[i * BUFFER_INDEX_SIZE];
		//ignores the case when comparing
		if(strcasecmp(word, dictionary_word) == 0){
			return 1;	
		}
	}
	return 0;
}

//just prints the buffer that the read_file() function stores the words in 
void print_read_file(){
	for(int i = 0; i < amount_rows_in_file; i++){
		printf("word printed: %s", &input_buffer[i * BUFFER_INDEX_SIZE]); 
	}
}

void count_rows(char *filename){
	file_ptr = fopen(filename, "r");
	if(file_ptr == NULL){
		printf("Unable to open file\n");
		exit(1);
	}
	char temp_read_buffer[READ_MAX];
	int i = 0;
	while(fgets(temp_read_buffer, READ_MAX, file_ptr)){
		//trim newlines
		temp_read_buffer[strcspn(temp_read_buffer, "\n")] = 0;
		//put the word into the buffer that holds the dictionary
		strcpy(&input_buffer[i * BUFFER_INDEX_SIZE], temp_read_buffer);
		amount_rows_in_file++;
		i++;
	}
	fclose(file_ptr);
}

//the read input file function, takes a filename as argument
void read_file(char *filename){
	//opens the file in read mode 
	file_ptr = fopen(filename, "r");
	//check if successful, exit program if not
	if(file_ptr == NULL){
		printf("Unable to open file\n");
		exit(1);
	}
	int i = 0;
	//declares the largest size of readable word, will be the same size as how big the index differences are in
	//the buffer with the stored words, i.e. 100
	char temp_read_buffer[READ_MAX];
	//read line for at most READ_MAX chars, store in the temporary read buffer
	while(fgets(temp_read_buffer, READ_MAX, file_ptr)){
		//trim newlines
		temp_read_buffer[strcspn(temp_read_buffer, "\n")] = 0;
		//put the word into the buffer that holds the dictionary
		strcpy(&input_buffer[i * BUFFER_INDEX_SIZE], temp_read_buffer);
		i++;
	}
	//close the file
	fclose(file_ptr);
}

//the thread function itself, has a pointer to a struct as an argument
void *find_palindromic(void *threadarg){
	//malloc a pointer to hold the return value of the function
	int *thread_found = malloc(sizeof(int));
	//make a local struct pointer to cast the argument to
	struct thread_indices *my_limits;
	//cast said argument
	my_limits = (struct thread_indices *) threadarg;
	//set some local variables for easier reading, defines where in the input buffer the thread will be reading
	int start = my_limits->start;
	int end = my_limits->end;
	for(; start < end; start++){
		//temporary buffer to manipulate words from the dictionary buffer
		char word[READ_MAX];
		//copy the word from the dictionary buffer into said word buffer
		strcpy(word, &input_buffer[start * BUFFER_INDEX_SIZE]);
		//reverse the word
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
		//check if the word exists in the dictionary, it will find both palindromes and semordnilaps
		int res = lookup(word);
		if(res == 1){
			//if the word is found, request a lock and edit shared data, else wait for a lock to be free
			pthread_mutex_lock(&mutexwrite);
			amount_of_palindromics++;
			found_words.found_indices[found_words.index] = start;
			found_words.index++;
			pthread_mutex_unlock(&mutexwrite);
			//thread specific data is updated so no mutex needed
			my_limits->found++;
			(*thread_found)++;
		}
	}
	//a runtime print for each thread, mainly for quick viewing of the results
	printf("A total of %d palindromics found by %d\n", my_limits->found, my_limits->t_id);
	//let thread exit gracefully when finished, send the previous pointer as return value to be caught when joining
	pthread_exit(thread_found);
}
