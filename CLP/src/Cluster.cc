// -*- C++ -*-
//--------------------------------------------------------------------------------------------------------
// Cluster.cc
//
// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//--------------------------------------------------------------------------------------------------------

#include <cstdlib>
#include <iomanip>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <list>

using namespace std;

#include "common_lattice.h"
#include "Link.h"
#include "Cluster.h"
#include "Lattice.h"



Cluster::Cluster():
  lat_ptr((Lattice *)0), cid(0), best_link(-1), min_time(100000), max_time(-1){}

// //////////////////////////////////////////////////////////////////////

Cluster::Cluster(unsigned idx, const Lattice* from_lattice): 
  lat_ptr(from_lattice), cid(idx), best_link(-1), min_time(100000), max_time(-1){}

// //////////////////////////////////////////////////////////////////////

void
Cluster::add_link(const Link& l){
  clinks.push_front(l.Id());

  if (((Lattice* )lat_ptr)->has_time_info()){ // MOD 7/3/2000 - cast added
    if (min_time > l.start_time())
      min_time = l.start_time();
    
    if (max_time < l.end_time())
      max_time = l.end_time();
  }
  else{
    if (min_time > l.start_node().Max_dist())
      min_time = l.start_node().Max_dist();
    
    if (max_time < l.end_node().Max_dist())
      max_time = l.end_node().Max_dist();
  }

  if (best_link == -1)
    best_link = l.Id();
  else
    if (l.Pscore() > lat_ptr->link(best_link).Pscore())
      best_link = l.Id();
}

// //////////////////////////////////////////////////////////////////////

void 
Cluster::fill_cwords()
{
  for (IdsListIt it = clinks.begin(); it != clinks.end(); it++){
    int widx = (lat_ptr->link(*it)).Word_idx();
    IntDblMapIt it1 = cwords.find(widx);

    LnProb linkprob = (lat_ptr->link(*it)).Pscore();
      
    if (it1 == cwords.end()){ 
      cwords[widx] = linkprob;
    }
    else{
      (*it1).second = LogPlus((*it1).second,linkprob);
    }
  }
}

// ////////////////////////////////////////////////////////////////////////

void
Cluster::merge_with(const Cluster& c, const int stage)
{
  copy(c.clinks.begin(), c.clinks.end(),inserter(clinks, clinks.end()));
  if (lat_ptr->link(best_link).Pscore() < lat_ptr->link(c.best_link).Pscore())
    best_link = c.best_link;
  
  if (stage == 2 || stage == 3 || stage == 4){ 
    for (IntDblMapConstIt it = c.cwords.begin(); it != c.cwords.end(); it++){
      IntDblMapIt it1 = cwords.find((*it).first);
      
      if (it1 != cwords.end()){ 
	// this is a common word for the two clusters, add their posterior probabilities
	(*it1).second = LogPlus((*it1).second, (*it).second);
      }
      else
	cwords[(*it).first]=(*it).second;
    }
  }
  if (min_time > c.min_time)
    min_time = c.min_time;
  if (max_time < c.max_time)
    max_time = c.max_time;
}

// ////////////////////////////////////////////////////////////////////////////

bool 
Cluster::match_start_word(float starttime, unsigned widx){
  assert(clinks.size() != 0);
  LinkId id = *(clinks.begin());
  float start = lat_ptr->link(id).start_time();
  unsigned idx = lat_ptr->link(id).Word_idx();
  return (starttime == start && widx == idx);
}	

// ///////////////////////////////////////////////////////////////////////////////


unsigned
Cluster::wordidx(){   
  assert(clinks.size() != 0);
  LinkId id = *(clinks.begin());
  return  lat_ptr->link(id).Word_idx();
}	

// /////////////////////////////////////////////////////////////////////////////////

float
Cluster::end_time(){   
  assert(clinks.size() != 0);
  LinkId id = *(clinks.begin());
  return  lat_ptr->link(id).end_time();
}

// /////////////////////////////////////////////////////////////////////////////////

float
Cluster::start_time(){   
  assert(clinks.size() != 0);
  LinkId id = *(clinks.begin());
  return  lat_ptr->link(id).start_time();
}

// /////////////////////////////////////////////////////////////////////////////////

int
Cluster::approx_end_time(){   
  assert(clinks.size() != 0);
  LinkId id = *(clinks.begin());
  return  lat_ptr->link(id).end_node().Max_dist();
}
	
// /////////////////////////////////////////////////////////////////////////////////

