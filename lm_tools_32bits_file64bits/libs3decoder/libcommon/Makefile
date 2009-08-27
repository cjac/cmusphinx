all: libcommon

OBJECTS = corpus.o encoding.o logs3.o
INCLUDES = -I../liblm_Ng -I../../include/sphinx3 -I../../include/sphinxbase -I../../include/lmmax

.c.o:
	gcc -g -O2 -Wall $(INCLUDES) -c -o $@ $<

libcommon: $(OBJECTS)

clean:
	rm -f *.o
