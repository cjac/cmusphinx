all: jsgfparser

CPPFLAGS := -DYYDEBUG=1
CFLAGS   := -g -Wall `pkg-config --cflags sphinxbase`
LIBS     := `pkg-config --libs sphinxbase` -lfl

jsgfparser: jsgf.tab.o jsgf.lex.o
	gcc -o $@ jsgf.lex.o jsgf.tab.o $(LIBS)

jsgf.lex.c: jsgf.l jsgf.tab.h
	flex -o $@ $<

jsgf.tab.c: jsgf.y
	bison -d $<

clean:
	$(RM) *.tab.* *.o *.lex.* jsgfparser