int
Cluster::approx_start_time(){   
  assert(clinks.size() != 0);
  LinkId id = *(clinks.begin());
  return  lat_ptr->link(id).start_node().Max_dist();
}

// /////////////////////////////////////////////////////////////////////////////////

int
Cluster::max_dist(){   
  assert(clinks.size() != 0);
  LinkId id = *(clinks.begin());
  return  lat_ptr->link(id).end_node().Max_dist();
}

// /////////////////////////////////////////////////////////////////////////////////

unsigned
Cluster::compare_less(const Cluster& c, IntIntIntMap& are_less, unsigned MAX_DIST){
  IdsList& cl = (IdsList& )c.clinks; // MOD 6/30/2000 - cast added
  
  for(IdsListIt it1 = clinks.begin(); it1 != clinks.end(); ++it1){
    NodeId firstnode    = lat_ptr->link(*it1).End_node_id();
    for(IdsListIt it2 = cl.begin(); it2 != cl.end(); ++it2){
      NodeId secondnode   = lat_ptr->link(*it1).Start_node_id();
      if (firstnode == secondnode) return 1;
      else {
        if (firstnode > secondnode) 
          continue;
        else{
	  IntIntIntMapIt it = are_less.find(pair<int,int>(firstnode,secondnode));
	  if (it != are_less.end()){
	    if ((*it).second == 1)
	      return 1;
	  }
	  else {
	    if (MAX_DIST > 1){
	      bool answer = ((Lattice* )lat_ptr)->less_nodes(firstnode, secondnode, MAX_DIST); // MOD 7/3/2000 - cast added
	      are_less[pair<int,int>(firstnode,secondnode)] = answer;
	      if (answer)
		return 1;
	    }
	  }
	}
      }
    }
  }
  return 0;
}

 
// ///////////////////////////////////////////////////////////////////

  
int  Cluster::is_less(const Cluster& c, const IntIntIntMap& removed){
  IdsList& cl = (IdsList& )c.clinks; // MOD 6/30/2000 - cast added
  
  for(IdsListIt it1 = clinks.begin(); it1 != clinks.end(); ++it1){
    NodeId endnode1     = lat_ptr->link(*it1).End_node_id();	
    
    for(IdsListIt it2 = cl.begin(); it2 != cl.end(); ++it2){	
      NodeId startnode2 = lat_ptr->link(*it2).Start_node_id();
      
      if (endnode1 == startnode2)
	return 1;
      else if (endnode1 > startnode2)
	continue;
      else if (((IntIntIntMap&) removed)[(IntInt) pair<int,int>(endnode1,startnode2)]) // MOD 7/3/2000 - cast added
	return 1;
    }
  }
  return 0;
}

// ///////////////////////////////////////////////////////////////////////////////////

void
Cluster::print(const Prons& P, bool print_links, bool print_words){
  cout.setf(ios::left, ios::adjustfield);
  cout << "Cluster no " << setw(6)    << cid ;
  cout << "  " << min_time << " -- " << max_time << endl;
  cout << "-----------------\n";
  
  if (print_links){
    if (print_words)
      cout << "Links:" << endl;
    for (IdsListConstIt it = clinks.begin(); it != clinks.end(); it++)
      ((Lattice* )lat_ptr)->link(*it).print_it(P); // MOD 7/3/2000 - cast added
  }
  if (print_words){
    if (print_links)
      cout << "Words:" << endl;	
    for (IntDblMapConstIt it = cwords.begin(); it != cwords.end(); it++){
      if ((*it).first == -1)
	cout << " " << "eps" << "\t" << (*it).second << endl;
      else	
	cout << " " << P.get_word((*it).first) << "\t" << (*it).second << endl;
    }		
  }
  cout.setf(ios::internal, ios::adjustfield);	
}		

// /////////////////////////////////////////////////////////////////////////////////////////////

ostream&
operator<<(ostream& os, const Cluster& c)
{
  os.setf(ios::left, ios::adjustfield);
  os << "Cluster no " << setw(6)    << c.cid << endl;
  
  for (IdsListConstIt it = c.clinks.begin(); it != c.clinks.end(); it++)	
      os << c.lat_ptr->link(*it) << endl;

  for (IntDblMapConstIt it = c.cwords.begin(); it != c.cwords.end(); it++)
    os << (*it).first << setw(20) << (*it).second << endl;
  
  os.setf(ios::internal, ios::adjustfield);
  assert(os.good());
  return os;
}

// ////////////////////////////////////////////////////////////////////////////////////////////
