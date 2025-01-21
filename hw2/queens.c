#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>
#include <omp.h>

#define BOARD_SIZE 64
#define AMOUNT_SOLUTIONS 92

int *solutions[AMOUNT_SOLUTIONS];
int board[BOARD_SIZE];
int solutions_found = 0;
int depth = 0;

int check_row(int);
int check_col(int);
int check_dgl_top_right(int);
int check_dgl_top_left(int);
int check_dgl_bottom_right(int);
int check_dgl_bottom_left(int);
int check_right_wall(int num);
int check_left_wall(int num);
int check_all(int);
void print_sample_board();
void print_solution(int *);
void generate_queens(int *, int);
void setup();

int main(int argc, char *argv[]){
	setup();
	printf("hello from process: %d\n", omp_get_thread_num());
	printf("sample board: \n");
	print_sample_board();
	generate_queens(board, 0);
	/*
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
																print_solution(board);
																printf("solutions found: %d\n", solutions_found);
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
	}*/
	printf("solutions found: %d\n", solutions_found);
	return 0;
}

void generate_queens(int *board, int counter){
	if(counter == 8){
		solutions_found++;
		print_solution(board);
		return;
	}
	for(int i = counter * 8; i <= counter * 8 + 7; i++){
		if(check_all(i) == 1){
			board[i] = 1;
			generate_queens(board, counter + 1);
		} 
		board[i] = 0;
	}
	return;
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

int check_all(int pos){
	if(check_row(pos) == 1)
		return 0;
	if(check_col(pos) == 1)
		return 0;
	if(check_dgl_top_right(pos) == 1)
		return 0;
	if(check_dgl_top_left(pos) == 1)
		return 0;
	if(check_dgl_bottom_right(pos) == 1)
		return 0;
	if(check_dgl_bottom_left(pos) == 1)
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

int check_row(int index){
	int i = ((index - (index % 8)));
	for(; i < ((index - (index % 8))+8); i++){
		if(board[i] == 1)
			return 1;
	}
	return 0;
}
int check_col(int index){
	for(int i = index % 8; i < 64; i = i + 8){
		if(board[i] == 1)
			return 1;
	}
	return 0;
}
int check_dgl_top_right(int index){
	for(int i = index; i < BOARD_SIZE; i = i + 9){
		if(board[i] == 1)
			return 1;
		if(check_right_wall(i) == 1)
			break;
	}
	return 0;
}
int check_dgl_top_left(int index){
	for(int i = index; i < BOARD_SIZE; i = i + 7){
		if(board[i] == 1)
			return 1;
		if(check_left_wall(i) == 1)
			break;
	}
	return 0;
}
int check_dgl_bottom_right(int index){
	for(int i = index; i >= 0; i = i - 7){
		if(board[i] == 1)
			return 1;
		if(check_right_wall(i) == 1)
			break;
	}
	return 0;
}
int check_dgl_bottom_left(int index){
	for(int i = index; i >= 0; i = i - 9){
		if(board[i] == 1)
			return 1;
		if(check_left_wall(i) == 1)
			break;
	}
	return 0;
}

void setup(){
	for(int i = 0; i < BOARD_SIZE; i++){
		board[i] = 0;
	}
}
