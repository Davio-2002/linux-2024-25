#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1

typedef struct {
    FILE *stream;
    pid_t pid;
} PopenFile;


static void handle_child_process(const char *command, const char *type, int pipe_fds[2]) {
    if (strcmp(type, "r") == 0) {
        close(pipe_fds[READ_END]);
        dup2(pipe_fds[WRITE_END], STDOUT_FILENO);
        close(pipe_fds[WRITE_END]);
    } else if (strcmp(type, "w") == 0) {
        close(pipe_fds[WRITE_END]);
        dup2(pipe_fds[READ_END], STDIN_FILENO);
        close(pipe_fds[READ_END]);
    } else {
        fprintf(stderr, "Invalid type\n");
        exit(EXIT_FAILURE);
    }
    execl("/bin/sh", "sh", "-c", command, (char *)NULL);
    perror("execl");
    exit(EXIT_FAILURE);
}

static PopenFile* create_pipe_and_fork(const char *command, const char *type) {
    int pipe_fds[2];
    pid_t pid;
    FILE *stream;

    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return NULL;
    }

    if ((pid = fork()) == -1) {
        perror("fork");
        close(pipe_fds[READ_END]);
        close(pipe_fds[WRITE_END]);
        return NULL;
    }

    if (pid == 0) {
        handle_child_process(command, type, pipe_fds);
    }

    if (strcmp(type, "r") == 0) {
        close(pipe_fds[WRITE_END]);
        stream = fdopen(pipe_fds[READ_END], "r");
    } else if (strcmp(type, "w") == 0) {
        close(pipe_fds[READ_END]);
        stream = fdopen(pipe_fds[WRITE_END], "w");
    } else {
        close(pipe_fds[READ_END]);
        close(pipe_fds[WRITE_END]);
        return NULL;
    }

    if (stream == NULL) {
        close(pipe_fds[READ_END]);
        close(pipe_fds[WRITE_END]);
        return NULL;
    }

    PopenFile *popen_file = malloc(sizeof(PopenFile));
    if (popen_file == NULL) {
        fclose(stream);
        return NULL;
    }

    popen_file->stream = stream;
    popen_file->pid = pid;

    return popen_file;
}

static int close_pipe_and_wait(PopenFile *popen_file) {
    int status;

    if (popen_file == NULL) {
        errno = EINVAL;
        return -1;
    }

    fclose(popen_file->stream);
    pid_t pid = waitpid(popen_file->pid, &status, 0);

    free(popen_file);

    if (pid == -1) {
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return -1;
}

PopenFile* my_popen(const char *command, const char *type) {
    return create_pipe_and_fork(command, type);
}

int my_pclose(PopenFile *popen_file) {
    return close_pipe_and_wait(popen_file);
}

int main() {
    char buffer[128];

    PopenFile* pf = my_popen("ls", "r");
    if (pf == NULL) {
        perror("my_popen");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), pf->stream) != NULL) {
        printf("%s", buffer);
    }

    const int status = my_pclose(pf);
    printf("Process exited with status %d\n", status);

    return 0;
}

