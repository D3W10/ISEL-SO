#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void process_work(long niter) {
    for (long i = 0; i < niter; i++)
        sqrt(rand());
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number_of_processes>\n", argv[0]);
        return -1;
    }

    int pNum = atoi(argv[1]);

    for (int i = 0; i < pNum; i++) {
        pid_t pId = fork(); // Create child process
        printf("forked %d\n", pId);

        if (pId < 0) { // Error creating child process
            perror("fork");
            return -1;
        }
        else if (pId == 0) { // Code to be executed by the child process
            process_work(1e9);
            return 0;
        }
    }

    for (int i = 0; i < pNum; i++)
        wait(NULL);

    return 0;
}