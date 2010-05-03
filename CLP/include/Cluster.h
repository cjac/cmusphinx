//-------------------------------------------------------------------------------------------
//  Implementation of a cluster (set of links with the corresponding set of words)
//-------------------------------------------------------------------------------------------

// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------

#ifndef _Cluster_h
#define _Cluster_h

#include "common_lattice.h"
#include "Link.h"
#include "Prob.h"
#include "Lattice.h"
#include "Similarities.h"

class Cluster
{
  friend class Clustering;
  
 public:
  Cluster();
  Cluster(unsigned, const Lattice*);
  
  const unsigned Id() const {return cid;}
  void setId(unsigned id) {cid = id;} 
  unsigned Best_link() const {return best_link;}
  unsigned no_links() const {return clinks.size();}
  unsigned no_words() const {return cwords.size();}
  
  // add a link to a cluster
  void add_link(const Link& l);
  
  // returns 1 if the first link in the cluster has the start time = starttime and the word index = widx
  bool match_start_word(float starttime, unsigned widx);
  
  // merge with cluster c  (stage = intra or inter clustering) 
  void merge_with(const Cluster& c, const int stage);
  
  // returns 1 if the two clusters are ordered (there is a link in one class which is less than a link in the second one)
  int is_less(const Cluster& c, const IntIntIntMap& removed);
  
  unsigned compare_less(const Cluster& c, IntIntIntMap& are_less, unsigned MAX_DIST);
      
  // from the list of links obtain the list of words with their corresponding posterior probabilities
  void fill_cwords();	 
  
  // index of the word on the first link in the cluster
  unsigned wordidx();
  
  // ending time of the first link in the cluster
  float end_time();   
  
  // starting time of the first link in the cluster
  float start_time(); 
  
  // approximative ending time of the first link in the cluster (for the lattices with no time info)
  int approx_end_time();   
  
  // starting time of the first link in the cluster
  int approx_start_time(); 
  
  // the length of the longest path from the lattice start node to the end node of the first link in the cluster
  int max_dist();
  
  friend ostream& operator << (ostream& os, const Cluster& c);		
  void print(const Prons& P, bool print_links, bool print_words);
  
  float Min_time() { return min_time;}
  float Max_time(){return max_time;}
  
 private:
  const Lattice*  lat_ptr;  
  unsigned        cid;        /* cluster id */
  
  IdsList         clinks;     /* vector of ids of all the links in the class */
  IntDblMap       cwords;     /* all the words occuring in the cluster (and their post probs) */
  
  int             best_link;  /* the link with the highest posterior in the cluster */
  
  float min_time;             
  float max_time;

};
#endif
