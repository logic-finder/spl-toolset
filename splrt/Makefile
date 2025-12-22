splrt:
	gcc -c -Wall -Werror -fpic splcore.c stack.adt.c stage.adt.c
	gcc -shared splcore.o stack.adt.o stage.adt.o -o libsplrt.so

sfname := hello.spl
exec:
	cd ../spldbm && spldbm --make=../dat/name,../dat/adj,../dat/noun,../dat/comp && mv words.spldb ../splt
	cd ../splt && splt ../dat/$(sfname) && mv temp.c ../splrt
	gcc -L/home/logicseeker/desktop/spl/splrt \
		-Wl,-rpath=/home/logicseeker/desktop/spl/splrt \
		-Wall -g -o temp temp.c -lsplrt -lm

clean:
	rm -f *.o libsplrt.so temp.c temp
