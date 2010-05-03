#!/usr/bin/env python

# Copyright (c) 2010  Carnegie Mellon University
#
# You may copy and modify this freely under the same terms as
# Sphinx-III

"""
Convert sausages from confusion network code to OpenFST files
"""

__author__ = "David Huggins-Daines <dhuggins@cs.cmu.edu>"
__version__ = "$Revision: 10058 $"

import openfst
import collections
import fileinput
import itertools
import math
import sys

def fstcompile(infile):
    fst = openfst.StdVectorFst()
    symtab = openfst.SymbolTable("symbols")
    symtab.AddSymbol("&epsilon;")
    statemap = collections.defaultdict(fst.AddState)
    for spam in infile:
        fields = spam.strip().split()
        if len(fields) == 1:
            fst.SetFinal(int(fields[0]), 0)
        elif len(fields) == 2:
            fst.SetFinal(int(fields[0]), float(fields[1]))
        elif len(fields) > 2:
            if len(fields) > 3:
                prob = float(fields[3])
            else:
                prob = 1.0
            if fields[2] == 'eps':
                fields[2] = '&epsilon;'
            sym = symtab.AddSymbol(fields[2])
            src = statemap[fields[0]]
            dest = statemap[fields[1]]
            fst.AddArc(src, sym, sym, -math.log(prob), dest)
    fst.SetStart(0)
    fst.SetInputSymbols(symtab)
    fst.SetOutputSymbols(symtab)
    return fst

if __name__ == "__main__":
    from optparse import OptionParser
    optparse = OptionParser(usage="%prog [SAUSAGE]")
    optparse.add_option("-o", "--outfile")
    opts, args = optparse.parse_args(sys.argv[1:])
    fst = fstcompile(fileinput.input(args))
    if opts.outfile:
        fst.Write(opts.outfile)
