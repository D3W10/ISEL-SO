#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main () {
    
    for (int i = 0; i < 10; ++i) {
        execlp("date", "date", NULL);
        sleep(1);
    }

    return 0;
}
