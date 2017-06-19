#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include "omp.h"

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
    #pragma omp parallel shared(left, counter, vector)
    {
        #pragma omp for schedule(dynamic)
        for (i = start; i <= middle; i++) left[counter++] = vector[i];
    	left[counter] = INT_MAX;

    	counter = 0;
        #pragma omp for schedule(dynamic)
    	for (i = middle+1; i <= end; i++) right[counter++] = vector[i];
    	right[counter] = INT_MAX;
    	// Intercalacao
    	l = r = 0;
        #pragma omp for schedule(dynamic)
    	for (i = start; i <= end; i++) {
    		if (left[l] <= right[r]) {
    			vector[i] = left[l++];
    		} else {
    			vector[i] = right[r++];
    		}
    	}
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

void write_time(FILE *f, int size, double rtime, int num_threads){
    fprintf(f, "\n\nArray of size = %i\n", size);
    if(num_threads == 1)
        fprintf(f, "Mean runtime with %i threadtime = %lf\n", num_threads, rtime);
    else
        fprintf(f, "Mean runtime with %i threadtime = %lf\n", num_threads, rtime);
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

void mergesortP(int *vector, int start, int end, int nivel) {
	int middle;
    
	if (start < end) {
        if( nivel > 5 ) {
            mergesort(vector, start, end);
            return;
        }


        #pragma omp parallel sections
        {
    		middle = (end + start) / 2;
            
            #pragma omp section
    		mergesortP(vector, start, middle, nivel+1);

            #pragma omp section
    		mergesortP(vector, middle+1, end, nivel+1);
            
        }
        #pragma omp parallel sections
        {
            merge(vector, start, middle, end);
        }
	}
}

double runExperiment(int *data, int size, int num_threads, int level){
    double runtime;
    clock_t start, stop;
    
    start = clock(); 
    omp_set_num_threads(num_threads);
    #pragma omp parallel
    {
       #pragma omp master
       {
           mergesortP(data, 0, size-1, level);
       }
    }
    stop = clock(); 

    runtime = ((double)stop)-((double)start);
    
    return runtime;
}

void call_test(int iterations, int size, int *unsorted, int *data, double *times_with_thread, int num_threads){
    double runtime = 0.0;
    int level = 1; //nivel de recursao, usado para garantir que o vetor seja dividido em 32 pedacos
    int k; 

    create_array(unsorted, size); 

    for(k = 0; k < iterations; k++){
        level = 1;
        restore_array(unsorted, data, size); 
        runtime = runtime + runExperiment(data, size, num_threads, level); 
    }

    *times_with_thread = runtime; 
}



int main(int argc, char const **argv) {

    FILE *f = NULL; 
    char *fileName = "output.txt";
    f= fopen(fileName, "w");
    
    double times_with_1_thread = 0.0; 
    double times_with_2_thread = 0.0; 
    double times_with_4_thread = 0.0; 
    double times_with_8_thread = 0.0;  
    double times_with_16_thread = 0.0; 
    double times_with_32_thread = 0.0; 

    
    int num_iterations = 10;
    int size; 
    int num_threads; 
    //vetor de no maximo 1600 posicoes
    int data[MAX_SIZE]; 
    int unsorted_data[MAX_SIZE]; 
    //double runtime = 0.0;

    //test for array of size 200 with 1 thread
    size = 200; 
    num_threads = 1;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_1_thread, num_threads);
    num_threads = 2;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_2_thread, num_threads);
    num_threads = 4;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_4_thread, num_threads);
    num_threads = 8;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_8_thread, num_threads);
    num_threads = 16;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_16_thread, num_threads);
    num_threads = 32;
    call_test( num_iterations, size, unsorted_data, data, &times_with_32_thread, num_threads);

    write_file(unsorted_data, size, f, 0); 
    write_file(data, size, f, 1); 

    times_with_1_thread = (times_with_1_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_2_thread = (times_with_2_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_4_thread = (times_with_4_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_8_thread = (times_with_8_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_16_thread = (times_with_16_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_32_thread = (times_with_32_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;

    write_time(f, size, times_with_1_thread, 1);
    write_time(f, size, times_with_2_thread, 2);
    write_time(f, size, times_with_4_thread, 4);
    write_time(f, size, times_with_8_thread, 8);
    write_time(f, size, times_with_16_thread, 16);
    write_time(f, size, times_with_32_thread, 32);
    
    printf("\n\nArray of size: %d\n", size);
    printf("Mean time with %d: %lf\n", 1 , times_with_1_thread);
    printf("Mean time with %d: %lf\n", 2 , times_with_2_thread);
    printf("Mean time with %d: %lf\n", 4 , times_with_4_thread);
    printf("Mean time with %d: %lf\n", 8 , times_with_8_thread);
    printf("Mean time with %d: %lf\n", 16 , times_with_16_thread);
    printf("Mean time with %d: %lf\n", 32 , times_with_32_thread);

    //test on array of size 400 
    size = 400; 
    times_with_1_thread = 0.0; 
    times_with_2_thread = 0.0; 
    times_with_4_thread = 0.0; 
    times_with_8_thread = 0.0;  
    times_with_16_thread = 0.0; 
    times_with_32_thread = 0.0; 
    num_threads = 1;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_1_thread, num_threads);
    num_threads = 2;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_2_thread, num_threads);
    num_threads = 4;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_4_thread, num_threads);
    num_threads = 8;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_8_thread, num_threads);
    num_threads = 16;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_16_thread, num_threads);
    num_threads = 32;
    call_test( num_iterations, size, unsorted_data, data, &times_with_32_thread, num_threads);

    write_file(unsorted_data, size, f, 0); 
    write_file(data, size, f, 1); 

    times_with_1_thread = (times_with_1_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_2_thread = (times_with_2_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_4_thread = (times_with_4_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_8_thread = (times_with_8_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_16_thread = (times_with_16_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_32_thread = (times_with_32_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;

    write_time(f, size, times_with_1_thread, 1);
    write_time(f, size, times_with_2_thread, 2);
    write_time(f, size, times_with_4_thread, 4);
    write_time(f, size, times_with_8_thread, 8);
    write_time(f, size, times_with_16_thread, 16);
    write_time(f, size, times_with_32_thread, 32);
    
    printf("\n\nArray of size: %d\n", size);
    printf("Mean time with %d: %lf\n", 1 , times_with_1_thread);
    printf("Mean time with %d: %lf\n", 2 , times_with_2_thread);
    printf("Mean time with %d: %lf\n", 4 , times_with_4_thread);
    printf("Mean time with %d: %lf\n", 8 , times_with_8_thread);
    printf("Mean time with %d: %lf\n", 16 , times_with_16_thread);
    printf("Mean time with %d: %lf\n", 32 , times_with_32_thread);
    
    size = 800; 
    num_threads = 1;  
    times_with_1_thread = 0.0; 
    times_with_2_thread = 0.0; 
    times_with_4_thread = 0.0; 
    times_with_8_thread = 0.0;  
    times_with_16_thread = 0.0; 
    times_with_32_thread = 0.0; 
    call_test( num_iterations, size, unsorted_data, data, &times_with_1_thread, num_threads);
    num_threads = 2;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_2_thread, num_threads);
    num_threads = 4;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_4_thread, num_threads);
    num_threads = 8;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_8_thread, num_threads);
    num_threads = 16;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_16_thread, num_threads);
    num_threads = 32;
    call_test( num_iterations, size, unsorted_data, data, &times_with_32_thread, num_threads);
    
    
    write_file(unsorted_data, size, f, 0); 
    write_file(data, size, f, 1); 

    times_with_1_thread = (times_with_1_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_2_thread = (times_with_2_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_4_thread = (times_with_4_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_8_thread = (times_with_8_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_16_thread = (times_with_16_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_32_thread = (times_with_32_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;


    write_time(f, size, times_with_1_thread, 1);
    write_time(f, size, times_with_2_thread, 2);
    write_time(f, size, times_with_4_thread, 4);
    write_time(f, size, times_with_8_thread, 8);
    write_time(f, size, times_with_16_thread, 16);
    write_time(f, size, times_with_32_thread, 32);
    
    printf("\n\nArray of size: %d\n", size);
    printf("Mean time with %d: %lf\n", 1 , times_with_1_thread);
    printf("Mean time with %d: %lf\n", 2 , times_with_2_thread);
    printf("Mean time with %d: %lf\n", 4 , times_with_4_thread);
    printf("Mean time with %d: %lf\n", 8 , times_with_8_thread);
    printf("Mean time with %d: %lf\n", 16 , times_with_16_thread);
    printf("Mean time with %d: %lf\n", 32 , times_with_32_thread);

    size = 1600; 
    num_threads = 1;  
    times_with_1_thread = 0.0; 
    times_with_2_thread = 0.0; 
    times_with_4_thread = 0.0; 
    times_with_8_thread = 0.0;  
    times_with_16_thread = 0.0; 
    times_with_32_thread = 0.0;
    call_test( num_iterations, size, unsorted_data, data, &times_with_1_thread, num_threads);
    num_threads = 2;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_2_thread, num_threads);
    num_threads = 4;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_4_thread, num_threads);
    num_threads = 8;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_8_thread, num_threads);
    num_threads = 16;  
    call_test( num_iterations, size, unsorted_data, data, &times_with_16_thread, num_threads);
    num_threads = 32;
    call_test( num_iterations, size, unsorted_data, data, &times_with_32_thread, num_threads);
    
    
    write_file(unsorted_data, size, f, 0); 
    write_file(data, size, f, 1); 

    times_with_1_thread = (times_with_1_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_2_thread = (times_with_2_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_4_thread = (times_with_4_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_8_thread = (times_with_8_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_16_thread = (times_with_16_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;
    times_with_32_thread = (times_with_32_thread/(double)num_iterations)* 1000.0 / CLOCKS_PER_SEC;

    write_time(f, size, times_with_1_thread, 1);
    write_time(f, size, times_with_2_thread, 2);
    write_time(f, size, times_with_4_thread, 4);
    write_time(f, size, times_with_8_thread, 8);
    write_time(f, size, times_with_16_thread, 16);
    write_time(f, size, times_with_32_thread, 32);
    
    printf("\n\nArray of size: %d\n", size);
    printf("Mean time with %d: %lf\n", 1 , times_with_1_thread);
    printf("Mean time with %d: %lf\n", 2 , times_with_2_thread);
    printf("Mean time with %d: %lf\n", 4 , times_with_4_thread);
    printf("Mean time with %d: %lf\n", 8 , times_with_8_thread);
    printf("Mean time with %d: %lf\n", 16 , times_with_16_thread);
    printf("Mean time with %d: %lf\n", 32 , times_with_32_thread);
    
    return 0;
}
