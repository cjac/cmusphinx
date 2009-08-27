all: liblm

OBJECTS = fillpen.o lm.o lm_attfsm.o lm_debug.o lm_ng.o lm_ng_dmp.o lmclass.o lmset.o
INCLUDES = -I../../include/sphinx3 -I../../include/sphinxbase

.c.o:
	gcc -g -O2 -Wall $(INCLUDES) -c -o $@ $<

liblm: $(OBJECTS)

clean:
	rm -f *.o
