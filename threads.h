#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>

void* task0(void* arg);
void* task1(void* arg);
void* task2(void* arg);
void* task3(void* arg);
void* task4(void* arg);

typedef struct {
	int task;
	int threadNb;
	int nextThread;
	int line;
	int pos;
	int deadline;
} TaskInfo;
