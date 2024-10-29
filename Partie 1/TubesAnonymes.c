// Ajouter les directives d'inclusion nécessaires
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
    int pipe1[2]; // Tube entre P1 et P2
    int pipe2[2]; // Tube entre P2 et P3
    
    pipe(pipe1);

    pid_t pid3 = fork();
    if (pid3 == 0) { // P3
        pipe(pipe2);

        if (fork() == 0) { // P2
            if (fork() == 0) { // P1
                int fd = open("In.txt", O_RDONLY);

                dup2(fd, 0);

                close(fd);

                dup2(pipe1[1], 1);

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

    waitpid(pid3, &status, 0);

    if(WIFEXITED(status)) {
        int cmp = WEXITSTATUS(status);
        if (cmp == 0) {
            printf("Les fichiers sont identiques\n");
        } else if (cmp == 1) {
            printf("Les fichiers sont différents\n");
        }
    }

    close(pipe1[0]);
    close(pipe1[1]);

    return 0;
}
