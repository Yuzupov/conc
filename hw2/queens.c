#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>
#include <time.h>
#include <omp.h>

#define NANO 1000000000
#define BOARD_SIZE 64
#define AMOUNT_SOLUTIONS 92
#define CLOCKSTART clock_gettime(CLOCK_MONOTONIC, &t_start)
#define CLOCKSTOP clock_gettime(CLOCK_MONOTONIC, &t_stop)

int *solutions[AMOUNT_SOLUTIONS];
int og_board[BOARD_SIZE];
int solutions_found = 0;
int depth = 0;


int check_row(int, int*);
int check_col(int, int*);
int check_dgl_top_right(int, int*);
int check_dgl_top_left(int, int*);
int check_dgl_bottom_right(int, int*);
int check_dgl_bottom_left(int, int*);
int check_right_wall(int num);
int check_left_wall(int num);
int check_all(int, int*);
long nano_seconds(struct timespec*, struct timespec*);
void print_sample_board();
void print_solution(int *);
void generate_queens_seq(int *, int, int *);
void generate_queens_par(int *, int, int *);
void generate_iteratively();
int comp(const void *, const void *);
int *setup();

int main(int argc, char *argv[]){
	omp_set_num_threads(atoi(argv[1]));
	int loop = 1000;
	double times_par[loop];
	double times_seq[loop];
	int *solutions = malloc(sizeof(int));
	*solutions = 0;
	struct timespec t_start, t_stop;
	struct timeval start;
	struct timeval stop;
	int *board = setup(og_board);
	long wall = LONG_MAX;
	long total_time;
	long average_time = 0;


	for(int i = 0; i < loop; i++){ 
		*solutions = 0;
		board = setup(og_board);
		CLOCKSTART;
		generate_queens_par(board, 0, solutions);
		CLOCKSTOP;
		long parallel_time = nano_seconds(&t_start, &t_stop);
		times_par[i] = (double) parallel_time;
		average_time += parallel_time;
		if(parallel_time < wall)
			wall = parallel_time;
	}
	printf("solutions parallel: %d\n", *solutions);
	printf("recursive parallel quickest solution time: %0.6f s\n", (double)wall/NANO);
	printf("recursive parallel average time: %0.6f s\n", (double)(average_time/loop)/NANO);
	qsort(times_par, sizeof(times_par)/sizeof(*times_par), sizeof(int), comp);
	printf("recursive parallel median time: %0.6f s\n\n", times_par[loop/2]/NANO);
	double diff = (double) wall/NANO;
	solutions_found = 0;
	wall = LONG_MAX;
	average_time = 0;

	for(int i = 0; i < loop; i++){ 
		*solutions = 0;
		board = setup(og_board);
		CLOCKSTART;
		generate_queens_seq(board, 0, solutions);
		CLOCKSTOP;
		long sequential_time = nano_seconds(&t_start, &t_stop);
		times_seq[i] = (double) sequential_time;
		average_time += sequential_time;
		if(sequential_time < wall)
			wall = sequential_time;
	}
	printf("solutions sequential: %d\n", *solutions);
	printf("recursive sequential quickest solution time: %0.6f s\n", (double)wall/NANO);
	printf("recursive sequential average time: %0.6f s\n", (double)(average_time/loop)/NANO);
	qsort(times_seq, loop, sizeof(int), comp);
	printf("recursive sequential median time: %0.6f s\n\n", times_seq[loop/2]/NANO);
	printf("parallell %0.6f s quicker\n\n", (double)wall/NANO - diff);

	return 0;
}

//following function is not my work, it is taken from stack overflow at link: https://stackoverflow.com/questions/1787996/c-library-function-to-perform-sort
int comp (const void * elem1, const void * elem2) {
	double f = *((double*)elem1);
	double s = *((double*)elem2);
	if (f > s) return  1;
	if (f < s) return -1;
	return 0;
}


void generate_queens_par(int *board, int counter, int *solutions){
	if(counter == 8){
		#pragma omp atomic
		*solutions = *solutions + 1;
		return;
	}
	#pragma omp parallel for
	for(int i = counter * 8; i <= counter * 8 + 7; i++){
		//printf("Thread ID: %d\n", omp_get_thread_num());
		int res = 0;

		res = check_all(i, board);
		if(res == 1){
			int local_board[BOARD_SIZE];
			memcpy(local_board, board, sizeof(int) * BOARD_SIZE);
			local_board[i] = 1;
			generate_queens_seq(local_board, counter + 1, solutions);
		}
		board[i] = 0;
	}
}

