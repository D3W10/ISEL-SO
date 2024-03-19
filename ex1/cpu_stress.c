#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void process_work(long niter) {
    for (long i = 0; i < niter; i++) {
        sqrt(rand());
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number_of_processes>\n", argv[0]);
        exit(1);
    }

    for (int i = 0; i < atoi(argv[1]); i++)
    {
        pid_t pId = fork();
        if (pId < 0) {
            perror("fork");
            return -1;
        }
        else if (pId != 0) {
            process_work(1e9);
            return 0;
        }
    }

    wait(12);

    return 0;
}