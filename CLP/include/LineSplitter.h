//-------------------------------------------------------------------------------------------
// Split a line using different separators
//-------------------------------------------------------------------------------------------
// Copyright (c) sept 1999      Radu Florian  rflorian@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------


#ifndef _LineSplitter_h_
#define _LineSplitter_h_
#include <stdlib.h>
#include <string>
#include <vector.h>

class LineSplitter {
protected:
  unsigned int noWords;
  vector<string> words;
  string separators;
  // if sep_not_included is true, the separators are not included in the result. Otherwise, separators that
  // are not white space are included.
  bool sep_not_included;
  char seps[256];
public:

  // Constructor : the argument is a string of separator chars - default space and tab
  LineSplitter(const string& separats = " \t", bool s=true) :  noWords(1000), words(noWords), separators(separats), sep_not_included(s)
    {
	  for(int i=0 ; i<256 ; i++)
		seps[i] = 0;
	  
	  for(unsigned i=0 ; i<separators.size() ; i++)
		seps[separators[i]] = 1;
    }

  ~LineSplitter() {
  }

  bool isseparator(char c) {
	return seps[c] == 1;
  }

  // splits the line
  void Split(const string& line);
  
  const string& operator[] (int i) const {
#ifdef _DEBUG_
	assert(i>0 && i<nowords);
#endif
    return words[i];
  }

  // returns the i-th word in the line
  string& operator[] (int i) {
#ifdef _DEBUG_
	assert(i>0 && i<nowords);
#endif
    return words[i];
  }

  // returns the number of words in the split line
  int NoWords() {
    return noWords;
  }

  // returns a reference to the vector of words in the split line
  vector<string>& GetWords() {
    return words;
  }

protected:
  void addWord(char word[]) {
	if(words.size() <= noWords)
	  words.push_back(word);
	else
	  words[noWords] = word;
	noWords++;
  }
};

#endif                // ! _LineSplitter_h_
