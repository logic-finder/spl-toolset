splrt:
	gcc -c -Wall -Werror -fpic core.c stack.adt.c stage.adt.c
	gcc -shared core.o stack.adt.o stage.adt.o -o libsplrt.so

exec:
	gcc -L/home/logicseeker/desktop/spl/rt -Wall -g -o temp temp.c -lsplrt -lm