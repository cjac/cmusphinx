# This file contains a parser for HTK model files in ASCII format.
#
# @author W.J. Maaskant


from ply import *

import htk_lexer
tokens = htk_lexer.tokens

from htk_models import *

# Exception class.
class HtkParseError(Exception):
	def __init__(self, p, message):
		Exception.__init__(self, message + ' at line %s.' % p.lineno(1))

# Place to store macros that can be used inside other definitions.
macros = {
	's': {}, # state
	't': {}, # tmat
	'u': {}, # mean
	'v': {}, # var
	}

# Grammar definition.
def p_macros(p):
	'''
	macros : macro macros
	       | macro
	'''
	if isinstance(p[1], Hmm):
		if len(p) == 3:
			p[0] = [p[1]] + p[2]
		else:
			p[0] = [p[1]]
	else:
		if len(p) == 3:
			p[0] = p[2]
		else:
			p[0] = []

def p_macro(p):
	'''
	macro : MACRO_H STRING hmmdef
		  | MACRO_O globaloptions
		  | MACRO_S STRING state
		  | MACRO_T STRING tmat
		  | MACRO_U STRING mean
		  | MACRO_V STRING var
	'''
	if p[1] == 'h':
		p[3].name = p[2]
		p[0] = p[3]
		pass
	elif p[1] == 'o':
		pass
	elif p[1] == 's':
		macros['s'][p[2]] = p[3]
	elif p[1] == 't':
		macros['t'][p[2]] = p[3]
	elif p[1] == 'u':
		macros['u'][p[2]] = p[3]
	elif p[1] == 'v':
		macros['v'][p[2]] = p[3]

def p_hmmdef(p):
	'''
	hmmdef : BEGINHMM NUMSTATES INT hmmdef_states hmmdef_tmat ENDHMM
	'''
	if p[3] != len(p[4]) + 2:
		raise HtkParseError(p, 'The specified (%s) and actual (%s + 2) number of states don\'t correspond' % (p[3], len(p[4]) + 2))
	p[0] = Hmm(p[4], p[5])

def p_hmmdef_states(p):
	'''
	hmmdef_states : STATE INT hmmdef_state hmmdef_states
	              | STATE INT hmmdef_state
	'''
	if len(p) == 5:
		p[0] = [(p[2], p[3])] + p[4]
	else:
		p[0] = [(p[2], p[3])]
	
def p_hmmdef_state(p):
	'''
	hmmdef_state : state
				 | MACRO_S STRING
	'''
	if len(p) == 2:
		p[0] = p[1]
	else:
		if p[2] in macros['s']:
			p[0] = macros['s'][p[2]]
		else:
			raise HtkParseError(p, 'The macro %s is not defined' % p[2])

def p_hmmdef_tmat(p):
	'''
	hmmdef_tmat : tmat
				| MACRO_T STRING
	'''
	if len(p) == 2:
		p[0] = p[1]
	else:
		if p[2] in macros['t']:
			p[0] = macros['t'][p[2]]
		else:
			raise HtkParseError(p, 'The macro %s is not defined' % p[2])

def p_globaloptions(p):
	'''
	globaloptions : filler
	'''
	pass

def p_state_1(p):
	'''
	state : NUMMIXES INT state_mixtures
	'''
	if p[2] != len(p[3]):
		raise HtkParseError(p, 'The specified (%s) and actual (%s) number of mixtures don\'t correspond' % (p[2], len(p[3])))
	p[0] = State(p[3])
	
def p_state_mixtures(p):
	'''
	state_mixtures : MIXTURE INT FLOAT state_mean state_var filler state_mixtures
				   | MIXTURE INT FLOAT state_mean state_var filler
	'''
	if len(p) == 8:
		p[0] = [(p[2], p[3], Mixture(p[4], p[5]))] + p[7]
	else:
		p[0] = [(p[2], p[3], Mixture(p[4], p[5]))]

def p_state_2(p):
	'''
	state : state_mean state_var filler
	'''
	p[0] = State([(1, 1.0, Mixture(p[1], p[2]))])

def p_state_mean(p):
	'''
	state_mean : mean
			   | MACRO_U STRING
	'''
	if len(p) == 2:
		p[0] = p[1]
	else:
		if p[2] in macros['u']:
			p[0] = macros['u'][p[2]]
		else:
			raise HtkParseError(p, 'The macro %s is not defined' % p[2])

def p_state_var(p):
	'''
	state_var : var
			  | MACRO_V STRING
	'''
	if len(p) == 2:
		p[0] = p[1]
	else:
		if p[2] in macros['v']:
			p[0] = macros['v'][p[2]]
		else:
			raise HtkParseError(p, 'The macro %s is not defined' % p[2])

def p_tmat(p):
	'''
	tmat : TRANSP INT vector
	'''
	if p[2]**2 != len(p[3]):
		raise HtkParseError(p, 'The number of states (%s) doesn\'t correspond to the vector size (%s)' % (p[2], len(p[3])))
	p[0] = Tmat(p[3])

def p_mean(p):
	'''
	mean : MEAN INT vector
	'''
	if p[2] != len(p[3]):
		raise HtkParseError(p, 'The specified (%s) and actual (%s) vector size don\'t correspond' % (p[2], len(p[3])))
	p[0] = Mean(p[3])

def p_var(p):
	'''
	var : VARIANCE INT vector
	'''
	if p[2] != len(p[3]):
		raise HtkParseError(p, 'The specified (%s) and actual (%s) vector size don\'t correspond' % (p[2], len(p[3])))
	p[0] = Var(p[3])

def p_vector(p):
	'''
	vector : FLOAT vector
		   | FLOAT
	'''
	if len(p) == 3:
		p[0] = [p[1]] + p[2]
	else:
		p[0] = [p[1]]

def p_filler(p):
	'''
	filler : OTHER_TAG filler1 filler1 filler1 filler1 filler1 filler1 filler1
		   | OTHER_TAG filler1 filler1 filler1 filler1 filler1 filler1
		   | OTHER_TAG filler1 filler1 filler1 filler1 filler1
		   | OTHER_TAG filler1 filler1 filler1 filler1
		   | OTHER_TAG filler1 filler1 filler1
		   | OTHER_TAG filler1 filler1
		   | OTHER_TAG filler1
		   |
	'''
	pass

def p_filler1(p):
	'''
	filler1 : INT
			| STRING
			| FLOAT
			| OTHER_TAG
	'''
	pass
	
def p_error(p):
	print 'Syntax error at "%s" (line %s)' % (p.value, p.lineno)


# Build the parser.
yacc.yacc()
