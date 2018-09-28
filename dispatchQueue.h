/* 
 * File:   dispatchQueue.h
 * Author: robert
 *
 * Modified by: dyan263
 */

#ifndef DISPATCHQUEUE_H
#define DISPATCHQUEUE_H

#include <pthread.h>
#include <semaphore.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define error_exit(MESSAGE) perror(MESSAGE), exit(EXIT_FAILURE)

typedef enum
{ // whether dispatching a task synchronously or asynchronously
    ASYNC,
    SYNC
} task_dispatch_type_t;

typedef enum
{ // The type of dispatch queue.
    CONCURRENT,
    SERIAL
} queue_type_t;

typedef struct task
{
    char name[64];             // to identify it when debugging
    void (*work)(void *);      // the function to perform
    void *params;              // parameters to pass to the function
    task_dispatch_type_t type; // asynchronous or synchronous
} task_t;

typedef struct dispatch_queue_t dispatch_queue_t;               // the dispatch queue type
typedef struct dispatch_queue_thread_t dispatch_queue_thread_t; // the dispatch queue thread type

struct dispatch_queue_thread_t
{
    dispatch_queue_t *queue; // the queue this thread is associated with
    pthread_t thread;        // the thread which runs the task
    sem_t thread_semaphore;  // the semaphore the thread waits on until a task is allocated
    task_t *task;            // the current task for this thread
};

typedef struct node node_t;

struct node
{
    task_t *task;
    node_t *prev;
    node_t *next;
};

struct dispatch_queue_t
{
    queue_type_t queue_type;
    node_t *listhead;
    pthread_t *queue_threads;
    sem_t semqueue;
    int threadnums;
};

task_t *task_create(void (*work)(void *), void *params, char *name);

void task_destroy(task_t *);

dispatch_queue_t *dispatch_queue_create(queue_type_t);

void dispatch_queue_destroy(dispatch_queue_t *);

int dispatch_async(dispatch_queue_t *, task_t *);

int dispatch_sync(dispatch_queue_t *, task_t *);

int dispatch_queue_wait(dispatch_queue_t *);

// Initialise global semaphores
sem_t sem, semsync;

//Initialise thread pool array
pthread_t tid[30];

#endif /* DISPATCHQUEUE_H */