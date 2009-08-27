all: liblmmax

OBJECTS = lm_max.o
INCLUDES = -I../include/lmmax -I../include/sphinx3 -I../include/sphinxbase

.c.o:
	gcc -g -O2 -Wall $(INCLUDES) -c -o $@ $<

liblmmax: $(OBJECTS)

clean:
	rm -f *.o
