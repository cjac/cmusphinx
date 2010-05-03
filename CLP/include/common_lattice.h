//-------------------------------------------------------------------------------------------
// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------

// 05/31/2001 changed MAX_VOC_SIZE to a bigger val (200000)  

#ifndef _common_lattice_h
#define _common_lattice_h

#include <map.h>
#include <hash_map.h>
#include <vector.h>
#include <slist.h>
#include <list.h>
#include <string>


typedef unsigned                  LinkId;
typedef unsigned                  NodeId;
typedef unsigned                  WordId;

typedef vector<unsigned>          IdsVector;
typedef vector<IdsVector>         IdsMatrix;

typedef vector<double>            DblVector;
typedef vector<DblVector>         DblMatrix;

typedef string                    Word;

typedef pair<int,double>          IntDbl;
typedef pair<int,int>             IntInt;

typedef list<LinkId>              IdsList;
typedef IdsList::iterator         IdsListIt;
typedef IdsList::const_iterator   IdsListConstIt;


struct ess {
    bool operator() (const IntInt& a, const IntInt& b) const{
	return (a.first < b.first || (a.first == b.first && a.second < b.second));
    }
};

struct compW {
    bool operator() (const IntDbl& a, const IntDbl& b) const{
	return (a.second > b.second || (a.second == b.second && a.first < b.first));
    }
};


typedef map<IntInt, double, ess>  IntIntDblMap; 

typedef IntIntDblMap::iterator    IntIntDblMapIt;

typedef map<IntInt, int, ess>     IntIntIntMap;
typedef IntIntIntMap::iterator    IntIntIntMapIt;

typedef slist<IntDbl>             IntDblList;
typedef IntDblList::iterator      IntDblListIt;
typedef IntDblList::const_iterator IntDblListConstIt;

typedef map<Word,int>             WordIntMap;
typedef WordIntMap::iterator      WordIntMapIt;

typedef map<int,double>           IntDblMap;
typedef IntDblMap::const_iterator IntDblMapConstIt;
typedef IntDblMap::iterator       IntDblMapIt;

typedef map<int,int>              IntIntMap;
typedef IntIntMap::iterator       IntIntMapIt;

typedef vector<Word>              WordVector;
typedef vector<int>               IntVector;
typedef vector<IntDbl>            IntDblVector;

const Word      START_WD               = "!SENT_START";
const Word      END_WD                 = "!SENT_END";
const Word      EPS                    = "!NULL";

const WordId    MAX_NO_WORDS           = 512;  // in a sentence
const WordId    MAX_NO_WORDS_LAT       = 5000; // in a lattice

const unsigned  MAX_LINE               = 1000;   // max no of chars on a line

const float     MAX_TIME               = 1000;   // max end time hypothesized for a word (secs)

const unsigned  MAX_VOC_SIZE           = 200000; // max no of words in the vocabulary
const unsigned  MAX_NO_FIELDS_NODE     = 6;
const unsigned  MIN_NO_FIELDS_NODE     = 1; 

const unsigned  MAX_NO_FIELDS_LINK     = 10;
const unsigned  MIN_NO_FIELDS_LINK     = 3;
 
const unsigned  MAX_NO_FIELDS_INFO_SLF = 20;
const unsigned  MIN_NO_FIELDS_INFO_SLF = 4;

const unsigned  MAX_NO_FIELDS_FSM  = 5;
const unsigned  MIN_NO_FIELDS_FSM  = 1;

const unsigned DEFAULT_NO_PRONS    = 2;


const string    BLANK                   = " \t";


#endif







