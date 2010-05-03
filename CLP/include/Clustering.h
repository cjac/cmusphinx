//-------------------------------------------------------------------------------------------
// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------

#ifndef _Clustering_h
#define _Clustering_h

#include "Cluster.h"
#include "Lattice.h"
#include "common_lattice.h"
#include "Prons.h"
#include "Similarities.h"

typedef list<Cluster>                ClusterList;
typedef ClusterList::iterator        ClusterListIt; 
typedef ClusterList::const_iterator  ClusterListConstIt;


class Clustering	
{
 public:
  Clustering();
  Clustering(const Lattice*, const Prons&);
  
  int  no_clusters() { return C.size();}
  int  No_links() { return no_links;}
  
  // the number of word types in the list of clusters
  int  no_words();
  
  // add the word lists in each clusters 
  void fill_words();
  
  // sort the clusters in topological order
  void TopSort();
  
  // print the consensus hypothesis and the confusion networks (with the words in decreasing order of their scores)
  void print_sausages_FSMformat(const string& outdir, const string& outfile, const string& infile, float lowEpsT, float highEpsT, float lowWordT, float highWordT, const Prons& P);
  
  // print the list of clusters, specifying either only the links, or only words, or both 
  void print(const Prons& P, bool print_links, bool print_words);
  
  // add a link "eps" to each cluster which has as prob the remaining probability mass (weighted by DELweight)	
  void add_EPS(float DELweight);
  
  // cluster according to stage (intra-word or inter-word), mode (max, avg), in the constrained mode or not 
  void go_cluster(const Prons& P, const int stage, const int mode, const bool constrained);
  
  
 private:
  const Lattice*  lat_ptr;       // the lattice the clusters are to be built from 
  ClusterList   C;               // the list of clusters
  
  IntIntIntMap  constraints;     // the partial order on the clusters
  
  int no_links;                  // the number of links used to build the clusters (left after pruning)
  
  double compute_TIME_sim(ClusterListConstIt it1, ClusterListConstIt it2, IntIntDblMap& linkTimeSim, const int mode, const bool constrained);
  double compute_PHON_sim(ClusterListConstIt cit1, ClusterListConstIt cit2, Prons& P, IntIntDblMap& wordPhSim, const int mode, const bool use_phon_info,const bool constrained );
  void DFS_visit(const unsigned, IdsMatrix& , IdsList& , IdsVector&);
};

#endif
    














