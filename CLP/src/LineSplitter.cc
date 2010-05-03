// -*- C++ -*-
//-------------------------------------------------------------------------------------------
//  LineSplitter.cc
//-------------------------------------------------------------------------------------------
// Copyright (c) sept 1999      Radu Florian  rflorian@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------

#include "LineSplitter.h"
#include <ctype.h>
#include <string>

void LineSplitter::Split(const string& line) 
{
    int index = -1, length = line.length(), wind=0;
    char prev = ' ', curr;
    noWords = 0;
    const char * temp = line.c_str();
    char word[1000];
    word[0] = '\0';
    bool prev_separator = true;

    while(++index < length) {
	curr = temp[index];
	if(isseparator(curr)) { // we have a separator here..
	  if(prev_separator) {
		if(sep_not_included || isspace(curr))
		  continue;
		word[0] = curr;
		word[1] = '\0';
		wind = 0;
		addWord(word);
	  }
	  else {
		word[wind] = '\0';
		addWord(word);
		wind = 0;
		if(!sep_not_included && !isspace(curr)) {
		  word[0] = curr; word[1] = '\0';
		  wind = 0;
		  addWord(word);
		}
	  }
	  prev_separator = true;
	}
	else {                              // Here the current word is not a separator..
	  word[wind++] = curr;
	  prev_separator = false;
	}
	prev = curr;
  }
  if(wind) {
      word[wind] = '\0';
      addWord(word);
  }    	
  words.resize(noWords);
}