void generate_queens_seq(int *board, int counter, int *solutions){
	int thread_id;
	if(counter == 8){
		#pragma omp atomic
		*solutions = *solutions + 1;
		return;
	}
	for(int i = counter * 8; i <= counter * 8 + 7; i++){
		if(check_all(i, board) == 1){
			int local_board[BOARD_SIZE];
			memcpy(local_board, board, sizeof(int) * BOARD_SIZE);
			local_board[i] = 1;
			generate_queens_seq(local_board, counter + 1, solutions);
		} 
		board[i] = 0;
	}
}

long nano_seconds(struct timespec *t_start, struct timespec *t_stop){
	return (t_stop->tv_nsec - t_start->tv_nsec) +
		(t_stop->tv_sec - t_start->tv_sec)*NANO;
}

//place 8 queens on a chess board in such a way that no one queen can attack another
//
void print_solution(int *solution){
	for(int i = 0; i < BOARD_SIZE; i++){
		if(i % 8 == 0)
			printf("\n");
		printf("%d\t", solution[i]);
	}
	printf("\n");
}

void print_sample_board(){
	int sum = -1;
	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j++){
			printf("%d\t", sum += 1);
		}
		printf("\n\n");
	}
}

int check_all(int pos, int *board){
	int res = 0;
	{
		res = check_row(pos, board);
	}
	if(res == 1)
		return 0;
	{
		res = check_col(pos, board);
	}
	if(res == 1)
		return 0;
	{
		res = check_dgl_top_right(pos, board);
	}
	if(res == 1)
		return 0;
	{
		res = check_dgl_top_left(pos, board);
	}
	if(res == 1)
		return 0;
	{
		res = check_dgl_bottom_right(pos, board);
	}
	if(res == 1)
		return 0;
	{
		res = check_dgl_bottom_left(pos, board);
	}
	if(res == 1)
		return 0;
	return 1;
}

int check_right_wall(int num){
	if(num % 8 == 7)
		return 1;
	return 0;
}

int check_left_wall(int num){
	if(num % 8 == 0)
		return 1;
	return 0;
}

int check_row(int index, int *board){
	int i = ((index - (index % 8)));
	for(; i < ((index - (index % 8))+8); i++){
		if(board[i] == 1)
			return 1;
	}
	return 0;
}
int check_col(int index, int *board){
	for(int i = index % 8; i < 64; i = i + 8){
		if(board[i] == 1)
			return 1;
	}
	return 0;
}
int check_dgl_top_right(int index, int *board){
	for(int i = index; i < BOARD_SIZE; i = i + 9){
		if(board[i] == 1)
			return 1;
		if(check_right_wall(i) == 1)
			break;
	}
	return 0;
}
int check_dgl_top_left(int index, int *board){
	for(int i = index; i < BOARD_SIZE; i = i + 7){
		if(board[i] == 1)
			return 1;
		if(check_left_wall(i) == 1)
			break;
	}
	return 0;
}
int check_dgl_bottom_right(int index, int *board){
	for(int i = index; i >= 0; i = i - 7){
		if(board[i] == 1)
			return 1;
		if(check_right_wall(i) == 1)
			break;
	}
	return 0;
}
int check_dgl_bottom_left(int index, int *board){
	for(int i = index; i >= 0; i = i - 9){
		if(board[i] == 1)
			return 1;
		if(check_left_wall(i) == 1)
			break;
	}
	return 0;
}

int *setup(int *board){
	for(int i = 0; i < BOARD_SIZE; i++){
		board[i] = 0;
	}
}
/*
   void generate_iteratively(){
   for(int a = 0; a <= 7; a++){
   board[a] = 1;
   for(int b = 8; b <= 15; b++){
   if(check_all(b) == 1){
   board[b] = 1;
   for(int c = 16; c <= 23; c++){
   if(check_all(c) == 1){
   board[c] = 1;
   for(int d = 24; d <= 31; d++){
   if(check_all(d) == 1){
   board[d] = 1;
   for(int e = 32; e <= 39; e++){
   if(check_all(e) == 1){
   board[e] = 1;
   for(int f = 40; f <= 47; f++){
   if(check_all(f) == 1){
   board[f] = 1;
   for(int g = 48; g <= 55; g++){
   if(check_all(g) == 1){
   board[g] = 1;
   for(int h = 56; h <= 63; h++){
   if(check_all(h) == 1){
   board[h] = 1;
   solutions_found++;
   } 
   board[h] = 0;
   }
   } 
   board[g] = 0;
   }
   } 
   board[f] = 0;
   }
   } 
   board[e] = 0;
   }
   } 
   board[d] = 0;
   }
   } 
   board[c] = 0;
   }
   } 
   board[b] = 0;
   }
   board[a] = 0;
   }

   }
   */
