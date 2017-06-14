#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "omp.h"

#define MAX_SIZE 10000

int QUANT = 1;

void generate_list(int * x, int n) {
   int i,j,t;
   for (i = 0; i < n; i++)
     x[i] = i;
   for (i = 0; i < n; i++) {
     j = rand() % n;
     t = x[i];
     x[i] = x[j];
     x[j] = t;
   }
}

void print_list(int * x, int n) {
   int i;
   for (i = 0; i < n; i++) {
      printf("%d ",x[i]);
   }
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


void mergesortP(int *vector, int start, int end) {
	int middle;

	if (start < end) {
        if( QUANT > 32 ) {
            mergesort(vector, start, end);
            return;
        }

        #pragma omp parallel sections
        {
    		middle = (end + start) / 2;
            #pragma omp section
    		mergesortP(vector, start, middle);

            #pragma omp section
    		mergesortP(vector, middle+1, end);
        }
        merge(vector, start, middle, end);
	}
}



int main()
{
   int n = 1600;
   int num = 2000;
   double start, stop;

   int data[MAX_SIZE];

   generate_list(data, num);
   printf("List Before Sorting...\n");
   print_list(data, n);
   start = omp_get_wtime();
   omp_set_num_threads(4);
   #pragma omp parallel
   {
      #pragma omp master
      {
          mergesortP(data, 0, n-1);
      }
   }
   stop = omp_get_wtime();
   printf("\nList After Sorting...\n");
   print_list(data, n);
   printf("\nTime: %g\n",stop-start);
}
