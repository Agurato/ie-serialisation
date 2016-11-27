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
		if(info->threadNb == lineInfo[info->line].firstTask) {
			time(&lineInfo[info->line].start);
			printf("\x1b[31mline %d : task0 begin - timer starts\x1b[0m\n", info->line);
		}
		else {
			printf("\x1b[31mline %d : task0 begin\x1b[0m\n", info->line);
		}

		sleep(2);

		time(&lineInfo[info->line].end);
		double diff = difftime(lineInfo[info->line].end, lineInfo[info->line].start);
		int millidiff = 1000*diff;
		int deadline = lineInfo[info->line].deadline;
		if(millidiff < deadline) {
			if(info->nextThread == lineInfo[info->line].firstTask) {
				printf("\x1b[31mline %d : task0 end - line %d ended before deadline (%d < %d)\x1b[0m\n", info->line, info->line, millidiff, deadline);
			}
			else {
				printf("\x1b[31mline %d : task0 end\x1b[0m\n", info->line);
			}
			sem_post(&mutexList[info->nextThread]);
		}
		else {
			printf("\x1b[31mline %d : task0 end - deadline reached (%d > %d) : line %d stopped at task0\x1b[0m\n", info->line, millidiff, deadline, info->line);
			sem_post(&mutexList[lineInfo[info->line].firstTask]);
		}
	}

	return 0;
}

void* task1(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		if(info->threadNb == lineInfo[info->line].firstTask) {
			time(&lineInfo[info->line].start);
			printf("\x1b[32mline %d : task1 begin - timer starts\x1b[0m\n", info->line);
		}
		else {
			printf("\x1b[32mline %d : task1 begin\x1b[0m\n", info->line);
		}

		sleep(2);

		time(&lineInfo[info->line].end);
		double diff = difftime(lineInfo[info->line].end, lineInfo[info->line].start);
		int millidiff = 1000*diff;
		int deadline = lineInfo[info->line].deadline;
		if(millidiff < deadline) {
			if(info->nextThread == lineInfo[info->line].firstTask) {
				printf("\x1b[32mline %d : task1 end - line %d ended before deadline (%d < %d)\x1b[0m\n", info->line, info->line, millidiff, deadline);
			}
			else {
				printf("\x1b[32mline %d : task1 end\x1b[0m\n", info->line);
			}
			sem_post(&mutexList[info->nextThread]);
		}
		else {
			printf("\x1b[32mline %d : task1 end - deadline reached (%d > %d) : line %d stopped at task1\x1b[0m\n", info->line, millidiff, deadline, info->line);
			sem_post(&mutexList[lineInfo[info->line].firstTask]);
		}
	}

	return 0;
}

void* task2(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		if(info->threadNb == lineInfo[info->line].firstTask) {
			time(&lineInfo[info->line].start);
			printf("\x1b[32mline %d : task2 begin - timer starts\x1b[0m\n", info->line);
		}
		else {
			printf("\x1b[32mline %d : task2 begin\x1b[0m\n", info->line);
		}

		sleep(2);

		time(&lineInfo[info->line].end);
		double diff = difftime(lineInfo[info->line].end, lineInfo[info->line].start);
		int millidiff = 1000*diff;
		int deadline = lineInfo[info->line].deadline;
		if(millidiff < deadline) {
			if(info->nextThread == lineInfo[info->line].firstTask) {
				printf("\x1b[32mline %d : task2 end - line %d ended before deadline (%d < %d)\x1b[0m\n", info->line, info->line, millidiff, deadline);
			}
			else {
				printf("\x1b[32mline %d : task2 end\x1b[0m\n", info->line);
			}
			sem_post(&mutexList[info->nextThread]);
		}
		else {
			printf("\x1b[32mline %d : task2 end - deadline reached (%d > %d) : line %d stopped at task2\x1b[0m\n", info->line, millidiff, deadline, info->line);
			sem_post(&mutexList[lineInfo[info->line].firstTask]);
		}
	}

	return 0;
}

void* task3(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		if(info->threadNb == lineInfo[info->line].firstTask) {
			time(&lineInfo[info->line].start);
			printf("\x1b[32mline %d : task3 begin - timer starts\x1b[0m\n", info->line);
		}
		else {
			printf("\x1b[32mline %d : task3 begin\x1b[0m\n", info->line);
		}

		sleep(2);

		time(&lineInfo[info->line].end);
		double diff = difftime(lineInfo[info->line].end, lineInfo[info->line].start);
		int millidiff = 1000*diff;
		int deadline = lineInfo[info->line].deadline;
		if(millidiff < deadline) {
			if(info->nextThread == lineInfo[info->line].firstTask) {
				printf("\x1b[32mline %d : task3 end - line %d ended before deadline (%d < %d)\x1b[0m\n", info->line, info->line, millidiff, deadline);
			}
			else {
				printf("\x1b[32mline %d : task3 end\x1b[0m\n", info->line);
			}
			sem_post(&mutexList[info->nextThread]);
		}
		else {
			printf("\x1b[32mline %d : task3 end - deadline reached (%d > %d) : line %d stopped at task3\x1b[0m\n", info->line, millidiff, deadline, info->line);
			sem_post(&mutexList[lineInfo[info->line].firstTask]);
		}
	}

	return 0;
}

void* task4(void* arg) {

	TaskInfo *info = (TaskInfo*) arg;

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		if(info->threadNb == lineInfo[info->line].firstTask) {
			time(&lineInfo[info->line].start);
			printf("\x1b[34mline %d : task4 begin - timer starts\x1b[0m\n", info->line);
		}
		else {
			printf("\x1b[34mline %d : task4 begin\x1b[0m\n", info->line);
		}

		sleep(2);

		time(&lineInfo[info->line].end);
		double diff = difftime(lineInfo[info->line].end, lineInfo[info->line].start);
		int millidiff = 1000*diff;
		int deadline = lineInfo[info->line].deadline;
		if(millidiff < deadline) {
			if(info->nextThread == lineInfo[info->line].firstTask) {
				printf("\x1b[34mline %d : task4 end - line %d ended before deadline (%d < %d)\x1b[0m\n", info->line, info->line, millidiff, deadline);
			}
			else {
				printf("\x1b[34mline %d : task4 end\x1b[0m\n", info->line);
			}
			sem_post(&mutexList[info->nextThread]);
		}
		else {
			printf("\x1b[34mline %d : task4 end - deadline reached (%d > %d) : line %d stopped at task4\x1b[0m\n", info->line, millidiff, deadline, info->line);
			sem_post(&mutexList[lineInfo[info->line].firstTask]);
		}
	}

	return 0;
}
