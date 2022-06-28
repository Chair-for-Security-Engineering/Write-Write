CC=gcc
OBJDMP=objdump

all: demo

demo: demo.c util.h
	$(CC) -o demo demo.c -lm
	$(OBJDMP) -drwC demo > dump_demo

clean:
	rm -f demo
