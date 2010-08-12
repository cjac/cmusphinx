# This file contains a lexer HTK model files in ASCII format.
#
# @author W.J. Maaskant


from ply import *


# Exception class.
class HtkScanningError(Exception):
	pass

# List of token names.
tags = (
	'BEGINHMM',
	'ENDHMM',
	'MEAN',
	'MIXTURE',
	'NUMMIXES',
	'NUMSTATES',
	'STATE',
	'TRANSP',
	'VARIANCE',
	'OTHER_TAG'
	)

macros = (
	'MACRO_H',
	'MACRO_O',
	'MACRO_S',
	'MACRO_T',
	'MACRO_U',
	'MACRO_V',
	'OTHER_MACRO',
	)

tokens = tags + macros + (
	'FLOAT',
	'INT',
	'STRING',
	)

# Lexer definitions.

def t_NEWLINE(t):
	r'\n+'
	t.lexer.lineno += t.value.count("\n")
	
def t_FLOAT(t):
	r'-?\d.\d{6}e[+-]\d{2}'
	t.value = float(t.value)
	return t

def t_INT(t):
	r'\d+'
	t.value = int(t.value)
	return t

def t_MACRO(t):
	r'~[hostuv]'
	type = t.value[1]
	macro = "MACRO_" + type.upper()
	
	if macro in macros:
		t.type = macro
		t.value = type
	else:
		t.type = "OTHER_MACRO"
		t.value = type
	return t

def t_STRING(t):
	r'([a-zA-Z]\w*|"[\w@\-+:]+"|\'[\w@\-+:]+\')'
	if t.value[0] == '"' and t.value[-1] == '"' or t.value[0] == "'" and t.value[-1] == "'":
		t.value = t.value[1:-1]
	return t

def t_TAG(t):
	r'<\w+>'
	tag = t.value[1:-1].upper()
	
	if tag in tags:
		t.type = tag
		t.value = tag
	#elif tag[0:4] == 'MFCC':
	#	t.type = 'MFCC'
	#	t.value = tag[4:-1].upper()
	else:
		t.type = 'OTHER_TAG'
		t.value = tag
	return t

t_ignore = ' \t'

def t_error(t):
	raise HtkScanningError, ("Scanning error. Illegal character '%s' at line %s " % (lexdata[lexpos], t.lexer.lineno), lexdata[lexpos:])

# Build the lexer.
lex.lex()

