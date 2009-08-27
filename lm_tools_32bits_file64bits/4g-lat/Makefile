all: 4g-lat

OBJECTS = linklist.o s3_dag.o
INCLUDES = -I../include/4g-lat -I../include/sphinx3 -I../include/sphinxbase

.c.o:
	gcc -g -O2 -Wall $(INCLUDES) -c -o $@ $<

4g-lat: $(OBJECTS)

clean:
	rm -f *.o
