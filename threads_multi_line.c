#include "threads.h"

sem_t* mutexList = NULL;
LineInfo* lineInfo = NULL;
time_t* lineStarts = NULL;
time_t* lineEnds = NULL;

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

	/*
	taskNb : place de la tâche dans la ligne
	lineNb : place de la ligne dans le fichier
	totalTask : nombre de tâches à exécuter
	taskCount : compteur du nombre de tâches (incrémenté à chaque fois qu'on en voit une dans le fichier)
	lineBeginThreadNb : numéro de la tâche en début de ligne
	lineSize : nombre de tâches dans la ligne
	*/
	int taskNb = 0, lineNb = -1, totalTask = 0, taskCount = 0, lineBeginThreadNb = 0, lineSize = 0;

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
			tasks = malloc(totalTask*sizeof(pthread_t));
			taskInfo = malloc(totalTask*sizeof(TaskInfo));

			continue;
		}
		if(lineNb == -1 && strcmp(lineToken, "LINE_NB") == 0) {
			lineInfo = malloc(atoi(strtok(NULL, ":"))*sizeof(LineInfo));

			continue;
		}
		lineNb ++;
		lineInfo[lineNb].taskNb = atoi(lineToken);
		lineInfo[lineNb].firstTask = taskCount;
		lineBeginThreadNb = taskCount;

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
				taskInfo[taskCount-1].deadline = atoi(taskToken);

				lineInfo[lineNb].deadline = atoi(taskToken);
				printf("%d\n", lineInfo[lineNb].deadline);

				break;
			}
			else {

				if(taskCount == lineBeginThreadNb) {
					sem_init(&mutexList[taskCount], 0, 1);
				}
				else {
					sem_init(&mutexList[taskCount], 0, 0);
				}

				taskInfo[taskCount].task = atoi(taskToken);
				taskInfo[taskCount].threadNb = taskCount;
				taskInfo[taskCount].line = lineNb;
				taskInfo[taskCount].pos = taskNb;
				taskInfo[taskCount].nextThread = taskCount+1;
				if(taskNb == 0) {
					taskInfo[taskCount].deadline = -2;
				}
				else {
					taskInfo[taskCount].deadline = -1;
				}
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

	lineStarts = malloc((lineNb+1)*sizeof(time_t));
	lineEnds = malloc((lineNb+1)*sizeof(time_t));

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
	}

	for(i=0 ; i<5 ; i++) {
		pthread_join(tasks[i], NULL);
	}

	return 0;
}

void* task0(void* arg) {
	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		printf("\x1b[31mtask0 begin\x1b[0m");
		if(info->deadline == -2 || info->threadNb == info->nextThread) {
			puts(" - timer starts\x1b[0m");
			time(&lineStarts[info->line]);
		}
		else {
			puts("");
		}

		sleep(2);

		if(info->deadline != -2 && info->deadline != -1) {
			time(&lineEnds[info->line]);
			double diff = difftime(lineEnds[info->line], lineStarts[info->line]);
			int millidiff = 1000*diff;
			if(millidiff < info->deadline) {
				printf("\x1b[31mtask0 end (%d < %d) OK\x1b[0m\n", millidiff, info->deadline);
			}
			else {
				printf("\x1b[31mtask0 end (%d > %d) PAS OK\x1b[0m\n", millidiff, info->deadline);
			}
		}
		else {
			puts("\x1b[31mtask0 end\x1b[0m");
		}
		sem_post(&mutexList[info->nextThread]);
	}

	return 0;
}

void* task1(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		printf("\x1b[32mtask1 begin\x1b[0m");
		if(info->deadline == -2 || info->threadNb == info->nextThread) {
			puts(" - timer starts\x1b[0m");
			time(&lineStarts[info->line]);
		}
		else {
			puts("");
		}

		sleep(2);

		if(info->deadline != -2 && info->deadline != -1) {
			time(&lineEnds[info->line]);
			double diff = difftime(lineEnds[info->line], lineStarts[info->line]);
			int millidiff = 1000*diff;
			if(millidiff < info->deadline) {
				printf("\x1b[32mtask1 end (%d < %d) OK\x1b[0m\n", millidiff, info->deadline);
			}
			else {
				printf("\x1b[32mtask1 end (%d > %d) PAS OK\x1b[0m\n", millidiff, info->deadline);
			}
		}
		else {
			puts("\x1b[32mtask1 end\x1b[0m");
		}
		sem_post(&mutexList[info->nextThread]);
	}

	return 0;
}

void* task2(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		printf("\x1b[32mtask2 begin\x1b[0m");
		if(info->deadline == -2 || info->threadNb == info->nextThread) {
			puts(" - timer starts\x1b[0m");
			time(&lineStarts[info->line]);
		}
		else {
			puts("");
		}

		sleep(2);

		if(info->deadline != -2 && info->deadline != -1) {
			time(&lineEnds[info->line]);
			double diff = difftime(lineEnds[info->line], lineStarts[info->line]);
			int millidiff = 1000*diff;
			if(millidiff < info->deadline) {
				printf("\x1b[32mtask2 end (%d < %d) OK\x1b[0m\n", millidiff, info->deadline);
			}
			else {
				printf("\x1b[32mtask2 end (%d > %d) PAS OK\x1b[0m\n", millidiff, info->deadline);
			}
		}
		else {
			puts("\x1b[32mtask2 end\x1b[0m");
		}
		sem_post(&mutexList[info->nextThread]);
	}

	return 0;
}

void* task3(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		printf("\x1b[32mtask3 begin\x1b[0m");
		if(info->deadline == -2 || info->threadNb == info->nextThread) {
			puts(" - timer starts\x1b[0m");
			time(&lineStarts[info->line]);
		}
		else {
			puts("");
		}

		sleep(2);

		if(info->deadline != -2 && info->deadline != -1) {
			time(&lineEnds[info->line]);
			double diff = difftime(lineEnds[info->line], lineStarts[info->line]);
			int millidiff = 1000*diff;
			if(millidiff < info->deadline) {
				printf("\x1b[32mtask3 end (%d < %d) OK\x1b[0m\n", millidiff, info->deadline);
			}
			else {
				printf("\x1b[32mtask3 end (%d > %d) PAS OK\x1b[0m\n", millidiff, info->deadline);
			}
		}
		else {
			puts("\x1b[32mtask3 end\x1b[0m");
		}
		sem_post(&mutexList[info->nextThread]);
	}

	return 0;
}

void* task4(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		printf("\x1b[34mtask4 begin\x1b[0m");
		if(info->deadline == -2 || info->threadNb == info->nextThread) {
			puts(" - timer starts\x1b[0m");
			time(&lineStarts[info->line]);
		}
		else {
			puts("");
		}

		sleep(2);

		if(info->deadline != -2 && info->deadline != -1) {
			time(&lineEnds[info->line]);
			double diff = difftime(lineEnds[info->line], lineStarts[info->line]);
			int millidiff = 1000*diff;
			if(millidiff < info->deadline) {
				printf("\x1b[34mtask4 end (%d < %d) OK\x1b[0m\n", millidiff, info->deadline);
			}
			else {
				printf("\x1b[34mtask4 end (%d > %d) PAS OK\x1b[0m\n", millidiff, info->deadline);
			}
		}
		else {
			puts("\x1b[34mtask4 end\x1b[0m");
		}
		sem_post(&mutexList[info->nextThread]);
	}

	return 0;
}
