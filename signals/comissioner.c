#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#define FILENAME "postman.txt"


void signal_handler(int sig, siginfo_t *info, void *context) {
    printf("%d Սայուզ նեռուշիմի տավարիշի, Das spionen ist բռնված und պանյատներով նակազատ էղած. Respect+.\n", info->si_pid);
    kill(info->si_pid, SIGKILL);
    exit(EXIT_SUCCESS);
}

int main() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO; 
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask); 

    FILE *fd = fopen(FILENAME, "r");
    if (fd == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    fseek(fd, 0, SEEK_END);
    long sizeoffile = ftell(fd);
    rewind(fd);

    int *pids = malloc((sizeoffile / 4 + 1) * sizeof(int));
    if (pids == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fd);
        return EXIT_FAILURE;
    }

    int count = 0;
    while (fscanf(fd, "%d", &pids[count]) == 1) {
        count++;
    }
    fclose(fd); 

    if (sigaction(SIGILL, &sa, NULL) == -1) {
        perror("Error setting signal handler");
        free(pids);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < count; ++i) {
        kill(pids[i], SIGCONT);
    }

    free(pids); 
    return EXIT_SUCCESS;
}
