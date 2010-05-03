// -------------------------------------------------------------------------------------------------------------
// LatticeInfo.h 
// Stores the general information found in the lattice header for SLF lattices or whatever it can get from FSMs
// -------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
// Copyright (c) nov 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------


#ifndef _LatticeInfo_h
#define _LatticeInfo_h

#include "Prob.h"
#include "common_lattice.h"
#include <string>


class LatticeInfo
{
  friend class Lattice;
  
 public:
  LatticeInfo(const string& infile, const string& graph_type);
  
  double Wdpenalty() const {return wdpenalty;}
  
  double LMscale() const {return lmscale;}
  
  double PRscale() const {return prscale;}
  
  LnProb No_links() const {return no_links;}
  
  LnProb No_nodes() const {return no_nodes;}
  
  const string& Utt() const {return utterance;}
  
  const string& Filename() const {return file_name;}
  
  friend ostream&   operator<<(ostream& os, const LatticeInfo& info);
  
 private:
  string   file_name;            /* lattice file name */
  string   type;                 /* "FSM" or "SLF" depending on the input file */
  
  string   utterance;            /* the utterance label */
  
  unsigned        no_nodes;      /* no of nodes in the lattice */
  unsigned        no_links;      /* no of links in the lattice */
  
  double          lmscale;       /* the language model scale */
  double          prscale;       /* the pronunciation model scale */
  double          wdpenalty;     /* word insertion penalty */
  
  void fill_info_fields(const string& idf, const string& entry);
};

#endif
