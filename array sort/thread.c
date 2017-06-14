#include <stdio.h>
#include <stdlib.h>
#include <omp.h>


int main(int argc, char const **argv) {
    int nthreads, tid;

    //fork a team of threads giving them their own copies of variables
    #pragma omp parallel private(nthreads, tid)
    {
        //obtain thread number
        tid = omp_get_thread_num();
        printf("Hello World from thread = %i\n", tid );

        //only master thread does this
        if(tid == 0)
        {
            nthreads = omp_get_num_threads();
            printf("Number of threads: %d\n", nthreads);
        }
    } //all threads join master thread and disband


    return 0;
}
