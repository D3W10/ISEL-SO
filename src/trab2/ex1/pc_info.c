#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        execl("/usr/bin/lscpu", "lscpu", NULL);
    }
    else {
        close(fd[1]);

        char buffer[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0) {
            for (int i = 0; i < bytes_read; i++)
                putchar(toupper(buffer[i]));
        }

        wait(pid);
    }

    return 0;
}