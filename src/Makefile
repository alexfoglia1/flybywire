CC = gcc
CFLAGS = -I.
DEPS = genfail.h pfc.h pfcswitch.h transducers.h wes.h defs.h utils.h
LIBS = -lm
fly_by_wire:
	mkdir ../log
	mkdir ../tmp
	$(CC) -o ../bin/fly_by_wire utils.c genfail.c pfc.c pfcswitch.c transducers.c wes.c main.c $(LIBS)
