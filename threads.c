#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

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
} TaskInfo;

sem_t* mutexList = NULL;

int main(int argc, char const *argv[]) {
	TaskInfo* taskInfo = NULL;
	pthread_t* tasks = NULL;

	FILE* taskListFile;
	char* line = NULL;
	size_t length = 0;
	ssize_t read;

	char* lineToken;
	char* taskToken;
	char* lineTasks;
	int taskNb = 0, lineNb = -1, totalTask = 0, taskCount = 0, taskNbLine = 0, lineBeginThreadNb = 0;

	int i = 0;

	taskListFile = fopen("taskList.txt", "r");
	if(taskListFile == NULL) {
		return 1;
	}

	while((read = getline(&line, &length, taskListFile)) != -1) {
		if(line[strlen(line)-1] == '\n') {
			line[strlen(line)-1] = '\0';
		}

		lineToken = strtok(line, ":");

		if(lineNb == -1 && strcmp(lineToken, "TASK_NB") == 0) {
			totalTask = atoi(strtok(NULL, ":"));

			mutexList = malloc(totalTask*sizeof(sem_t));
			sem_init(&mutexList[0], 0, 1);
			sem_init(&mutexList[1], 0, 1);
			sem_init(&mutexList[2], 0, 0);
			sem_init(&mutexList[3], 0, 0);
			sem_init(&mutexList[4], 0, 1);

			tasks = malloc(totalTask*sizeof(pthread_t));
			taskInfo = malloc(totalTask*sizeof(TaskInfo));

			continue;
		}
		lineNb ++;
		lineBeginThreadNb = taskCount;

		taskNbLine = atoi(lineToken);
		lineTasks = strtok(NULL, ":");

		while(lineToken != NULL) {
			lineToken = strtok(NULL, ":");
		}

		taskNb = 0;
		taskToken = strtok(lineTasks, "-");
		while(taskToken != NULL) {

			if(strcmp(taskToken, "END") == 0) {
				taskInfo[taskCount-1].nextThread = lineBeginThreadNb;
				taskToken = strtok(NULL, "-");
				continue;
			}
			else {
				taskInfo[taskCount].task = atoi(taskToken);
				taskInfo[taskCount].threadNb = taskCount;
				taskInfo[taskCount].line = lineNb;
				taskInfo[taskCount].pos = taskNb;
				taskInfo[taskCount].nextThread = taskCount+1;
			}
			taskToken = strtok(NULL, "-");

			taskCount ++;
			printf("%d/%d - %d;%d -> %s\n", taskCount, totalTask, lineNb, taskNb, taskToken);
			taskNb ++;
		}
	}

	fclose(taskListFile);
	if(line) {
		free(line);
	}

	printf("%d lines, %d tasks\n", lineNb+1, totalTask);

	for(i=0 ; i<totalTask ; i++) {
		switch(taskInfo[i].task) {
			case 0:
				pthread_create(&tasks[i], NULL, task0, &taskInfo[i]);
				break;
			case 1:
				pthread_create(&tasks[i], NULL, task1, &taskInfo[i]);
				break;
			case 2:
				pthread_create(&tasks[i], NULL, task2, &taskInfo[i]);
				break;
			case 3:
				pthread_create(&tasks[i], NULL, task3, &taskInfo[i]);
				break;
			case 4:
				pthread_create(&tasks[i], NULL, task4, &taskInfo[i]);
				break;
		}
		printf("\n");
	}

	for(i=0 ; i<5 ; i++) {
		pthread_join(tasks[i], NULL);
		pthread_join(tasks[i], NULL);
		pthread_join(tasks[i], NULL);
		pthread_join(tasks[i], NULL);
		pthread_join(tasks[i], NULL);
	}

	return 0;
}

void* task0(void* arg) {
	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		puts("\x1b[31mtask0 begin\x1b[0m");
		sleep(2);
		puts("\x1b[31mtask0 end\x1b[0m");
		sem_post(&mutexList[info->nextThread]);
	}

	return 0;
}

void* task1(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		puts("\x1b[32mtask1 begin\x1b[0m");
		sleep(2);
		puts("\x1b[32mtask1 end\x1b[0m");
		sem_post(&mutexList[info->nextThread]);
	}

	return 0;
}

void* task2(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		puts("\x1b[32mtask2 begin\x1b[0m");
		sleep(2);
		puts("\x1b[32mtask2 end\x1b[0m");
		sem_post(&mutexList[info->nextThread]);
	}

	return 0;
}

void* task3(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		puts("\x1b[32mtask3 begin\x1b[0m");
		sleep(2);
		puts("\x1b[32mtask3 end\x1b[0m");
		sem_post(&mutexList[info->nextThread]);
	}

	return 0;
}

void* task4(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		puts("\x1b[34mtask4 begin\x1b[0m");
		sleep(2);
		puts("\x1b[34mtask4 end\x1b[0m");
		sem_post(&mutexList[info->nextThread]);
	}

	return 0;
}
