//run       mpiexec -np 5 ./merge-test 1600
//compile   mpiexcc merge-test.c -o merge-test

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
// #include "omp.h"
#include <mpi.h>

#define MAX_SIZE 1600

// vetor de entrada que deve ser particionado em 32 pedaços (independente do tamanho e do numero de threads)
//testar com vetores de tamanho = 200, 400, 800 e 1600
//testar usando 1,2,4,8,16 e 32 threads

void create_array(int *array, int size){
    int i, j, temp; 
    //preenche o vetor com valores de 0 ate size
    for(i = 0; i < size; i++)
        array[i] = i; 
    //atribui valores aleatorios ao vetor
    for(i = 0; i < size; i++){
        j = rand() % size; 
        temp = array[i]; 
        array[i] = array[j]; 
        array[j] = temp;
    }    
}

void show_array(int *array, int size) {
   int i;
   for (i = 0; i < size; i++) {
      printf("%d ",array[i]);
   }
}

void restore_array(int *unsorted_data, int *data, int size){
    int i; 
    for(i = 0; i < size; i++)
        data[i] = unsorted_data[i];
}

void merge(int *vector, int start, int middle, int end) {
	int *left, *right;
	int nleft, nright;
	int counter, l, r, i;

	nleft = middle - start + 2;
	nright = end - middle + 1;

	left = (int *) malloc(sizeof(int) * nleft);
	right = (int *) malloc(sizeof(int) * nright);

	counter = 0;
	for (i = start; i <= middle; i++) left[counter++] = vector[i];
	left[counter] = INT_MAX;

	counter = 0;
	for (i = middle+1; i <= end; i++) right[counter++] = vector[i];
	right[counter] = INT_MAX;

	// Intercalacao
	l = r = 0;
	for (i = start; i <= end; i++) {
		if (left[l] <= right[r]) {
			vector[i] = left[l++];
		} else {
			vector[i] = right[r++];
		}
	}
}

void mergesort(int *vector, int start, int end) {
	int middle;

	if (start < end) {
		middle = (end + start) / 2;
		mergesort(vector, start, middle);
		mergesort(vector, middle+1, end);
		merge(vector, start, middle, end);
	}
}

void write_file(int *array, int size, FILE *f, int sorted){
    int i; 
    if(sorted == 1) {
        fprintf(f, "Sorted array of size %i: \n[", size);
        for(i = 0; i < size; i++){
            fprintf(f, "%i ", array[i]);
        }
        fprintf(f, "]\n\n");
    }
    if(sorted == 0){
        fprintf(f, "Unsorted array of size %i: \n[", size);
        for(i = 0; i < size; i++){
            fprintf(f, "%i ", array[i]);
        }
        fprintf(f, "]\n\n");
    } 
}

void write_time(FILE *f, int size, double rtime){
    fprintf(f, "\n\nArray of size = %i\n", size);
    fprintf(f, "Mean runtime = %lf\n", rtime);
}

double runExperiment(int *data, int size){
    double runtime;
    clock_t start, stop;
     

    runtime = ((double)stop)-((double)start);

    return runtime;
}

void call_test(int iterations, int size, int *unsorted, int *data, double *times){
    double runtime = 0.0;
    int k; 

    create_array(unsorted, size); 

    for(k = 0; k < iterations; k++){
        restore_array(unsorted, data, size); 
        runtime = runtime + runExperiment(data, size); 
    }

    *times = runtime; 
}



int main(int argc, char const **argv) {

    FILE *f = NULL; 
    char *fileName = "output.txt";
    f= fopen(fileName, "w");
    
    double times= 0.0;
    clock_t start, stop;
    int num_iterations = 10;
    int size = atoi(argv[1]); 
    //vetor de no maximo 1600 posicoes
    int data[MAX_SIZE]; 
    int unsorted_data[MAX_SIZE]; 
    int i;
    double runtime = 0.0;
    int k; 
    create_array(unsorted_data, size);
    restore_array(unsorted_data, data, size); 
    printf("Vector size = %d\n", size);
    /********** Initialize MPI **********/
	int world_rank;
	int world_size;
	
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
		
	/********** Divide the array in equal-sized chunks **********/
	int s = size/32;
    start = clock(); 

	/********** Send each subarray to each process **********/
	int *sub_array = malloc(s * sizeof(int));
	MPI_Scatter(data, s, MPI_INT, sub_array, s, MPI_INT, 0, MPI_COMM_WORLD);
	
	/********** Perform the mergesort on each process **********/
	mergesort(sub_array, 0, (s - 1));
	
    /********** Gather the sorted subarrays into one **********/    
    MPI_Gather(sub_array, s, MPI_INT, data, s, MPI_INT, 0, MPI_COMM_WORLD);

    /********** Make the final mergeSort call **********/
	if(world_rank == 0) {		
		int *other_array = malloc(size * sizeof(int));
		mergesort(data, 0, (size - 1));
		
		/********** Display the sorted array **********/
		// printf("\nThis is the sorted array: ");
		// for(i = 0; i < size; i++) {
		// 	printf("%d ", data[i]);	
		// }
			
		// printf("\n");
		// printf("\n");
		stop = clock();
		/********** Clean up root **********/
        times = ((double)stop)-((double)start);
        times = (times)* 1000.0 / CLOCKS_PER_SEC;
        printf("\nThis is the processing time: %lf", times);
        write_time(f, size, times);
        write_file(unsorted_data, size, f, 0); 
        write_file(data, size, f, 1);
		// free(sorted);
		free(other_array);
	}
   
    /********** Finalize MPI **********/
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
   
    //double runtime = 0.0;

    // //test for array of size 200 with 1 thread
    // size = 200; 
    // call_test( num_iterations, size, unsorted_data, data, &times);

    // write_file(unsorted_data, size, f, 0); 
    // write_file(data, size, f, 1); 

    // times = (times/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;

    // write_time(f, size, times);
    
    // printf("\n\nArray of size: %d\n", size);
    // printf("Mean time: %lf\n",times);

    // //test on array of size 400 
    // size = 400; 
    // times = 0.0; 
     
    // call_test( num_iterations, size, unsorted_data, data, &times);

    // write_file(unsorted_data, size, f, 0); 
    // write_file(data, size, f, 1); 

    // times = (times/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;

    // write_time(f, size, times);
    
    // printf("\n\nArray of size: %d\n", size);
    // printf("Mean time: %lf\n",times);
    
    // size = 800; 
    // times = 0.0; 
     
    // call_test( num_iterations, size, unsorted_data, data, &times);

    // write_file(unsorted_data, size, f, 0); 
    // write_file(data, size, f, 1); 

    // times = (times/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;

    // write_time(f, size, times);
    
    // printf("\n\nArray of size: %d\n", size);
    // printf("Mean time: %lf\n",times);

    // size = 1600; 
    // times = 0.0; 
     
    // call_test( num_iterations, size, unsorted_data, data, &times);

    // write_file(unsorted_data, size, f, 0); 
    // write_file(data, size, f, 1); 

    // times = (times/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;

    // write_time(f, size, times);
    
    // printf("\n\nArray of size: %d\n", size);
    // printf("Mean time: %lf\n",times);
    
    return 0;
}
