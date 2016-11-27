all:
	gcc -Wall -pedantic -pthread -o threads.out threads.c

tasks.o: tasks.c tasks.h
	gcc -c -fPIC -Wall -o tasks.o tasks.c

tasks.so: tasks.o
	gcc -fPIC -shared -Wl,-soname,tasks.so -o tasks.so tasks.o

dl: tasks.so
	gcc -Wall -pedantic -pthread -o threads_dl.out threads_dl.c -ldl

clean:
	rm -rf *.o

mrproper: clean
	rm -rf *.out
	rm -rf *.so
