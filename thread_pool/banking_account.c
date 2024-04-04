#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_ACCOUNTS 40
#define THREAD_NUM 4
#define MAX_TASKS 120

typedef enum { WITHDRAW, DEPOSIT, SHUTDOWN } TaskType;

typedef struct {
    int user_id;
    double balance;
    pthread_mutex_t lock;
} Account;

typedef struct {
    TaskType type;
    Account *account;
    double amount;
} Task;

Account accounts[NUM_ACCOUNTS];
Task taskQueue[MAX_TASKS];
int taskCount = 0;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queueCond = PTHREAD_COND_INITIALIZER;

void performTask(Task *task) {
    pthread_mutex_lock(&task->account->lock);
    if (task->type == WITHDRAW) {
        task->account->balance -= task->amount;
        printf("Withdraw %.2f from Account ID %d. New Balance: %.2f\n", task->amount, task->account->user_id, task->account->balance);
    } else if (task->type == DEPOSIT) {
        task->account->balance += task->amount;
        printf("Deposit %.2f to Account ID %d. New Balance: %.2f\n", task->amount, task->account->user_id, task->account->balance);
    }
    pthread_mutex_unlock(&task->account->lock);
}

void submitTask(Task t) {
    pthread_mutex_lock(&queueMutex);
    if (taskCount < MAX_TASKS) {
        taskQueue[taskCount++] = t;
        pthread_cond_signal(&queueCond);
    }
    pthread_mutex_unlock(&queueMutex);
}

void *workerThread(void *arg) {
    while (1) {
        Task task;

        pthread_mutex_lock(&queueMutex);
        while (taskCount == 0) {
            pthread_cond_wait(&queueCond, &queueMutex);
        }
        task = taskQueue[0];
        for (int i = 0; i < taskCount - 1; i++) {
            taskQueue[i] = taskQueue[i + 1];
        }
        taskCount--;
        pthread_mutex_unlock(&queueMutex);

        if (task.type == SHUTDOWN) {
            break;
        }

        performTask(&task);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        accounts[i].user_id = i + 1; // Use a simple sequence for user IDs
        accounts[i].balance = 1000.00;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }

    pthread_t threads[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; ++i) {
        pthread_create(&threads[i], NULL, workerThread, NULL);
    }

    // Simulate banking operations
    for (int i = 0; i < MAX_TASKS - 1; i++) {
        Task t = {
            .type = rand() % 2,
            .account = &accounts[rand() % NUM_ACCOUNTS],
            .amount = (rand() % 100) + 1
        };
        submitTask(t);
    }

    // Submit SHUTDOWN tasks to stop worker threads
    for (int i = 0; i < THREAD_NUM; ++i) {
        Task shutdownTask = {.type = SHUTDOWN};
        submitTask(shutdownTask);
    }

    for (int i = 0; i < THREAD_NUM; ++i) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        printf("Final balance - Account ID: %d, Balance: %.2f\n", accounts[i].user_id, accounts[i].balance);
    }

    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&queueCond);
    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        pthread_mutex_destroy(&accounts[i].lock);
    }

    return 0;
}
