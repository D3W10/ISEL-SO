#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

int vector_get_in_range(int v[], int v_sz, int sv[], int min, int max, int n_processes);

int main(){
    struct timeval t1, t2; 
    gettimeofday(&t1, NULL); 
 
    // code to evaluate 
 
    gettimeofday(&t2, NULL); 
    long elapsed = ((long)t2.tv_sec - t1.tv_sec) * 1000000L + (t2.tv_usec - t1.tv_usec); 
    printf ("Elapsed time = %6li us\n", elapsed); 

    return 0;
}

int vector_get_in_range(int v[], int v_sz, int sv[], int min, int max, int n_processes){




    return ;
}
