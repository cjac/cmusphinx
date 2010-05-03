//-------------------------------------------------------------------------------------------
// Prons.cc  
//-------------------------------------------------------------------------------------------
// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------

#include <cstdlib>
#include <fstream>
#include <string>
#include <cstdio>

#include "Prons.h"
#include "common_lattice.h"
#include "LineSplitter.h"

using namespace std;

Prons::Prons(int s): size(s){};

Prons::Prons(const string& filename)
{
    ifstream f;

    f.open(filename.c_str());
    if(f == NULL) {
	char str[100];
	cerr << "ERROR: ReadProns: The required file " << filename << "does not exist or is corupted !!";
	size = 0;
    }
    else{
	int2word.resize(MAX_VOC_SIZE);
	word_pron.resize(MAX_VOC_SIZE);
	word_no_prons.resize(MAX_VOC_SIZE);
    
	string s;
    
	unsigned widx = 0;

	while(getlineH(f,s)){
	    int pos1 = s.find(";");
	    assert(pos1 != string::npos);
	
	    const string& wp = s.substr(0,pos1); // MOD 6/30/2000 - const added
	    const string& pp = s.substr(pos1+1,string::npos); // MOD 6/30/2000 - const added
	    int no_prons =  atoi(pp.c_str());
	
	    int pos2 = wp.find("=");
	    const string& word = wp.substr(0,pos2); // MOD 6/30/2000 - const added
	
	    string pron;
	    if (pos2+1 != string::npos)
		pron =  wp.substr(pos2+1,string::npos);
	    else
		pron = "";
	
	    word2int[word] = widx;
	
	    int2word[widx] = word;
	    word_pron[widx]= pron;
	    word_no_prons[widx] = no_prons;
	    widx++;
	}
	word2int[EPS] = widx;
	int2word[widx] = EPS;
	word_pron[widx]= "";
	word_no_prons[widx] = 1;

	size = word2int.size();
    
	f.sync();
    }
}


// ////////////////////////////////////////////////////////////////////////////

void
Prons::add_word(const string& w)
{
    word2int[w] = size;
    int2word[size] = w;
    word_no_prons[size] = DEFAULT_NO_PRONS;
    word_pron[size] = "";
    size = word2int.size();
}

// //////////////////////////////////////////////////////////////	
    

int 
Prons::get_idx(const string& w){
    if (word2int.find(w) == word2int.end()){
	cerr << "The word " << w << " could not be found in your dictionary " << endl;
	add_word(w);
    }
    return word2int[w];	
}	

// ////////////////////////////////////////////////////////

ostream& operator<<(ostream& os, const Prons& P)
{
    os << "#" << endl << "#BEGIN no pron" << endl << "#" << endl;
   WordIntHMap& m = (WordIntHMap& )P.word2int; // MOD 6/30/2000 - cast added
    for (WordIntHMapIt ii = m.begin(); ii != m.end(); ii++)
	os << (*ii).first << "\t" << (*ii).second << "\t" << P.int2word[(*ii).second] << "\t" << P.word_pron[(*ii).second] << " no_prons: " << P.word_no_prons[(*ii).second] << endl; 
    return os;
}
