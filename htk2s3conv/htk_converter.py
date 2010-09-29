#!/usr/bin/python
#
# Converts HTK models to Sphinx 3 models.
#
# @author W.J. Maaskant


import re
from struct import unpack, pack
import sys
from sys import exit
import time

from ply import *

import htk_lexer
import htk_parser

from htk_models import *

# Print time + text..
def pr(text):
	lt = time.localtime();
	ft = time.strftime('%d-%m-%Y %H:%M:%S', lt)
	print '%s: %s' % (ft, text)
	sys.stdout.flush()

# Exception raised for HtkConverter related errors.
class HtkConverterError(Exception):
	pass

# Order preserving to uniqify lists.
# Function f5 from http://www.peterbe.com/plog/uniqifiers-benchmark
def unique(seq, idfun=None): 
	if idfun is None:
		def idfun(x): return x
	seen = {}
	result = []
	for item in seq:
		marker = idfun(item)
		if marker in seen: continue
		seen[marker] = 1
		result.append(item)
	return result

class HtkConverter(object):
	# Regular expression objects to recognize monophone and triphone model names.
	monophone_reobj = re.compile(r'^([^-+]+)$')
	triphone_reobj = re.compile(r'^([^-+]+)\-([^-+]+)\+([^-+]+)$')
	
	# A list of (all|monophone|triphone) HMMs.
	hmms = []
	monophoneHmms = []
	triphoneHmms = []
	
	# A dictionary containing a mapping from (all|monophone|triphone) model
	# names to model names, representing the model tying. The model tying is
	# split into monophone and triphone model tyings based on the 'from'
	# name (the new model that is tied to an existing model).
	tiedlist = {}
	monophoneTiedlist = {}
	triphoneTiedlist = {}
	
	# A dictionary containing a mapping from model names to instances of
	# Hmm. Only the unique (physical) models have a mapping (i.e. tied
	# (logical) models should go through tiedlist first).
	namesToHmm = {}
	
	# Dictionaries containing mappings from instances of State/Tmat to id's.
	# This is needed to write the Sphinx 3 model files.
	statesToIds = {}
	tmatsToIds = {}
		
	def __init__(self):
		pass
	
	def load(self, hmmdefs, tiedlist):
		pr('Loading and parsing HTK HMM definitions.')
		
		# Load the HTK HMM defintions file.
		file = open(hmmdefs)
		try:
			data = file.read()
		finally:
			file.close()

		# Parse the HTK HMM definitions file.
		try:
			self.hmms = yacc.parse(data)
		except Exception, e:
			print e
			exit()
		
		# Create a mapping from HMM names to instances of Hmm.
		self.namesToHmms = dict([(hmm.name, hmm) for hmm in self.hmms])
		
		# Split the HMMs in monophone and triphone models.
		self.monophoneHmms = [hmm for hmm in self.hmms if self.monophone_reobj.match(hmm.name)]
		self.triphoneHmms = [hmm for hmm in self.hmms if self.triphone_reobj.match(hmm.name)]
		
		if len(self.monophoneHmms) + len(self.triphoneHmms) != len(self.hmms):
			raise HtkConverterError('Not all HMMs could be classified as either monophone or triphone.')
		
		# Check if a monophone model exists for the base phone of every
		# triphone model.
		for hmm in self.triphoneHmms:
			match = self.triphone_reobj.match(hmm.name)
			if match.group(2) not in self.namesToHmms:
				raise HtkConverterError('The triphone model %s has no corresponding monophone model %s.' % (hmm.name, match.group(2)))
		
		pr('Loading HTK model tying file.')
		
		# Load the HTK model tying file.
		file = open(tiedlist)
		try:
			data = file.readlines()
		finally:
			file.close()
		
		# Create a tied list dictionary.
		for line in data:
			line = line[0:-1].split(' ')
			if len(line) == 2:
				if line[1] in self.namesToHmms: 
					self.tiedlist[line[0]] = line[1]
				else:
					raise HtkConverterError('The model %s should be tied to %s, but that model doesn\'t exist.' % (line[0], line[1]))
			else:

				if line[0] in self.namesToHmms:
					self.tiedlist[line[0]] = line[0]
				else:
					raise HtkConverterError('The model %s appears in tiedlist, but that model doesn\'t exist.' % line[0])
		
		# Split the tied list in monophone and triphone tied lists.
		self.monophoneTiedlist = [(nameFrom, nameTo) for (nameFrom, nameTo) in self.tiedlist.iteritems() if self.monophone_reobj.match(nameFrom)]
		self.triphoneTiedlist = [(nameFrom, nameTo) for (nameFrom, nameTo) in self.tiedlist.iteritems() if self.triphone_reobj.match(nameFrom)]
		
		pr('HTK model files loaded and parsed.')
	
	# Display the loaded HTK models.
	def show(self):
		hmms = self.hmms
		if hmms == None:
			raise HtkConverterError('Please load the HTK models first.')
		
		for hmm in hmms:
			hmm.display()
		
	# Create Sphinx 3 model files.
	def writeS3(self, outprefix = ''):
		hmms = self.hmms
		if hmms == None:
			raise HtkConverterError('Please load the HTK models first.')
		
		self.outprefix = outprefix
		
		# Create a set of all states.
		states = unique([state for hmm in hmms for (iState, state) in hmm.states])
		self.states = states
		
		# Create a set of all transition matrices.
		tmats = unique([hmm.tmat for hmm in hmms])
		self.tmats = tmats

		monophoneStates = unique([state for hmm in self.monophoneHmms for (iState, state) in hmm.states])
		
		# Create the State/Tmat-instance to id mappings.
		for state in monophoneStates:
			self.statesToIds[state] = len(self.statesToIds)
		for state in states:
			if not self.statesToIds.has_key(state):
				self.statesToIds[state] = len(self.statesToIds)
		for tmat in tmats:
			self.tmatsToIds[tmat] = len(self.tmatsToIds)
		
		self._writeS3Mdef(hmms)
		self._writeS3MeanVar(hmms)
		self._writeS3Mixw(hmms)
		self._writeS3Tmat(hmms)
	
	# Create Sphinx 3 model definition file.
	def _writeS3Mdef(self, hmms):
		pr('Writing Sphinx 3 model definitions.')
		
		# Open the output file.
		file = open(self.outprefix + 'mdef', 'wb')
		
		file.write('#\n')
		file.write('# Parameters\n')
		file.write('#\n')
		
		# Write the header.
		file.write('0.3\n')
		
		# n_base: no. of phones (also called 'base' phones) that you have
		file.write('%s n_base\n' % len(self.monophoneTiedlist))
		
		# n_tri: no. of triphones
		file.write('%s n_tri\n' % len(self.triphoneTiedlist))
		
		# n_state_map: Total no. of HMM states (emitting and non-emitting)
		# The Sphinx appends an extra terminal non-emitting state to every
		# HMM, hence for 3 phones, each specified by the user to be modeled
		# by a 3-state HMM, this number will be 3phones*4states = 12
		#
		# Note: I'm not sure which of the following to use:
		file.write('%s n_state_map\n' % (len(self.tiedlist) * (hmms[0].tmat.numStates - 1)))

		# n_tied_state: no. of (emitting) states of all phones after
		# state-sharing is done.
		file.write('%s n_tied_state\n' % len(self.states))

		# n_tied_ci_state: no. of (emitting) states for your 'base' phones
		# after state-sharing is done.
		monophoneStates = unique([state for hmm in self.monophoneHmms for (iState, state) in hmm.states])
		file.write('%s n_tied_ci_state\n' % len(monophoneStates))

		# n_tied_tmat: The HMM for each CI phone has a transition
		# probability matrix associated it. This is the total number of
		# transition matrices for the given set of models.
		#
		# Note: The above is from the CI part of the tutorial. I guess that,
		# looking at the name, this is just the total number of transition
		# matrices.
		file.write('%s n_tied_tmat\n' % len(self.tmats))
		
		# Write the column definitions.
		#
		# base: name of each phone
		#
		# lft: left-context of the phone (- if none)
		#
		# rt: right-context of the phone (- if none)
		#
		# p: position of a triphone (not required at this stage) attrib:
		# attribute of phone. In the phone list, if the phone is "SIL", or
		# if the phone is enclosed by "+", as in "+BANG+", the sphinx
		# understands these phones to be non-speech events. These are also
		# called "filler" phones, and the attribute "filler" is assigned to
		# each such phone. Phones that have no special attributes are
		# labelled as "n/a", standing for "no attribute"
		#
		# tmat: the id of the transition matrix associated with the phone
		#
		# state id's: the ids of the HMM states associated with any phone.
		# This list is terminated by an "N" which stands for a non-emitting
		# state. No id is assigned to it. However, it exists, and is listed.
		
		# Prepare a sorted list of models (first all monophones, then all triphones sorted by base phone).
		tiedlist = []
		for (nameFrom, nameTo) in self.tiedlist.iteritems():
			tmr = self.triphone_reobj.match(nameFrom)
			if tmr:
				tupleFrom = (3,) + tmr.group(2, 1, 3)
			elif self.monophone_reobj.match(nameFrom):
				tupleFrom = (1, nameFrom, '-', '-')
			else:
				raise HtkConverterError('Model name %s in tiedlist could not be classified as either monophone or triphone.' % nameFrom)
			tiedlist.append((tupleFrom, nameTo));
		tiedlist.sort()
		
		file.write('#\n')
		file.write('# Columns definitions\n')
		file.write('#\n')
		file.write('# base lft rt p attrib tmat ... state id\'s ...\n')
		for (tupleFrom, nameTo) in tiedlist:
			if tupleFrom[0] == 1:
				blrp = tupleFrom[1:4] + ('-',)
			elif tupleFrom[0] == 3:
				blrp = tupleFrom[1:4] + ('i',)
			else:
				raise HtkConverterError('Programming error: tupleFrom[0] == %s' % tupleFrom[0])
			
			if tupleFrom[1].lower() != 'sil':
				a = 'n/a'
			else:
				a = 'filler'
			
			hmm = self.namesToHmms[nameTo]
			
			stateIds = tuple([self.statesToIds[state] for (iState, state) in hmm.states])
			
			#         b  l  r  p  a  t       s
			format = '%s\t%s\t%s\t%s\t%s\t%s\t' + '%s\t' * len(stateIds) + 'N\n'
			file.write(format % (blrp + (a, self.tmatsToIds[hmm.tmat]) + stateIds))
		
		file.close()
		
	# Create Sphinx 3 means and variances files.
	def _writeS3MeanVar(self, hmms):
		pr('Writing Sphinx 3 means and variances.')
		
		states = self.states
		
		# Open the output files.
		mfile = open(self.outprefix + 'means', 'wb')
		vfile = open(self.outprefix + 'variances', 'wb')
		
		# Write the headers.
		mfile.write('s3\n')
		mfile.write('version 1.0\n')
		mfile.write('endhdr\n')
		mfile.write(pack('=i', 0x11223344))
		vfile.write('s3\n')
		vfile.write('version 1.0\n')
		vfile.write('endhdr\n')
		vfile.write(pack('=i', 0x11223344))
		
		# Write a four-byte integer containing the number of codebooks
		# (Gaussian mixture models) in the file.
		mfile.write(pack('=I', len(states)))
		vfile.write(pack('=I', len(states)))
		
		# Write a four-byte integer containing the number of feature streams
		# (this is usually 1).
		mfile.write(pack('=I', 1))
		vfile.write(pack('=I', 1))
		
		# Write a four-byte integer containing the number of densities per
		# codebook.
		mfile.write(pack('=I', len(states[0].mixtures)))
		vfile.write(pack('=I', len(states[0].mixtures)))
		
		# For each feature stream, write a four-byte integer containing the
		# dimensionality of that stream.
		mfile.write(pack('=I', 39))
		vfile.write(pack('=I', 39))
		
		# Write a four-byte integer containing the number of 32-bit
		# floating-point numbers in the file. This should be equal to the
		# product of the number of codebooks and densities and the total
		# dimensionality of all streams.
		m = len(states) * 1 * len(states[0].mixtures) * 39
		mfile.write(pack('=I', m))
		vfile.write(pack('=I', m))
		
		# Write a 4-dimensional array of 32-bit floats, with size #tied
		# states x #feature streams x #mixtures per state x vector
		# dimensionality.
		n = 0
		o = 0
		for state in states:
			for (iMixture, mixtureWeight, mixture) in state.mixtures:
				for float in mixture.mean.vector:
					mfile.write(pack('=f', float))
					n += 1
				for float in mixture.var.vector:
					vfile.write(pack('=f', float))
					o += 1
		if m != n:
			raise HtkConverterError('The number of floats witten in means should be %s, but was %s.' % (m, n))
		if m != o:
			raise HtkConverterError('The number of floats witten in vars should be %s, but was %s.' % (m, o))
		
		mfile.close()
		vfile.close()
		
	# Create Sphinx 3 mixture weights file.
	def _writeS3Mixw(self, hmms):
		pr('Writing Sphinx 3 mixture weights.')
		
		states = self.states
		
		# Open the output file.
		file = open(self.outprefix + 'mixture_weights', 'wb')
		
		# Write the header.
		file.write('s3\n')
		file.write('version 1.0\n')
		file.write('endhdr\n')
		file.write(pack('=i', 0x11223344))
		
		# Write a four-byte integer containing the number of state output
		# distributions (senones, GMMs).
		file.write(pack('=I', len(states)))
		
		# Write a four-byte integer containing the number of feature
		# streams.
		file.write(pack('=I', 1))
		
		# Write a four-byte integer containing the number of densities per
		# mixture.
		file.write(pack('=I', len(states[0].mixtures)))
		
		# Write a four-byte integer containing the number of 32-bit
		# floating-point numbers in the file.
		m = len(states) * 1 * len(states[0].mixtures)
		file.write(pack('=I', m))
		
		# Write a 3-dimensional array of 32-bit floats, with size #tied
		# #states x feature streams x #mixtures per state.
		n = 0
		for state in states:
			for (iMixture, mixtureWeight, mixture) in state.mixtures:
				file.write(pack('=f', mixtureWeight))
				n += 1
		if m != n:
			raise HtkConverterError('The number of floats witten in mixw should be %s, but was %s.' % (m, n))
		
		file.close()
		
	# Create Sphinx 3 transition matrices file.
	def _writeS3Tmat(self, hmms):
		pr('Writing Sphinx 3 transition matrices.')
		
		tmats = self.tmats
		
		# Open the output file.
		file = open(self.outprefix + 'transition_matrices', 'wb')
		
		# Write the header.
		file.write('s3\n')
		file.write('version 1.0\n')
		file.write('endhdr\n')
		file.write(pack('=i', 0x11223344))
		
		# Write a four-byte integer containing the number of transition
		# matrices.
		file.write(pack('=I', len(tmats)))
		
		# Write a four-byte integer containing the number of emitting states
		# per phone.
		file.write(pack('=I', tmats[0].numStates - 2))
		
		# Write a four-byte integer containing the total number of states
		# per phone.
		file.write(pack('=I', tmats[0].numStates - 1))
		
		# Write a four-byte integer containing the number of 32-bit
		# floating-point numbers in the file.
		m = len(tmats) * (tmats[0].numStates - 2) * (tmats[0].numStates - 1)
		file.write(pack('=I', m))
		
		# Write a 3-dimensional array of 32-bit floats, with size #tmats x
		# #emitting states x #states.
		#
		# This means that for each tmat, the first and last row and the
		# first column are not written.
		n = 0
		for tmat in tmats:
			for o in range(1, tmat.numStates - 1):
				for i in range(o * tmat.numStates + 1, (o + 1) * tmat.numStates):
					file.write(pack('=f', tmat.vector[i]))
					n += 1
		if m != n:
			raise HtkConverterError('The number of floats witten in tmat should be %s, but was %s.' % (m, n))
		
		file.close()
