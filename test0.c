#include "dispatchQueue.h"
#include "dispatchQueue.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

void test0() {
    sleep(1);
    printf("test0 running\n");
}

void *dummy_runner(void *run_task) {
    task_t *my_task = (task_t *)run_task;
    printf("starting dummy_runner\n");
    my_task->work(NULL); /* possibly some parameters later */
    printf("finished dummy_runner and %s\n", my_task->name);
}

int main(int argc, char** argv) {
    //Task and Thread initialize
    task_t *task;
    pthread_t test_thread;
    //Create task with task_create, gets passed pointer to test0() function to print running, NULL params, char pointer name "test0"
    task = task_create(test0, NULL, "test0");

    //Start creating thread
    printf("Before\n");
    pthread_create(&test_thread, NULL, (void *)dummy_runner, (void *)task);
    printf("After\n");

    //Runs the thread
    pthread_join(test_thread, NULL);
    return EXIT_SUCCESS;
}