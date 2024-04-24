#include <stdio.h>
#include <stdlib.h>  
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file_name>\n", argv[0]);
        exit(1);
    }

    pid_t retfork = fork();

    if (retfork < 0) {
        perror("fork");
        exit(1);
    }
    else if(retfork == 0) {
        char *args[] = {"wc", "-w", argv[1], NULL};

        execvp("wc", args);
        perror("execvp");
    }

    wait(NULL);

    return 0;
}