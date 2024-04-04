#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define FILENAME "postman.txt"

void handler(int sig, siginfo_t *info, void *context) {
    kill(info->si_pid, SIGILL);
}

int main() {
    srand(time(NULL));
    FILE *fd = fopen(FILENAME, "w");
    if (fd == NULL) {
        perror("Error opening file\n");
        exit(1);
    }

    int file_size = rand() % 900 + 100;
    int spy_pid = rand() % (file_size - 50) + 40;
    for (int i = 0; i < file_size; ++i) {
        if (i == spy_pid) {
            printf("Spy process ID: %d\n", getpid());
            fprintf(fd, "%d ", getpid());
            continue;
        }
        fprintf(fd, "%d ", rand() % 9000 + 1000);
        fflush(fd);
    }
    fclose(fd);

    struct sigaction st;
    st.sa_flags = SA_SIGINFO;
    st.sa_sigaction = handler;
    sigemptyset(&st.sa_mask);

    sigaction(SIGCONT, &st, NULL);
    while (1) {
        pause();
    }

    return 0;
}
