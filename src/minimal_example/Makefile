CC=gcc
OBJDMP=objdump

all: demo

demo: minimal.c
	$(CC) -o demo minimal.c -lm 
	$(OBJDMP) -drwC demo > dump_demo

clean:
	rm -f demo dump_demo