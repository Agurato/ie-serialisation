#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

void* task0(void* arg);
void* task1(void* arg);
void* task2(void* arg);
void* task3(void* arg);
void* task4(void* arg);

typedef struct {
	int nb;
	int next;
} TaskInfo;

sem_t* mutexList;

int main(int argc, char const *argv[]) {
	TaskInfo* taskInfo;
	
	pthread_t tasks[5];
	
	mutexList = malloc(5*sizeof(sem_t));
	sem_init(&mutexList[0], 0, 1);
	sem_init(&mutexList[1], 0, 1);
	sem_init(&mutexList[2], 0, 0);
	sem_init(&mutexList[3], 0, 0);
	sem_init(&mutexList[4], 0, 1);

	taskInfo = malloc(5*sizeof(TaskInfo));
	taskInfo[0].nb = 0;
	taskInfo[0].next = 0;
	taskInfo[1].nb = 1;
	taskInfo[1].next = 2;
	taskInfo[2].nb = 2;
	taskInfo[2].next = 3;
	taskInfo[3].nb = 3;
	taskInfo[3].next = 1;
	taskInfo[4].nb = 4;
	taskInfo[4].next = 4;

	pthread_create(&tasks[0], NULL, task0, &taskInfo[0]);
	pthread_create(&tasks[1], NULL, task1, &taskInfo[1]);
	pthread_create(&tasks[2], NULL, task2, &taskInfo[2]);
	pthread_create(&tasks[3], NULL, task3, &taskInfo[3]);
	pthread_create(&tasks[4], NULL, task4, &taskInfo[4]);

	pthread_join(tasks[0], NULL);
	pthread_join(tasks[1], NULL);
	pthread_join(tasks[2], NULL);
	pthread_join(tasks[3], NULL);
	pthread_join(tasks[4], NULL);

	return 0;
}

void* task0(void* arg) {
	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->nb]);
		puts("\x1b[31mtask0 begin\x1b[0m");
		sleep(2);
		puts("\x1b[31mtask0 end\x1b[0m");
		sem_post(&mutexList[info->next]);
	}

	return 0;
}

void* task1(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->nb]);
		puts("\x1b[32mtask1 begin\x1b[0m");
		sleep(2);
		puts("\x1b[32mtask1 end\x1b[0m");
		sem_post(&mutexList[info->next]);
	}

	return 0;
}

void* task2(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->nb]);
		puts("\x1b[32mtask2 begin\x1b[0m");
		sleep(2);
		puts("\x1b[32mtask2 end\x1b[0m");
		sem_post(&mutexList[info->next]);
	}

	return 0;
}

void* task3(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->nb]);
		puts("\x1b[32mtask3 begin\x1b[0m");
		sleep(2);
		puts("\x1b[32mtask3 end\x1b[0m");
		sem_post(&mutexList[info->next]);
	}

	return 0;
}

void* task4(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->nb]);
		puts("\x1b[34mtask4 begin\x1b[0m");
		sleep(2);
		puts("\x1b[34mtask4 end\x1b[0m");
		sem_post(&mutexList[info->next]);
	}

	return 0;
}
