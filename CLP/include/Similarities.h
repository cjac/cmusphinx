//-------------------------------------------------------------------------------------------
// compute different similarities
//-------------------------------------------------------------------------------------------
// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------


#ifndef _Similarities_h
#define _Similarities_h

# include <iostream.h>
# include <fstream.h>
# include <iomanip.h>
# include <stdio.h>
# include <string>
# include <stdlib.h>
# include <math.h>
# include <assert.h> 

#include "LineSplitter.h"

inline int minim(int a, int b);

double compute_phonetic_similarity(const string& A, const string& B);
double compute_word_similarity(const string& A, const string& B);
float compute_time_overlap(float p1, float p2, float q1, float q2);
float compute_time_overlap_ph(float p1, float p2, float q1, float q2);
#endif 






