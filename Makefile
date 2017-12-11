all:
	gcc -o debugee debugee.c
	gcc -o antidebug antidebug.c
	gcc -o procexp procexp.c
	gcc -o debugger debugger.c
