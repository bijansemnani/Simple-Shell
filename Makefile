all: myshell
			flex lex.l
			cc 	lex.yy.c myshell.c -g -lfl -o myshell
