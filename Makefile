all: tasks
	gcc -Wall -pedantic -pthread -o threads.out threads.c -ldl

tasks: tasks.c tasks.h
	gcc -shared -o tasks.so -fPIC tasks.c

clean:
	rm -rf *.out
	rm -rf *.so
