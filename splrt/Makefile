splrt:
	gcc -c -Wall -Werror -fpic splcore.c stack.adt.c stage.adt.c
	gcc -shared splcore.o stack.adt.o stage.adt.o -o libsplrt.so

exec:
	cd ../spldbm && spldbm --make=name,adj,noun,comp && mv words.spldb ../splt
	cd ../splt && splt hello.spl && mv temp.c ../splrt
	gcc -L/home/logicseeker/desktop/spl/splrt \
		-Wl,-rpath=/home/logicseeker/desktop/spl/splrt \
		-Wall -g -o temp temp.c -lsplrt -lm

clean:
	rm -f *.o libsplrt.so temp.c temp
