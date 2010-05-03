//-------------------------------------
//  Node.h stores a lattice node
//-------------------------------------

//------------------------------------------------------------------------------------------
// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//------------------------------------------------------------------------------------------

#ifndef _Node_h
#define _Node_h

#include "common_lattice.h"
#include "Prons.h"

class Node
{
  friend class Lattice;
 public:
  Node();
  Node(const string&, const Lattice*, const Prons&);
  
  const int Word_idx() const {return word_idx;}
  const int Max_dist() const {return max_dist;}
  const unsigned Id() const {return id;}
  void  setId(unsigned i) {id = i;}
  const double Time() const { return time; }
  
  friend ostream& operator<<(ostream& os, const Node& node);
  void print_it(const class Prons &);
  IntIntMap& Outgoing_links() { return outgoing_links; }
  
 private:
  const Lattice*      lat_ptr;  
  NodeId              id;
  
  int                 word_idx;       /* index of the word at this node; -1 when the words are on the links */
  double              time;           /* Node time */ 
  int                 max_dist;       /* the max length of a path from the lattice start node to it
					 to be used in the graphs without time information */
  IntIntMap           outgoing_links; /* nodes that can be reached from this node */
    
  void fill_node_fields(const string&, const string& );
};

#endif


