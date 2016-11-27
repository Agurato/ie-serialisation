#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>

void* startThreads(void* arg);

void task0();
void task1();
void task2();
void task3();
void task4();

typedef struct {
	/* numéro de la tâche à exécuter (fonction appelée) */
	int task;
	/* numéro du thread (pour le tableau de mutex) */
	int threadNb;
	/* numéro du thread à exécuter ensuite */
	int nextThread;
	/* numéro de la ligne */
	int line;
	/* position de la tâche dans la ligne */
	int pos;
	/* deadline à atteindre */
	int deadline;
} TaskInfo;

typedef struct {
	/* nombre de tâches dans la ligne */
	int taskNb;
	/* numéro de la première tâche sur la ligne */
	int firstTask;
	/* temps de début et de fin de la ligne */
	time_t start, end;
	/* deadline à atteindre */
	int deadline;
} LineInfo;
