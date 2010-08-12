# This file contains classes to represent the HTK model structure.
#
# @author W.J. Maaskant


import operator

class HtkModelError(Exception):
	pass

# TODO:	It would be nice to parse the HMM name here, set an attribute
# indicating the 'gramness' (n=1, n=3) and splitting the name if n=3.
class Hmm:
	# @param states A list containing 2-tuples, consisting of the state index and an instance of State.
	# @param tmat An instance of Tmat.
	def __init__(self, states, tmat):
		if states == None:
			raise HtkModelError('Parameter states is None.')
		if tmat == None:
			raise HtkModelError('Parameter tmat is None.')

		# Sort the state list by state index.
		states.sort(key=operator.itemgetter(0))

		self.states = states
		self.tmat = tmat
	
	def display(self):
		print self.name
		for (iState, state) in self.states:
			print '\tState %s' % iState
			state.display()
		print "\tTmat"
		self.tmat.display()

class Mean:
	# @param vector A list containing floats.
	def __init__(self, vector):
		if vector == None:
			raise HtkModelError('Parameter vector is None.')
		self.vector = vector
	
	def display(self):
		print '\t\t\t\t[%s] %s' % (len(self.vector), ' '.join([str(f) for f in self.vector]))
		
class Mixture:
	# @param mean An instance of Mean.
	# @param var An instance of Var.
	def __init__(self, mean, var):
		if mean == None:
			raise HtkModelError('Parameter mean is None.')
		if var == None:
			raise HtkModelError('Parameter var is None.')
		self.mean = mean
		self.var = var
	
	def display(self):
		print '\t\t\tMean'
		self.mean.display()
		print '\t\t\tVar'
		self.var.display()

class State:
	# @param mixtures A list containing 3-tuples, consisting of the mixture index, mixture weight and an instance of Mixture.
	def __init__(self, mixtures):
		if mixtures == None:
			raise HtkModelError('Parameter mixtures is None.')
		
		# Sort the mixture list by mixture index.
		mixtures.sort(key=operator.itemgetter(0))
		
		self.mixtures = mixtures

	def display(self):
		for (iMixture, mixtureWeight, mixture) in self.mixtures:
			print '\t\tMixture %s (%s)' % (iMixture, mixtureWeight)
			mixture.display()

class Tmat:
	# @param vector A list containing floats.
	def __init__(self, vector):
		if vector == None:
			raise HtkModelError('Parameter vector is None.')
		self.numStates = int(len(vector)**0.5)
		self.vector = vector
	
	def display(self):
		for m in range(self.numStates):
			print '\t\t%s' % ' '.join([str(self.vector[i]) for i in range(m * self.numStates, (m + 1) * self.numStates)])

class Var:
	# @param vector A list containing floats.
	def __init__(self, vector):
		if vector == None:
			raise HtkModelError('Parameter vector is None.')
		self.vector = vector
	
	def display(self):
		print '\t\t\t\t[%s] %s' % (len(self.vector), ' '.join([str(f) for f in self.vector]))
