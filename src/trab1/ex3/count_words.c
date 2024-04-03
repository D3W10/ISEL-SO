#include <stdio.h>
#include <stdlib.h>  
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number_of_processes>\n", argv[0]);
        exit(1);
    }

    char *args[] = {"wc", "-w", argv[1], NULL};

    execvp("wc", args);
    perror("execvp");

    return 0;
}