all: util

OBJECTS = bio.o case.o ckd_alloc.o cmd_ln.o dtoa.o err.o filename.o glist.o hash_table.o heap.o info.o listelem_alloc.o logmath.o mmio.o pio.o profile.o strfuncs.o unlimit.o
INCLUDES = -I../../include/sphinxbase

.c.o:
	gcc -g -O2 -Wall $(INCLUDES) -c -o $@ $<

util: $(OBJECTS)

clean:
	rm -f *.o
