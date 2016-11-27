all:
	gcc -Wall -pedantic -pthread -o threads.out threads.c

tasks:
	gcc -shared -o tasks.so -fPIC tasks.c
	#gcc -c -fPIC tasks.c -o tasks.o
	#gcc tasks.o -shared -o tasks.so

dl: tasks
	gcc -Wall -pedantic -pthread -o threads_dl.out threads_dl.c -ldl

clean:
	rm -rf *.o

mrproper: clean
	rm -rf *.out
	rm -rf *.so
