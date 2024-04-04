/*
 * threadpool.h
 *
 *  Created on: Feb 21, 2024
 *      Author: levon
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <pthread.h>

#define MAX_THREADS 64
#define MAX_QUEUE 65536

typedef struct {
	void (*function)(void*);
	void *arg;
}task_t;

typedef enum {
    threadpool_graceful       = 1
} threadpool_destroy_flags_t;

typedef struct thread_pool_t {
	pthread_mutex_t mtx;
	pthread_cond_t cv;
	pthread_t* thread_buff;
	int tb_count;

	//queue
	task_t* task_buff;
	int task_buff_size;
	int head;
	int tail;

	int pending_count; //pending task count
	int start_thread_count;
	int shutdown; //thread_pool state (shutdown or working)
} thread_pool_t ;

typedef enum {
	POOL_INVALID = -1,
	POOL_LOCK_FAILURE = -2,
	POOL_QUEUE_FULL = -3,
	POOL_THREAD_FAILURE = -4,
	POOL_THREAD_SHUTDOWN = -5
} error_t;

thread_pool_t* init(int thread_count, int queue_size);
int push(thread_pool_t* pool, void (*function)(void*), void* arg);
int destroy(thread_pool_t*, int);
int pool_free(thread_pool_t*);
void* thread_pool_routine(void*);

#endif /* THREADPOOL_H_ */