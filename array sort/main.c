#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void create_array(int array[], int size){
    int i;
    for(i = 0; i < size; i++)
        array[i] = rand() % 1600;
}

// int split_array(int size){
//     int newArray[size];
//
//
//     return newArray;
// }

int main(int argc, char **argv){
    time_t now, later; 

    FILE *unsorted_arrays;

    int array200[200], array400[400], array800[800], array1600[1600];
    int i, j;
    int size[4] = {200, 400, 800, 1600};

    unsorted_arrays = fopen("test.txt", "w");

    fprintf(stdout, "Creating array of size 200...");
    create_array(array200, size[0]);
    fprintf(stdout, "Done.\n");

    fprintf(stdout, "Creating array of size 400...");
    create_array(array400, size[1]);
    fprintf(stdout, "Done.\n");

    fprintf(stdout, "Creating array of size 800...");
    create_array(array800, size[2]);
    fprintf(stdout, "Done.\n");

    fprintf(stdout, "Creating array of size 1600...");
    create_array(array1600, size[3]);
    fprintf(stdout, "Done.\n");

    fprintf(unsorted_arrays, "Testing 2...\n");

    //inserting unsorted arrays in txt unsorted_arrays
    for(j = 0; j < 4; j++){
        if(j == 0){
            fprintf(unsorted_arrays, "Array of size %i: [", size[j]);
            for(i = 0; i < size[j]; i++){
                fprintf(unsorted_arrays, "%i ", array200[i]);
            }
            fprintf(unsorted_arrays, "]\n\n");
        }
        if(j == 1){
            fprintf(unsorted_arrays, "Array of size %i: [", size[j]);
            for(i = 0; i < size[j]; i++){
                fprintf(unsorted_arrays, "%i ", array400[i]);
            }
            fprintf(unsorted_arrays, "]\n\n");
        }
        if(j == 2){
            fprintf(unsorted_arrays, "Array of size %i: [", size[j]);
            for(i = 0; i < size[j]; i++){
                fprintf(unsorted_arrays, "%i ", array800[i]);
            }
            fprintf(unsorted_arrays, "]\n\n");
        }
        if(j == 3){
            fprintf(unsorted_arrays, "Array of size %i: [", size[j]);
            for(i = 0; i < size[j]; i++){
                fprintf(unsorted_arrays, "%i ", array1600[i]);
            }
            fprintf(unsorted_arrays, "]\n\n");
        }
    }




    fclose(unsorted_arrays);
    return 0;
}
