CC = gcc
CFLAGS = -I.
DEPS = genfail.h pfc.h pfcswitch.h transducers.h wes.h defs.h utils.h
LIBS = -lm
fly_by_wire:
	mkdir -p ../log
	mkdir -p ../tmp
	mkdir -p ../bin
	$(CC) -o ../bin/fly_by_wire utils.c genfail.c pfc.c pfcswitch.c transducers.c wes.c main.c $(LIBS)
	cp ./input/G18.txt ../bin/G18.txt
