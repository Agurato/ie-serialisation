#include "threads.h"

sem_t* mutexList = NULL;
LineInfo* lineInfo = NULL;
time_t* lineStarts = NULL;
time_t* lineEnds = NULL;

void* tasksHandle = NULL;

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
	*/
	int taskNb = 0, lineNb = -1, totalTask = 0, taskCount = 0, lineBeginThreadNb = 0;

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

	tasksHandle = dlopen("./tasks.so", RTLD_LAZY);
	if(! tasksHandle) {
		puts(dlerror());
		return 1;
	}

	for(i=0 ; i<totalTask ; i++) {
		pthread_create(&tasks[i], NULL, startThreads, &taskInfo[i]);
	}

	for(i=0 ; i<5 ; i++) {
		pthread_join(tasks[i], NULL);
	}


	dlclose(tasksHandle);

	return 0;
}

void* startThreads(void* arg) {
	TaskInfo *info = (TaskInfo*) arg;

    char *error;
	char* taskName = NULL;
	taskName = malloc(7*sizeof(char));
	sprintf(taskName, "task%d", info->task);

	void (*taskPtr)();

	while(1) {
		sem_wait(&mutexList[info->threadNb]);
		if(info->threadNb == lineInfo[info->line].firstTask) {
			time(&lineInfo[info->line].start);
			printf("\x1b[%dmline %d : task%d begin - timer starts\x1b[0m\n", 31+info->task, info->line, info->task);
		}
		else {
			printf("\x1b[%dmline %d : task%d begin\x1b[0m\n", 31+info->task, info->line, info->task);
		}

		*(void **) (&taskPtr) = dlsym(tasksHandle, taskName);
		if ((error = dlerror()) != NULL)  {
            puts(error);
            exit(1);
        }
		(*taskPtr)();

		time(&lineInfo[info->line].end);
		double diff = difftime(lineInfo[info->line].end, lineInfo[info->line].start);
		int millidiff = 1000*diff;
		int deadline = lineInfo[info->line].deadline;
		if(millidiff < deadline) {
			if(info->nextThread == lineInfo[info->line].firstTask) {
				printf("\x1b[%dmline %d : task%d end - line %d ended before deadline (%d < %d)\x1b[0m\n", 31+info->task, info->line, info->task, info->line, millidiff, deadline);
			}
			else {
				printf("\x1b[%dmline %d : task%d end\x1b[0m\n", 31+info->task, info->line, info->task);
			}
			sem_post(&mutexList[info->nextThread]);
		}
		else {
			printf("\x1b[%dmline %d : task%d end - deadline reached (%d > %d) : line %d stopped at task%d\x1b[0m\n", 31+info->task, info->line, info->task, millidiff, deadline, info->line, info->task);
			sem_post(&mutexList[lineInfo[info->line].firstTask]);
		}
	}

	return 0;
}
