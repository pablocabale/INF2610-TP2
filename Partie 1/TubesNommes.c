// Ajouter les directives d'inclusion nécessaires
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
    mkfifo("tube1", 0600);
    mkfifo("tube2", 0600);

    pid_t p3 = fork();
    if (p3 == 0) {
        pid_t p2 = fork();
        if (p2 == 0) {
            pid_t p1 = fork();
            if (p1 == 0) {
                int fd = open("In.txt", O_RDONLY);
                int fdTube1 = open("tube1", O_WRONLY);

                dup2(fd, 0);
                dup2(fdTube1, 1);

                close(fd);
                close(fdTube1);

                execlp("rev", "rev", NULL);
            } 

            int fdTube1 = open("tube1", O_RDONLY);
            int fdTube2 = open("tube2", O_WRONLY);

            dup2(fdTube1, 0);
            dup2(fdTube2, 1);

            close(fdTube1);
            close(fdTube2);

            execlp("rev", "rev", NULL);
        } 

        int fdTube2 = open("tube2", O_RDONLY);

        dup2(fdTube2, 0);  

        close(fdTube2);

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
    } else {
        printf("Erreur avec cmp\n");
    }

    unlink("tube1");
    unlink("tube2");

    return 0;
}
