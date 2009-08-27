all: libam

OBJECTS = mdef.o
INCLUDES = -I../../include/sphinx3 -I../../include/sphinxbase

.c.o:
	gcc -g -O2 -Wall $(INCLUDES) -c -o $@ $<

libam: $(OBJECTS)

clean:
	rm -f *.o
