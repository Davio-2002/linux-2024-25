/*
 * threadpool.c
 *
 *  Created on: Feb 21, 2024
 *      Author: levon
 */

#include "thread_pool.h"
#include <stdlib.h>

typedef enum {
	immediate_shutdown = 1,
	graceful_shutdown = 2
}threadpool_shutdown_t;

thread_pool_t* init(int thread_count, int queue_size)
{
	thread_pool_t* pool = malloc(sizeof(thread_pool_t));

	// Usual checks for incorrect input values
	if(thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE) {
		return NULL;
	}

	// If pool didn't manage to allocate, jump to the end
	if(pool == NULL) {
		goto err;
	}

	// Initializing
	pool->tb_count = 0;
	pool->task_buff_size = queue_size;
	pool->head = 0;
	pool->tail = 0;
	pool->shutdown = 0;
	pool->start_thread_count = 0;
	pool->pending_count = 0;

	pool->thread_buff = malloc(sizeof(pthread_t) * thread_count);
	pool->task_buff = malloc(sizeof(task_t) * queue_size);

	// Mutex/Cond variable initializing
    if((pthread_mutex_init(&(pool->mtx), NULL) != 0) ||
       (pthread_cond_init(&(pool->cv), NULL) != 0) ||
       (pool->thread_buff == NULL) ||
       (pool->task_buff == NULL)) {
        goto err;
    }

    // Thread creation
	for(int i = 0; i < thread_count; ++i) {
		// If any of the thread creations returned with error, we destroy the pool
        if(pthread_create(&(pool->thread_buff[i]), NULL,
        		thread_pool_routine, (void*)pool) != 0) {
            destroy(pool, 0);
            return NULL;
        }

        // If thread is created, started threads count and actual thread count of the pool are incremented
		++pool->start_thread_count;
		++pool->tb_count;
	}

	return pool;

	// error handling's here
err:
	// if pool allocated successfuly, but there's any error during initialization, memory must be freed
	if(pool) {
		pool_free(pool);
	}
	return NULL;
}

int push(thread_pool_t* pool, void (*function)(void*), void* arg)
{
	int error = 0;

	// pool/function is NULL, therefore there is nowhere/nothing to push, exiting
	if (pool == NULL || function == NULL)
	{
		return POOL_INVALID;
	}
	if (pthread_mutex_lock(&pool->mtx))
		return POOL_LOCK_FAILURE;

	do {
		// Task queue is full? Okay, so we're not continuing to avoid buff overload
		if (pool->pending_count == pool->task_buff_size) {
			error = POOL_QUEUE_FULL;
			break;
		}

		// If pool's shutting down, there is no purpose to push tasks any further
		if (pool->shutdown) {
			error = POOL_THREAD_SHUTDOWN;
			break;
		}

		// Push function and it's argument to the tail of the queue, increment pending count
		pool->task_buff[pool->tail].function = function;
		pool->task_buff[pool->tail].arg = arg;
		pool->tail = (pool->tail + 1) % pool->task_buff_size;;
		++pool->pending_count;
		if (pthread_cond_signal(&pool->cv) != 0)
		{
			error = POOL_LOCK_FAILURE;
			break;
		}

	} while(0);

	pthread_mutex_unlock(&pool->mtx);
	return error;
}

int destroy(thread_pool_t* pool, int flag)
{
	int error = 0;

	// if pool is not allocated, there is nothing to destroy
	if(pool == NULL) {
		return POOL_INVALID;
	}

	// thread mutex lock check
	if(pthread_mutex_lock(&(pool->mtx)) != 0) {
			return POOL_LOCK_FAILURE;
	}
	do {
		// If we are already shutting down, there is no purpose to continue the actions
		if (pool->shutdown) {
			error = POOL_THREAD_SHUTDOWN;
			break;
		}

		pool->shutdown = (flag & threadpool_graceful) ?
		            graceful_shutdown : immediate_shutdown;
		// condition signal check
		if (pthread_cond_broadcast(&pool->cv) != 0)
		{
			error = POOL_LOCK_FAILURE;
			break;
		}

		pthread_mutex_unlock(&pool->mtx);
		for(int i = 0; i < pool->tb_count; i++) {
			// If join is not successful, break from loop
			if (pthread_join(pool->thread_buff[i], NULL) != 0)
			{
				error = POOL_THREAD_FAILURE;
			}
		}
	} while(0);

	// If we managed to broadcast and join all the threads successfully, only then we deallocate the pool
	if (!error)
		free(pool);

	return error;
}

int pool_free(thread_pool_t* pool)
{
	// If pool is not allocated or there are remaining started threads, pool is not eligible for deallocation
	if (pool == NULL || pool->start_thread_count > 0)
		return -1;

	// Deallocation only if we managed to allocate thread array
	if (pool->thread_buff)
	{
		free(pool->thread_buff);
		free(pool->task_buff);
		pthread_mutex_destroy(&pool->mtx);
		pthread_cond_destroy(&pool->cv);
	}
	free(pool);
	return 0;
}

void* thread_pool_routine(void* pool_ptr)
{
	// Casting our void* argument of the routine function to thread_pool_t* so we can work with it from now on
	thread_pool_t* pool = (thread_pool_t*)(pool_ptr);
	task_t task;
	while(1) {
		pthread_mutex_lock(&pool->mtx);
		while(pool->pending_count == 0 && !pool->shutdown) {
			pthread_cond_wait(&pool->cv, &pool->mtx);
		}

		if (pool->shutdown == immediate_shutdown
		 || pool->shutdown == graceful_shutdown) {
			break;
		}

		// Grabbing the task from queue head
		task.function = pool->task_buff[pool->head].function;
		task.arg = pool->task_buff[pool->head].arg;
        pool->head = (pool->head + 1) % pool->task_buff_size;
        pool->pending_count -= 1;

		pthread_mutex_unlock(&pool->mtx);
		(*(task.function))(task.arg);
	}
	--pool->start_thread_count;

	// I definitely can say that you'll ask yourself: why there is an additional pthread_mutex_unlock
	// outside of the loop. Point is that while(1) loop breaks only in one case - on line 177
	// where the condition is that threadpool is in the process of shutting down
	// And because mutex is already locked on line 170, and unlock on line 186 will not be reachable
	// you must be assured that mutex is unlocked to avoid deadlock, so shall be done :) on line 196
    pthread_mutex_unlock(&(pool->mtx));
	pthread_exit(NULL);
	return NULL;
}