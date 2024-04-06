#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#define FIFO_NAME "FifoFile"
#define TEXT_SIZE 1000

volatile sig_atomic_t next_message_ready = 0;

// Обработчик сигнала SIGCONT, указывающий, что можно читать следующее сообщение
void handle_next_message_signal(int i) {
    next_message_ready = 1;
}

// Функция, отвечающая за запись сообщений в FIFO
void run_writer_process() {
    int fifo_fd = open(FIFO_NAME, O_WRONLY);
    if (fifo_fd == -1) {
        perror("Ошибка открытия FIFO");
        exit(EXIT_FAILURE);
    }
    printf("Процесс записи готов! Введите 'quit' для выхода.\n");

    while (1) {
        char line[TEXT_SIZE + 1];
        printf("Введите сообщение: ");
        if (fgets(line, TEXT_SIZE, stdin) == NULL) {
            perror("Ошибка чтения ввода");
            break;
        }

        if (write(fifo_fd, line, TEXT_SIZE) < 0) {
            perror("Ошибка записи в FIFO");
            exit(EXIT_FAILURE);
        }

        // Удаление символа новой строки из ввода
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, "quit") == 0) {
            break;
        }

        // Ожидание сигнала от читателя перед продолжением
        while (!next_message_ready)
            signal(SIGCONT, handle_next_message_signal);
        next_message_ready = 0;
    }
    close(fifo_fd);
}

// Функция, отвечающая за чтение сообщений из FIFO
void run_reader_process(pid_t writer_pid) {
    int fifo_fd = open(FIFO_NAME, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Ошибка открытия FIFO");
        exit(EXIT_FAILURE);
    }

    while (1) {
        char buffer[TEXT_SIZE + 1];
        if (read(fifo_fd, buffer, TEXT_SIZE) == -1) {
            perror("Ошибка чтения из FIFO");
            exit(EXIT_FAILURE);
        }

        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(buffer, "quit") == 0) {
            printf("Завершение работы.\n");
            break;
        }

        printf("Получено сообщение: %s\n", buffer);
        kill(writer_pid, SIGCONT);
    }
    close(fifo_fd);
}

int main() {
    // Создание FIFO, если оно не существует
    if (mkfifo(FIFO_NAME, 0666) != 0 && errno != EEXIST) {
        perror("Не удалось создать FIFO");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Ошибка при создании процесса");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Дочерний процесс запускает записывающий процесс
        run_writer_process();
    } else {
        // Родительский процесс запускает читающий процесс
        run_reader_process(pid);
        wait(NULL); // Ожидание завершения дочернего процесса
    }

    unlink(FIFO_NAME); // Удаление файла FIFO
}
