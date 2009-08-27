all: libsearch

OBJECTS = astar.o ctxt_table.o dag.o dict2pid.o vithist.o linklist.o s3_paul.o
INCLUDES = -I../../include/sphinx3 -I../../include/sphinxbase -I../../include/lmmax

.c.o:
	gcc -g -O2 -Wall $(INCLUDES) -c -o $@ $<

libsearch: $(OBJECTS)

clean:
	rm -f *.o
