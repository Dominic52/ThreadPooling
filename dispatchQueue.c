// Written by dyan263

#include "dispatchQueue.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <stdio.h>

// Adds a new task to the queue. Places the task on a node and links node in queue.
void add_to_list(dispatch_queue_t *queue, task_t *task)
{
    node_t *new_node;
    new_node = (node_t *)malloc(sizeof(node_t));
    new_node->task = task;
    new_node->next = queue->listhead;
    new_node->prev = queue->listhead->prev;
    queue->listhead->prev->next = new_node;
    queue->listhead->prev = new_node;
};

// Pops the first node of the queue in linked list. Returns one node
node_t *pop(dispatch_queue_t *queue)
{
    node_t *current;
    current = queue->listhead->next;
    queue->listhead->next = current->next;
    current->next->prev = queue->listhead;
    return current;
};

// Returns int length of queue
int qlen(dispatch_queue_t *queue)
{
    int count = 0;
    node_t *node;
    for (node = queue->listhead->next; node != queue->listhead; node = node->next)
    {
        count++;
    }
    return count;
}

// Thread loop which runs the new
void *threadloop(void *arg)
{
    dispatch_queue_t *queue = arg;
    node_t *node;

    while (1)
    {
        // Threads wait until signified to do work
        sem_wait(&sem);

        // Threads popping first task from queue and running

        node = pop(queue);
        node->task->work(node->task->params);
        // If dispatch type is SYNC, notifies sem2 that task has been run
        if (node->task->type == SYNC)
        {
            sem_post(&semsync);
        }

        // Destroys task from memory
        task_destroy(node->task);

        // If queue is empty, posts to dispatch_queue_wait function to unlock main thread
        if (qlen(queue) == 0)
        {
            sem_post(&queue->semqueue);
        }
    };
    pthread_exit(0);
};

// Creates a task
task_t *task_create(void (*work)(void *), void *params, char *name)
{
    task_t *task = (task_t *)malloc(sizeof(task_t));
    task->work = work;
    task->params = params;
    strcpy(task->name, name);
    return task;
};

// Destroys the task after completion
void task_destroy(task_t *task)
{
    free(task);
};

// Creates the dispatch queue
dispatch_queue_t *dispatch_queue_create(queue_type_t queueType)
{
    // Gets number of cores of machine
    int numCores = get_nprocs();

    //Initialise queue
    dispatch_queue_t *dispatchQueue = (dispatch_queue_t *)malloc(sizeof(dispatch_queue_t));
    dispatchQueue->queue_type = queueType;

    // Creates dummy node as head of queue
    node_t *newhead = (node_t *)malloc(sizeof(node_t));
    dispatchQueue->listhead = newhead;
    dispatchQueue->listhead->next = dispatchQueue->listhead;
    dispatchQueue->listhead->prev = dispatchQueue->listhead;

    // Initialise semaphores
    sem_init(&sem, 0, 0);
    sem_init(&dispatchQueue->semqueue, 0, 0);

    // Separate queuetype creates different number of threads
    if (queueType == CONCURRENT)
    {
        for (int i = 0; i < numCores; i++)
        {
            //Create new threads
            if (0 != pthread_create(&(tid[i]), NULL, (void *)threadloop, (void *)dispatchQueue))
            {
                printf("thread creation failed");
            };
        };
        dispatchQueue->threadnums = numCores;
    }
    else if (queueType == SERIAL)
    {
        //Create one new thread
        if (0 != pthread_create(&(tid[0]), NULL, (void *)threadloop, (void *)dispatchQueue))
        {
            printf("thread creation failed");
        };
        dispatchQueue->threadnums = 1;
    }
    else
    {
        printf("error\n");
    };
    return dispatchQueue;
};

// Destroys all semaphores and frees queue structure
void dispatch_queue_destroy(dispatch_queue_t *queue)
{
    sem_destroy(&sem);
    sem_destroy(&semsync);
    sem_destroy(&queue->semqueue);
    free(queue);
};

// Async type dispatch
int dispatch_async(dispatch_queue_t *queue, task_t *task)
{
    task->type = ASYNC;

    add_to_list(queue, task);
    // Signals an item has been dispatched to wake threads
    sem_post(&sem);

    return 0;
};

// Sync type dispatch
int dispatch_sync(dispatch_queue_t *queue, task_t *task)
{
    task->type = SYNC;

    // Initialises sync semaphore
    sem_init(&semsync, 0, 0);
    add_to_list(queue, task);
    // Signals an item has been dispatched to wake threads
    sem_post(&sem);
    // Signals sync semaphore to wait until sync semaphore is 
    sem_wait(&semsync);

    return 0;
};

// Dispatch queue is told to wait until all tasks are completed before letting main exit
int dispatch_queue_wait(dispatch_queue_t *queue)
{
    sem_wait(&queue->semqueue);
    return 0;
};