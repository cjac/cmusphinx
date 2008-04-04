CFLAGS= -g -Wall
PHOENIX_OBS=  parse.o match.o grammar.o dict.o pconf.o  print_structs.o 

all: libparse.a compile_grammar concept_leaf parse_text nfa_to_dfa

libparse.a:  $(PHOENIX_OBS)
	ar rv $@ $?
	ranlib $@

parse.o:   parse.c parse.h grammar.h functions.h pconf.h
	gcc $(CFLAGS) -c parse.c 

match.o:   match.c parse.h grammar.h globals_parse.h globals_gram.h functions.h
	gcc $(CFLAGS) -c match.c 

grammar.o:   grammar.c parse.h grammar.h globals_gram.h functions.h
	gcc $(CFLAGS) -c grammar.c 

dict.o:   dict.c grammar.h
	gcc $(CFLAGS) -c dict.c 

pconf.o:   pconf.c pconf.h 
	gcc $(CFLAGS) -c pconf.c 

print_structs.o:   print_structs.c parse.h globals_gram.h globals_parse.h
	gcc $(CFLAGS) -c print_structs.c 

compile_grammar:  compile_grammar.o dict.o pconf.o grammar.h grammar.o
	gcc  -o compile_grammar compile_grammar.o dict.o pconf.o

compile_grammar.o:   compile_grammar.c 
	gcc $(CFLAGS) -c compile_grammar.c 

concept_leaf:  concept_leaf.o pconf.o grammar.o dict.o
	gcc  -o concept_leaf concept_leaf.o pconf.o grammar.o dict.o

concept_leaf.o:   concept_leaf.c
	gcc $(CFLAGS) -c concept_leaf.c 

train: train.o pconf.o
	gcc  -o train train.o pconf.o

train.o:	train.c pconf.c
	gcc $(CFLAGS) -c train.c

parse_text:  parse_text.o libparse.a
	gcc $(CFLAGS)  -o parse_text parse_text.o libparse.a

parse_text.o:   parse_text.c parse.h
	gcc $(CFLAGS) -c parse_text.c 

nfa_to_dfa:  nfa_to_dfa.o libparse.a
	gcc $(CFLAGS)  -o nfa_to_dfa nfa_to_dfa.o libparse.a

nfa_to_dfa.o:   nfa_to_dfa.c parse.h
	gcc $(CFLAGS) -c nfa_to_dfa.c 

