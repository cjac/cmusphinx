all: jsgfparser

CPPFLAGS := -DYYDEBUG=1
CFLAGS   := -g -Wall `pkg-config --cflags sphinxbase`
LIBS     := `pkg-config --libs sphinxbase` -lfl

jsgfparser: jsgf.tab.o jsgf.lex.o jsgf.o
	gcc -o $@ jsgf.lex.o jsgf.tab.o jsgf.o $(LIBS)

jsgf.lex.c: jsgf.l jsgf.tab.h
	flex -o $@ $<

jsgf.tab.c: jsgf.y jsgf.h
	bison -d $< -o $@

jsgf.c: jsgf.h

clean:
	$(RM) *.tab.* *.o *.lex.* jsgfparser
