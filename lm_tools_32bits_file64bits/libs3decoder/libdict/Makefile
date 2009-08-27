all: libdict

OBJECTS = cmu6_lts_rules.o dict.o lts.o wid.o
INCLUDES = -I../liblm_Ng -I../../include/sphinx3 -I../../include/sphinxbase

.c.o:
	gcc -g -O2 -Wall $(INCLUDES) -c -o $@ $<

libdict: $(OBJECTS)

clean:
	rm -f *.o
