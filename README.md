```
LIST OF PROGRAMS

splc (work in progress) - spl compiler
spldbm (work in progress) - db generator for splc
splrt (work in progress) - runtime for the transpiled C program

splda (to be implemented) - dynamic analyzer
spldbg (to be implemented) - debugger
splv (to be implemented) - splo player (virtual executor)
splfmt (to be implemented) - source code formatter
splsa (to be implemented) - linter (code style checker); static analyzer
spli (to be implemented) - interpreter
splt (to be implemented) - playground
```
```bash
BUILD / USAGE

$ make -j
$ cd spldbm && make db && mv words.spldb ../splc
$ cd ../splc && splc ../dat/hello.spl
$ cd ../splrt && make build SPLC_OUTPUT=../splc/hello.c
$ cd ../splc && ./hello
```