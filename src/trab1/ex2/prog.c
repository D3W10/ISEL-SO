#include <stdio.h>
#include <unistd.h>

int main() {
    printf("MSG 1\n");

    for (int i = 0; i < 3; i++) {
        if (fork() == 0)
            printf("MSG 2\n");
    }
    
    printf("MSG 3\n");

    return 0;
}