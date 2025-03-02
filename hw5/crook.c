/**
 * Communicator: MPI_COMM_WORLD;
 * is used since the scope of the assignment includes only program specific processes
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "mpi.h"

#define QUERY_TAG 10
#define RESPONSE_TAG 20 

int* generate_sorted_array(int);

int main(int argc, char *argv[]){
	/*
	 * Potentially do a star and ring solution?
	 * With ANY_SOURCE and ANY_TAG i can explicitly check from where im getting something...
	 * use Iprobe or probe
	 *
	 */
	int n = 12;
	int testArray[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	int testArray2[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
	int done = 0, myid, numprocs;
	int* rank0_array = calloc(n, sizeof(int));
	int* rank1_array = calloc(n, sizeof(int));
	int* rank2_array = calloc(n, sizeof(int));
	int count = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	if(myid == 0){
		int arr1[] ={
			1,
			4,
			7,
			10,
			13,
			16,
			19,
			23,
			26,
			28,
			31,
			36
		};
		int arr2[] ={
			1,
			4,
			8,
			11,
			14,
			17,
			20,
			23,
			26,
			29,
			33,
			36
		};
		int arr3[] ={
			3,
			4,
			9,
			12,
			15,
			18,
			20,
			23,
			27,
			30,
			33,
			36
		};
		for(int i = 0; i < n; i++){
			rank0_array[i] = arr1[i];
			rank1_array[i] = arr2[i];
			rank2_array[i] = arr3[i];
		}
		printf("Rank 0's array: \n");
		for(int i = 0; i < n; i++){
			printf("%d,\t", rank0_array[i]);
		}
		printf("\nRank 1's array: \n");
		for(int i = 0; i < n; i++){
			printf("%d,\t", rank1_array[i]);
		}
		printf("\nRank 2's array: \n");
		for(int i = 0; i < n; i++){
			printf("%d,\t", rank2_array[i]);
		}
	}
	MPI_Bcast(rank0_array, n, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Bcast(rank1_array, n, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Bcast(rank2_array, n, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	int a = 0, b = 0, c = 0;
	for(int i = 0; i < n; i++){
		if(myid == 0){
			int received_query_rank_1;
			int received_query_rank_2;
			int received_response_rank_1;
			int received_response_rank_2;

			MPI_Status status_array[2];
			MPI_Request request_array[2];

			//Sends non blocking queries to the other two processes 
			MPI_Isend(&rank0_array[i], 1, MPI_UNSIGNED, 1, QUERY_TAG, MPI_COMM_WORLD, &request_array[0]);
			MPI_Isend(&rank0_array[i], 1, MPI_UNSIGNED, 2, QUERY_TAG, MPI_COMM_WORLD, &request_array[1]);
			//Blocking receive from the other two processes
			MPI_Recv(&received_response_rank_1, 1, MPI_UNSIGNED, 1, QUERY_TAG, MPI_COMM_WORLD, &status_array[0]);
			MPI_Recv(&received_response_rank_2, 1, MPI_UNSIGNED, 2, QUERY_TAG, MPI_COMM_WORLD, &status_array[1]);
			int found = 0;
			for(int j = 0; j < n; j++){
				if(received_response_rank_1 == rank0_array[j]){
					found = 1;
				}
			}
			MPI_Send(&found, 1, MPI_UNSIGNED, 1, RESPONSE_TAG, MPI_COMM_WORLD);
			found = 0;
			for(int j = 0; j < n; j++){
				if(received_response_rank_2 == rank0_array[j]){
					found = 1;
				}
			}
			MPI_Wait(&request_array[0], MPI_STATUS_IGNORE);
			MPI_Wait(&request_array[1], MPI_STATUS_IGNORE);
			//Blocking send to each process
			MPI_Send(&found, 1, MPI_UNSIGNED, 2, RESPONSE_TAG, MPI_COMM_WORLD);
			//blocking receive to check if the response was correct
			MPI_Recv(&received_response_rank_1, 1, MPI_UNSIGNED, 1, RESPONSE_TAG, MPI_COMM_WORLD, &status_array[0]);
			MPI_Recv(&received_response_rank_2, 1, MPI_UNSIGNED, 2, RESPONSE_TAG, MPI_COMM_WORLD, &status_array[1]);
			if(received_response_rank_1 == 1 && received_response_rank_2 == 1){
				printf("\n rank %d found a triple with value: %d in rank 1 and 2!\n", myid, rank1_array[i]);
			}
		}
		if(myid == 1){
			int received_query_rank_0;
			int received_query_rank_2;
			int received_response_rank_0;
			int received_response_rank_2;

			MPI_Status status_array[2];
			MPI_Request request_array[2];

			MPI_Isend(&rank1_array[i], 1, MPI_UNSIGNED, 0, QUERY_TAG, MPI_COMM_WORLD, &request_array[0]);
			MPI_Isend(&rank1_array[i], 1, MPI_UNSIGNED, 2, QUERY_TAG, MPI_COMM_WORLD, &request_array[1]);
			MPI_Recv(&received_response_rank_0, 1, MPI_UNSIGNED, 0, QUERY_TAG, MPI_COMM_WORLD, &status_array[0]);
			MPI_Recv(&received_response_rank_2, 1, MPI_UNSIGNED, 2, QUERY_TAG, MPI_COMM_WORLD, &status_array[1]);
			int found = 0;
			for(int j = 0; j < n; j++){
				if(received_response_rank_0 == rank1_array[j]){
					found = 1;
				}
			}
			MPI_Send(&found, 1, MPI_UNSIGNED, 2, RESPONSE_TAG, MPI_COMM_WORLD);
			found = 0;
			for(int j = 0; j < n; j++){
				if(received_response_rank_2 == rank1_array[j]){
					found = 1;
				}
			}
			MPI_Send(&found, 1, MPI_UNSIGNED, 0, RESPONSE_TAG, MPI_COMM_WORLD);
			MPI_Wait(&request_array[0], MPI_STATUS_IGNORE);
			MPI_Wait(&request_array[1], MPI_STATUS_IGNORE);
			MPI_Recv(&received_response_rank_0, 1, MPI_UNSIGNED, 0, RESPONSE_TAG, MPI_COMM_WORLD, &status_array[0]);
			MPI_Recv(&received_response_rank_2, 1, MPI_UNSIGNED, 2, RESPONSE_TAG, MPI_COMM_WORLD, &status_array[1]);
			if(received_response_rank_0 == 1 && received_response_rank_2 == 1){
				printf("\n rank %d found a triple of value: %d in rank 1 and 2!\n", myid, rank1_array[i]);
			}


		}
		if(myid == 2){
			int received_query_rank_0;
			int received_query_rank_1;
			int received_response_rank_0;
			int received_response_rank_1;

			MPI_Status status_array[2];
			MPI_Request request_array[2];

			MPI_Isend(&rank2_array[i], 1, MPI_UNSIGNED, 0, QUERY_TAG, MPI_COMM_WORLD, &request_array[0]);
			MPI_Isend(&rank2_array[i], 1, MPI_UNSIGNED, 1, QUERY_TAG, MPI_COMM_WORLD, &request_array[1]);
			MPI_Recv(&received_response_rank_0, 1, MPI_UNSIGNED, 0, QUERY_TAG, MPI_COMM_WORLD, &status_array[0]);
			MPI_Recv(&received_response_rank_1, 1, MPI_UNSIGNED, 1, QUERY_TAG, MPI_COMM_WORLD, &status_array[1]);
			int found = 0;
			for(int j = 0; j < n; j++){
				if(received_response_rank_0 == rank2_array[j]){
					found = 1;
				}
			}
			MPI_Send(&found, 1, MPI_UNSIGNED, 0, RESPONSE_TAG, MPI_COMM_WORLD);
			found = 0;
			for(int j = 0; j < n; j++){
				if(received_response_rank_1 == rank2_array[j]){
					found = 1;
				}
			}
			MPI_Send(&found, 1, MPI_UNSIGNED, 1, RESPONSE_TAG, MPI_COMM_WORLD);
			MPI_Wait(&request_array[0], MPI_STATUS_IGNORE);
			MPI_Wait(&request_array[1], MPI_STATUS_IGNORE);

			MPI_Recv(&received_response_rank_0, 1, MPI_UNSIGNED, 0, RESPONSE_TAG, MPI_COMM_WORLD, &status_array[0]);
			MPI_Recv(&received_response_rank_1, 1, MPI_UNSIGNED, 1, RESPONSE_TAG, MPI_COMM_WORLD, &status_array[1]);
			if(received_response_rank_0 == 1 && received_response_rank_1 == 1){
				printf("\n rank %d found a triple of value: %d in rank 1 and 2!\n", myid, rank2_array[i]);
			}
		}
	}
	MPI_Finalize();

	return 0;
}

int *generate_sorted_array(int size){
	int *sorted = (int*)calloc(size, sizeof(int));
	int nxt = 0;
	for(int i = 0; i < size; i++){
		nxt += random()%10+1;
		sorted[i] = nxt;
	}
	return sorted;
}	/*
	   while(!done || !(a >= n && b >= n && c >= n)) {
	   if(myid == 0){
	   int received_data;
	   int flag;
	   MPI_Status status;
	   MPI_Request request;
	   MPI_Irecv(&received_data, 1, MPI_UNSIGNED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
//MPI_Test(&request, &flag, &status);
MPI_Wait(&request, &status);
//if(flag){
if(status.MPI_TAG == QUERY_TAG){
int temp_true;
if(rank1_array[received_data] > 0){
temp_true = received_data;
MPI_Send(&temp_true, 1, MPI_UNSIGNED, status.MPI_SOURCE, RESPONSE_TAG, MPI_COMM_WORLD);
} 
temp_true = -1;
MPI_Send(&temp_true, 1, MPI_UNSIGNED, status.MPI_SOURCE, RESPONSE_TAG, MPI_COMM_WORLD);
} else if(status.MPI_TAG == RESPONSE_TAG){
if(received_data >= 0){
found_tracker_rank0[received_data]++;
if(found_tracker_rank0[received_data] == 2){
printf("Rank %d found %d in two other lists!\n", myid, received_data);
}
}
}
//} else {
if(rank0_array[a] > 0 && a < n){
MPI_Send(&a, 1, MPI_UNSIGNED, 1, QUERY_TAG, MPI_COMM_WORLD);  
MPI_Send(&a, 1, MPI_UNSIGNED, 2, QUERY_TAG, MPI_COMM_WORLD); 
a++;
} else if(a < n) {
a++;
continue;
} else
break;
//}
} else if(myid == 1){
int received_data;
int flag;
MPI_Status status;
MPI_Request request;
MPI_Irecv(&received_data, 1, MPI_UNSIGNED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
//MPI_Test(&request, &flag, &status);
MPI_Wait(&request, &status);
//if(flag){
if(status.MPI_TAG == QUERY_TAG){
int temp_true;
//check if it has the value
if(rank1_array[received_data] > 0){
temp_true = received_data;
MPI_Send(&temp_true, 1, MPI_UNSIGNED, status.MPI_SOURCE, RESPONSE_TAG, MPI_COMM_WORLD);
} 
temp_true = -1;
MPI_Send(&temp_true, 1, MPI_UNSIGNED, status.MPI_SOURCE, RESPONSE_TAG, MPI_COMM_WORLD);
} else if(status.MPI_TAG == RESPONSE_TAG){
if(received_data >= 0){
found_tracker_rank1[received_data]++;
if(found_tracker_rank1[received_data] == 2){
printf("Rank %d found %d in two other lists!\n", myid, received_data);
}
}
}
//} else {
if(rank1_array[b] > 0 && b < n){
MPI_Send(&b, 1, MPI_UNSIGNED, 0, QUERY_TAG, MPI_COMM_WORLD);  
MPI_Send(&b, 1, MPI_UNSIGNED, 2, QUERY_TAG, MPI_COMM_WORLD);  
b++;
} else if(b < n){
b++;
continue;
} else
break;
//}
} else if(myid == 2){
	int received_data;
	int flag;
	MPI_Status status;
	MPI_Request request;
	MPI_Irecv(&received_data, 1, MPI_UNSIGNED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
	//MPI_Test(&request, &flag, &status);
	MPI_Wait(&request, &status);
	//if(flag){
	if(status.MPI_TAG == QUERY_TAG){
		int temp_true;
		if(rank2_array[received_data] > 0){
			temp_true = received_data;
			MPI_Send(&temp_true, 1, MPI_UNSIGNED, status.MPI_SOURCE, RESPONSE_TAG, MPI_COMM_WORLD);
		} 
		temp_true = -1;
		MPI_Send(&temp_true, 1, MPI_UNSIGNED, status.MPI_SOURCE, RESPONSE_TAG, MPI_COMM_WORLD);
	} else if(status.MPI_TAG == RESPONSE_TAG) {
		if(received_data >= 0){
			found_tracker_rank2[received_data]++;
			if(found_tracker_rank2[received_data] == 2){
				printf("Rank %d found %d in two other lists!\n", myid, received_data);
			}
		}
	}
	//} else {
	if(rank1_array[c] > 0 && c < n){
		MPI_Send(&c, 1, MPI_UNSIGNED, 0, QUERY_TAG, MPI_COMM_WORLD);  
		MPI_Send(&c, 1, MPI_UNSIGNED, 1, QUERY_TAG, MPI_COMM_WORLD);  
		c++;
	} else if(c <= n){
		c++;
		continue;
	} else
		break;
}
//} 
}*/

