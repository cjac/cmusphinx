//-------------------------------------------------------------------------------------------
// Copyright (c) nov 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------

#ifndef _Lattice_h
#define _Lattice_h

#include "zio.h"
#include "common.h"
#include "Prob.h"
#include "Prons.h"
#include "common_lattice.h"
#include "Link.h"
#include "Node.h"
#include "LatticeInfo.h"

#include <string>
#include <map>
#include <vector>

using namespace std;

class Lattice
{
 public:
  Lattice(const LatticeInfo& info, const Prons& P);           
  
  LnProb insertion_penalty() {return info.Wdpenalty();}
  LnProb lm_scale() {return info.LMscale();}
  LnProb pr_scale() {return info.PRscale();}
  LnProb Total_prob() {return total_prob;}
  string Type() const {return info.type;}
  bool has_time_info() { return time_info;}
  void set_no_time_info(){ time_info = false;}
  
  int no_nodes() {assert(info.No_nodes() == nodes.size()); return nodes.size();}
  int no_links() {assert(info.No_links() == links.size()); return links.size();}
  
  // returns the number of word types in the lattice
  int no_words(); 
  
  // get a node knowing its index
  const Node& node(NodeId id) const{ assert(id < nodes.size()); return nodes[id];}
  Node& node(NodeId id) { assert(id < nodes.size()); return nodes[id];}
  
  // get a link  knowing its index
  const Link& link(LinkId id) const{ assert(id < links.size()); return links[id];}
  Link& link(LinkId id) { assert(id < links.size()); return links[id];}
  
  //scale the links  
  void scale_link_scores(const float scale);
  
  // add pronunciation probabilities
  void add_prons(const Prons& P, const float PRweight);
  
  // add/substract word insertion penalty
  void add_WIP(const float WIP);
  
  // compute links probabilities as a combination of LM, AC and PM scores  
  void compute_link_scores(const float lmscale, const float prscale, const float WIP, const float allscale);
  
  // see if the nodes ids are in increasing order as a function of time
  bool check_nodes();
  
  // check if the ending nodes of the links are in increasing order as a function of time
  // this warranties that the links are in topological order
  bool check_links();
  
  // if the lattice doesn't have pronunciation probs (field r=...), put uniform probabilities; P contains the number of prons for each word
  unsigned put_uniform_pron_prob(const Prons& P);       
  
  // sort the links based on their posterior probs; we need this if we want to keep x% of the links and discard the others (-t option)
  void mark_pruned_percentage(const float thresh);
  void mark_pruned_score(const float thresh);
  
  void do_ForwardBackward();
  
  // when there is no time info, mark each node with the length of the longest path from the initial node to it; 
  // use this information for initializing the clusters and constraining the merging
  void put_max_dist(const Prons& P);
  
  friend ostream& operator << (ostream& os, const Lattice& lat);
  
  bool less_nodes(NodeId id1, NodeId id2, unsigned MAX_DIST);
  
 private:
  LatticeInfo     info;       /* the information in the header */
  
  vector<Node>    nodes;      /* vector[0..info.no_nodes-1] of nodes */  
  vector<Link>    links;      /* vector[0..info.no_links-1] of links */
  
  LnProb          total_prob; /* the sum of the posteriors of all the paths in the lattice */     
  bool            time_info;  /* 1 if the lattice has time information */
  
  // find the children of all the nodes 
  void fill_outgoing_links();
  
  // sort the links in topological order; needed for the Forward-Backward step
  void do_TopSort();
  
  // visit the graph in the depth-first manner
  void DFS_visit(int nodeid, IdsList& l, vector<int>& color);
  
};  	  


#endif

