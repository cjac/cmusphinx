lib_LIBRARIES = libparse.a
bin_PROGRAMS = compile_grammar concept_leaf parse_text nfa_to_dfa

libparse_a_SOURCES = parse.c match.c grammar.c dict.c pconf.c print_structs.c

compile_grammar_SOURCES = compile_grammar.c dict.c pconf.c

concept_leaf_SOURCES = concept_leaf.c pconf.c grammar.c dict.c

parse_text_SOURCES = parse_text.c
parse_text_LDADD = libparse.a

nfa_to_dfa = nfa_to_dfa.c
nfa_to_dfa_LDADD = libparse.a
