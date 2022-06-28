CC=gcc
OBJDMP=objdump

all: ev

ev: write+write.c write+write.h
	$(CC) -o ev_sets write+write.c -lm -O3
	$(OBJDMP) -drwC ev_sets > dump_evsets

clean:
	rm -f ev_sets