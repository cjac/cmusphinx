//-------------------------------------------------------------------------------------------
// Read the words, their most likely pronunciations and the number of prons they have 
//-------------------------------------------------------------------------------------------
// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------

#ifndef _Prons_h
#define _Prons_h

#include "common.h"
#include "common_lattice.h"
#include <ext/hash_map>

using namespace __gnu_cxx;

struct eqstr
{
    bool operator()(const string s1, const string s2) const
	{
	    return s1 == s2;
	}
};

namespace __gnu_cxx {
template<>
struct hash<string> {
  size_t operator() (string s) const {
        return __stl_hash_string(s.c_str());
  }
};
}



typedef hash_map<string, int, hash<string>, eqstr> WordIntHMap;
typedef WordIntHMap::iterator WordIntHMapIt;

class Prons 
{
 protected:
    WordIntHMap word2int;    // index words
    WordVector int2word;     // the inverse mapping
    IntVector word_no_prons; // the number of pronunciations for words 
    WordVector word_pron;    // the most likely pronunciation for a word
    int size;
    
    void add_word(const string&);
    
 public:
    Prons(int s = 0);
    
    Prons(const string& filename);
    
    int get_idx(const string& w);
    
    const string& get_pron(const int idx) const {return word_pron[idx];}
    
    int get_no_prons(const int idx) const {return word_no_prons[idx];}
    
    Word get_word(const int idx) const {return int2word[idx];}
    
    int Size() const {return size;}
    
    friend ostream& operator << (ostream& os, const Prons& pron);
}; 

#endif


