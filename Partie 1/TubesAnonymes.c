// Ajouter les directives d'inclusion nécessaires
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
    pid_t p3 = fork();
    if (p3 == 0) {
        int pipe1[2];
        int pipe2[2];

        pipe(pipe1);
        pipe(pipe2);

        pid_t p2 = fork();
        if (p2 == 0) {
            pid_t p3 = fork();
            if (p3 == 0) {
                int fd = open("In.txt", O_RDONLY);

                dup2(fd, 0);
                dup2(pipe1[1], 1);

                close(fd);
                close(pipe1[0]);
                close(pipe1[1]);
                close(pipe2[0]);
                close(pipe2[1]);

                execlp("rev", "rev", NULL);
            } 

            dup2(pipe1[0], 0);
            dup2(pipe2[1], 1);

            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            execlp("rev", "rev", NULL);
        } 

        dup2(pipe2[0], 0);  

        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        execlp("cmp", "cmp", "-", "In.txt", "-s", NULL);
    }

    int status;

    waitpid(p3, &status, 0);

    if(WIFEXITED(status)) {
        int cmp = WEXITSTATUS(status);
        if (cmp == 0) {
            printf("Les fichiers sont identiques\n");
        } else if (cmp == 1) {
            printf("Les fichiers sont différents\n");
        }
    }

    return 0;
}
