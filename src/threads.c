#include "threads.h"

/* mutexList: liste de mutex utilisés (1 par thread) */
sem_t* mutexList = NULL;
/* lineInfo: tableau stockant les infos pour chaque ligne */
LineInfo* lineInfo = NULL;
/* tasksHandle: utilisée par dlopen, dlsym et dlclose */
void* tasksHandle = NULL;

int main(int argc, char const *argv[]) {
	/* taskInfo: tableau pour stocker les infos de chaque tâche */
	TaskInfo* taskInfo = NULL;
	/* tasks: tableau de pthread_t pour les threads */
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
	*/
	int taskNb = 0, lineNb = -1, totalTask = 0, taskCount = 0;

	int i = 0;

	/* Ouverture du fichier de conf des tâches */
	taskListFile = fopen("taskList.txt", "r");
	if(taskListFile == NULL) {
		return 1;
	}
	/* Lecture du fichier ligne par ligne */
	while((read = getline(&line, &length, taskListFile)) != -1) {
		/* Suppression des retours à la lignes finaux */
		if(line[strlen(line)-1] == '\n') {
			line[strlen(line)-1] = '\0';
		}

		/* Parsing de la ligne */
		lineToken = strtok(line, ":");

		/* Si on est sur la ligne commençant par "TASK_NB", on récupère le nombre de tâches à exécuter */
		if(lineNb == -1 && strcmp(lineToken, "TASK_NB") == 0) {
			totalTask = atoi(strtok(NULL, ":"));

			/* On alloue les différents tableaux */
			mutexList = malloc(totalTask*sizeof(sem_t));
			tasks = malloc(totalTask*sizeof(pthread_t));
			taskInfo = malloc(totalTask*sizeof(TaskInfo));

			continue;
		}
		/* Si on est sur la ligne commençant par "LINE_NB", on récupère le nombre de lignes à paralléliser */
		if(lineNb == -1 && strcmp(lineToken, "LINE_NB") == 0) {
			lineInfo = malloc(atoi(strtok(NULL, ":"))*sizeof(LineInfo));

			continue;
		}
		lineNb ++;
		/* Complétion des infos de la ligne d'exécution */
		lineInfo[lineNb].taskNb = atoi(lineToken);
		lineInfo[lineNb].firstTask = taskCount;

		lineTasks = strtok(NULL, ":");

		/* Vidage du buffer de strtok() */
		while(lineToken != NULL) {
			lineToken = strtok(NULL, ":");
		}

		taskNb = 0;
		taskToken = strtok(lineTasks, "-");
		/* Parsing de la deuxième partie de la ligne (tâches à exécuter, échéance) */
		while(taskToken != NULL) {
			/* Si on atteint la fin des tâches */
			if(strcmp(taskToken, "END") == 0) {
				/* On boucle la dernière tâche de la ligne sur la première */
				taskInfo[taskCount-1].nextThread = lineInfo[lineNb].firstTask;

				/* On récupère l'échéance */
				taskToken = strtok(NULL, "-");
				lineInfo[lineNb].deadline = atoi(taskToken);

				break;
			}
			/* Si on parse un numéro de tâche */
			else {
				/* Si on est à la première tâche de la ligne */
				if(taskCount == lineInfo[lineNb].firstTask) {
					/* Initialisation du mutex à 1 */
					sem_init(&mutexList[taskCount], 0, 1);
				}
				/* Sinon */
				else {
					/* Initialisation du mutex à 0 */
					sem_init(&mutexList[taskCount], 0, 0);
				}

				/* Complétion des infos sur la tâche à effectuer */
				taskInfo[taskCount].task = atoi(taskToken);
				taskInfo[taskCount].threadNb = taskCount;
				taskInfo[taskCount].line = lineNb;
				taskInfo[taskCount].nextThread = taskCount+1;
			}
			taskToken = strtok(NULL, "-");

			taskCount ++;
			taskNb ++;
		}
	}

	/* Fermeture du fichier */
	fclose(taskListFile);
	if(line) {
		free(line);
	}

	/* Initialisation du chargement à chaud de la bibliothèque */
	tasksHandle = dlopen("./tasks.so", RTLD_LAZY);
	if(! tasksHandle) {
		puts(dlerror());
		return 1;
	}

	/* Création des threads */
	for(i=0 ; i<totalTask ; i++) {
		pthread_create(&tasks[i], NULL, startThread, &taskInfo[i]);
	}

	/* Attente de la fin des threads */
	for(i=0 ; i<5 ; i++) {
		pthread_join(tasks[i], NULL);
	}

	/* Fermeture de la bibliothèque utilisée */
	dlclose(tasksHandle);

	return 0;
}

void* startThread(void* arg) {
	/* Récupération des infos de la tâche */
	TaskInfo *info = (TaskInfo*) arg;

	void (*taskPtr)();
    char *error;
	char* taskName = NULL;
	/* Chargement du nom de la tâche à appeler */
	taskName = malloc(7*sizeof(char));
	sprintf(taskName, "task%d", info->task);

	/* Récupération de la tâche à appeler */
	*(void **) (&taskPtr) = dlsym(tasksHandle, taskName);
	if ((error = dlerror()) != NULL)  {
		puts(error);
		exit(1);
	}

	/* Boucle principale */
	while(1) {
		/* Attente de la libération du mutex */
		sem_wait(&mutexList[info->threadNb]);
		/* Si on est au début de la ligne */
		if(info->threadNb == lineInfo[info->line].firstTask) {
			/* Début du timer */
			time(&lineInfo[info->line].start);
			printf("\x1b[%dmline %d : task%d begin - timer starts\x1b[0m\n", 31+info->task, info->line, info->task);
		}
		else {
			printf("\x1b[%dmline %d : task%d begin\x1b[0m\n", 31+info->task, info->line, info->task);
		}
		
		/* Appel de la tâche */
		(*taskPtr)();

		/* Fin du timer */
		time(&lineInfo[info->line].end);
		double diff = difftime(lineInfo[info->line].end, lineInfo[info->line].start);
		int millidiff = 1000*diff;
		int deadline = lineInfo[info->line].deadline;
		/* Si la deadline n'a pas été dépassée */
		if(millidiff < deadline) {
			if(info->nextThread == lineInfo[info->line].firstTask) {
				printf("\x1b[%dmline %d : task%d end - line %d ended before deadline (%d < %d)\x1b[0m\n", 31+info->task, info->line, info->task, info->line, millidiff, deadline);
			}
			else {
				printf("\x1b[%dmline %d : task%d end\x1b[0m\n", 31+info->task, info->line, info->task);
			}
			/* On continue avec la prochaine tâche de la ligne */
			sem_post(&mutexList[info->nextThread]);
		}
		/* Si la deadline a été dépassée */
		else {
			printf("\x1b[%dmline %d : task%d end - deadline reached (%d > %d) : line %d stopped at task%d\x1b[0m\n", 31+info->task, info->line, info->task, millidiff, deadline, info->line, info->task);
			/* On reprend depuis le début de la ligne */
			sem_post(&mutexList[lineInfo[info->line].firstTask]);
		}
	}

	return 0;
}
