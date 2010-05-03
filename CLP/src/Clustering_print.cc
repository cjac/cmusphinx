//--------------------------------------------
// Clustering.cc 
// Implementation of the clustering procedure 
//--------------------------------------------
// Copyright (c) sept 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//--------------------------------------------------------------------------------------------------------

#include <cstdlib>
#include <iomanip>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <list>

using namespace std;

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "common_lattice.h"
#include "Link.h"
#include "Clustering.h"
#include "Prons.h"
#include "Similarities.h"


inline double maximf(double a, double b){
    if (a>b) return a; else return b;
}

inline double averagef(double a, double b, int m, int n){ 
    return (m*a + b*n)/(m+n); 
}

Clustering::Clustering():
    lat_ptr((Lattice *)0) {}


Clustering::Clustering(const Lattice* from_lattice, const Prons& P): lat_ptr(from_lattice){
       
  IntIntIntMap removed;
  assert(removed.empty());
  assert(C.empty());
  
  unsigned E = ((Lattice* )lat_ptr)->no_links(); // MOD 7/3/2000 - cast added
  unsigned N = ((Lattice* )lat_ptr)->no_nodes(); // MOD 7/3/2000 - cast added
  
  bool use_time_info =  ((Lattice* )lat_ptr)->has_time_info(); // MOD 7/3/2000 - cast added
  
  int EPS_idx = ((Prons&) P).get_idx(EPS); // MOD 6/30/2000 - cast added
  int START_WD_idx = ((Prons&) P).get_idx(START_WD); // MOD 6/30/2000 - cast added
  int END_WD_idx = ((Prons&) P).get_idx(END_WD); // MOD 6/30/2000 - cast added
  
  ClusterListIt before_end;
  
  int count_links_kept = 0;
  
  for (unsigned i = 0; i < E; i++){
    Link& curr_link = (Link &)lat_ptr->link(i); // MOD 6/30/2000 - cast added
    
    if (curr_link.is_pruned() == 0){
      
      count_links_kept++;
      bool found = false;
      int widx = curr_link.Word_idx();
      
      if (widx != END_WD_idx && widx != EPS_idx && widx != START_WD_idx){ 
	// don't form clusters for !SENT_START, !SENT_END and !NULL 
	
	if (C.begin() != C.end()){
	  before_end = C.end();
	  before_end--;
	  
	  if (use_time_info){
	    float starttime = curr_link.start_time();
	    float endtime   = curr_link.end_time();
	    assert (starttime >=0 && endtime >=0);
	    
	    if (before_end == C.begin()){
	      if (endtime == (*before_end).end_time() && (*before_end).match_start_word(starttime,widx)){
		(*before_end).add_link(curr_link);
		found = true;
	      }
	    }
	    else {
	      for (ClusterListIt it = before_end; it != C.begin(); --it){
		if (endtime == (*it).end_time()){
		  if ((*it).match_start_word(starttime,widx)){
		    (*it).add_link(curr_link);
		    found = true;
		    break;
		  }
		}	
		else
		  break;
	      }
	    }
	  }
	  else{
	    int approx_start_time = curr_link.start_node().Max_dist();
	    int approx_end_time = curr_link.end_node().Max_dist();
	    
	    for (ClusterListIt it = C.begin(); it != C.end(); it++){
	      if (widx == (*it).wordidx() && approx_start_time == (*it).approx_start_time() && approx_end_time == (*it).approx_end_time()){
		(*it).add_link(curr_link);
		found = true;
		break;
	      }
	    }
	  }
	}
	if (found == false){	
	  Cluster c(C.size(),lat_ptr);
	  c.add_link(curr_link);
	  C.insert(C.end(),c);
	}		
      }
      else
	if (widx == EPS_idx)
	  removed[pair<int,int>(curr_link.Start_node_id(), curr_link.End_node_id())] = 1;
    }
    else
      removed[pair<int,int>(curr_link.Start_node_id(), curr_link.End_node_id())] = 1;
  }
  no_links = count_links_kept;
    
  unsigned no_clusters = C.size();
  
  IdsMatrix ordered;
  ordered.resize(no_clusters);	
    
  for (int i = 0; i < no_clusters; i++)
    ordered[i].resize(no_clusters);
  
  for (int i = 0; i < no_clusters; i++)
    for (int j = 0; j < no_clusters; j++)
      assert(ordered[i][j] == 0);
  
  // record the partial constrains
  if (use_time_info){
    for (ClusterListConstIt it1 = C.begin(); it1 != C.end(); it1++){
      unsigned id1 = (*it1).Id();	
	    	    
	    ClusterListConstIt it2 = it1;
	    it2++;
	    
	    while(it2 != C.end()){
	      unsigned id2 = (*it2).Id();
	      assert(id1 < id2);
				
	      if ( ((Cluster& )(*it1)).end_time() <= ((Cluster& )(*it2)).start_time()){ // MOD 7/3/2000 - cast added
		int answer =  ((Cluster& )(*it1)).is_less(*it2, removed); // MOD 7/3/2000 - cast added
		if (answer >=1) { // MOD 7/3/2000 - added "{"
		  //cout << id1 << " and " << id2 << "->" << answer << endl;
		  ordered[id1][id2] = 1;
		} // MOD 7/3/2000 - added "}"
	      }	
	      it2++;
	    }	
    }	
  }
  else{	
    for (ClusterListConstIt it1 = C.begin(); it1 != C.end(); it1++){
      unsigned id1 = (*it1).Id();	
      ClusterListConstIt it2 = C.begin();
      
      while(it2 != C.end()){
	unsigned id2 = (*it2).Id();
	if (id1 != id2){
	  int answer = ((Cluster& )(*it1)).is_less(*it2, removed); // MOD 7/3/2000 - cast added
	  if (answer>=1){
	    cout << id1 << " and " << id2 << "->" << answer << endl;
	    ordered[id1][id2] = 1;
	  }
	}
	it2++;
      }
    }
  }
  
  // do the transitive closure
  if (use_time_info){
    for (int i = 0; i < no_clusters; i++)
      for (int j = i+1; j < no_clusters; j++)
	for (int k = i+1; k < j; k++)
	  if (ordered[i][k] == 1 && ordered[k][j] == 1) ordered[i][j] = 1;
  }
  else{	
    for (int i = 0; i < no_clusters; i++)
      for (int j = 0; j < no_clusters; j++)
	for (int k = 0; k < no_clusters; k++)
	  if (ordered[i][k] == 1 && ordered[k][j] == 1) ordered[i][j] = 1;	
  }
  
  assert(constraints.empty());
    
  // record the partial order
  for (int i = 0; i < no_clusters; i++)	
    for (int j = 0; j < no_clusters; j++)
      if (ordered[i][j]){
	constraints[pair<int,int>(i,j)]=1;  
      }
}	


// ////////////////////////////////////////////////////////////////////////////////

void
Clustering::print(const Prons& P, bool print_links, bool print_words){
    for (ClusterListIt it = C.begin(); it != C.end(); it++){
	(*it).print(P, print_links, print_words); 
	cout << endl;	
    }	
}	
    
// ////////////////////////////////////////////////////////////////////////////////

void
Clustering::fill_words(){
    for (ClusterListConstIt it = C.begin(); it != C.end(); it++)
     ((Cluster& )(*it)).fill_cwords();  // MOD 7/3/2000 - cast added
    return; // MOD 6/30/2000 - 1 removed
}

// //////////////////////////////////////////////////////////////////////////////////

void
Clustering::go_cluster(const Prons& P, const int stage, const int mode, const bool constrained)
{
  static IntIntDblMap linkTimeSim;
  assert(linkTimeSim.empty());
  static IntIntDblMap wordPhSim;
 
  static IntIntIntMap are_less;
 
  unsigned no_clusters = C.size();
  
  vector<ClusterListIt> most_similar(no_clusters); // for each cluster keeps a pointer to the most_similar cluster to it
                                                   // the actual similarity values are to be found in 'ordered'
    
  for (int i = 0; i<no_clusters; i++)
      most_similar[i] = C.end();
  
  DblMatrix SIM;	
  SIM.resize(no_clusters);
  for (int i = 0; i < no_clusters; i++)
    SIM[i].resize(no_clusters);
  
  for (int i = 0; i < no_clusters; i++)
    for (int j = 0; j < no_clusters; j++)
      assert(SIM[i][j] == 0);
  
  for (IntIntIntMapIt it = constraints.begin(); it!= constraints.end(); ++it){
    unsigned firstnode = (*it).first.first;
    unsigned secondnode = (*it).first.second;
    assert(firstnode < no_clusters && secondnode < no_clusters);
    SIM[firstnode][secondnode] = -1;
  }
  
  for (int i = 0; i < no_clusters; i++)
    SIM[i][i] = -1;
        
    // add in SIM the similarity values between the compatible clusters  
    
    for (ClusterListIt it1 = C.begin(); it1 != C.end(); it1++){
	unsigned id1 = (*it1).Id();
	
	double max_sim_row = 0;
	ClusterListIt itmax_row=C.end();
	
	for (ClusterListIt it2 = C.begin(); it2 != C.end(); it2++){
	    unsigned id2 = (*it2).Id();
	    double sim = 0;

	     if (id1 < id2){
	       if (stage == 1){
		 if (SIM[id1][id2] == 0 && SIM[id2][id1] == 0)
		   if ((*it1).wordidx() == (*it2).wordidx())
		     sim = compute_TIME_sim(it1,it2,linkTimeSim,mode,constrained);
	       }		
	       else if (stage == 2){        // inter-word clustering w/ phonetic similarity
		 if (SIM[id1][id2] == 0 && SIM[id2][id1] == 0){
		   sim = compute_PHON_sim(it1,it2, (Prons&) P,wordPhSim,mode,1, constrained); // MOD 7/3/2000 - cast added
		}
	      }	
	      else if (stage == 3){       // inter-word clustering w/o phonetic similarity
		if (SIM[id1][id2] == 0 && SIM[id2][id1] == 0){
		  sim = compute_PHON_sim(it1,it2, (Prons&) P,wordPhSim,mode,0, constrained); // MOD 7/3/2000 - cast added
		}
	      }
	      else if (stage == 4){      // the last clustering stage
		if (SIM[id1][id2] == 0 && SIM[id2][id1] == 0){
		  if ((*it1).Max_time() < (*it2).Min_time()){
		    if( ((Lattice* )lat_ptr)->has_time_info() && ((*it2).Min_time() - (*it1).Max_time() > 0.5)){ // MOD 7/3/2000 - cast added
		      SIM[id1][id2] = -1;
		      sim = -1;
		    }
		    else if ( ((Lattice* )lat_ptr)->has_time_info() == 0 && ((*it2).Min_time() - (*it1).Max_time() > 20)){ // MOD 7/3/2000 - cast added
		      SIM[id1][id2] = -1;
		      sim = -1;
		    }
		    else{
		      unsigned order = (*it1).compare_less(*it2,are_less,10);
		      if (order == 1){
			SIM[id1][id2] = -1;
			sim = -1;
		      }
		      else{
			sim = compute_PHON_sim(it1,it2, (Prons&)P,wordPhSim,mode,0, constrained); // MOD 7/3/2000 - cast added
		      }
		    }
		  }
		  else{
		    if( ((Lattice* )lat_ptr)->has_time_info() && ((*it1).Min_time() - (*it2).Max_time() > 0.5)){ // MOD 7/3/2000 - cast added
		      SIM[id2][id1] = -1;
		      sim = -1;
		    }
		    else if (((Lattice* )lat_ptr)->has_time_info() == 0 && ((*it1).Min_time() - (*it2).Max_time() > 20)){ // MOD 7/3/2000 - cast added
		      SIM[id2][id1] = -1;
		      sim = -1;
		    }
		    else{
		      unsigned order = (*it2).compare_less(*it1, are_less, 10);
		      if (order == 1){
			SIM[id2][id1] = -1;
			sim = -1;
		      }
		      else{
			sim = compute_PHON_sim(it1,it2, (Prons&)P,wordPhSim,mode,0, constrained); // MOD 7/3/2000 - cast added
		      }
		    }
		  }
		}
	      }
	      else cerr << "ERROR in go_cluster::stage " << stage  << " is not supported\n";
	    }
	    else if (id1 > id2)
	      if (SIM[id2][id1] > 0)
		sim = SIM[id2][id1];
	    
	    if (sim > 0){
	      SIM[id1][id2] = sim;
	      
	      if (sim > max_sim_row){
		max_sim_row  = sim;
		itmax_row = it2;
	      }
	    }
	}
	if (itmax_row != C.end())
	  most_similar[id1] = itmax_row;
    }	
    
    // Explanation for the values to be found in "SIM":
    // SIM[i][j] is 0 if there is no partial constraint between cluster i and cluster j, 
    //                but also there is no reason to compute the similarity value because they don't correspond to the same word
    // SIM[i][j] is -1 if there is a partial constraint  between cluster i and cluster j; it can also be 0 if the similarity is 0
    // SIM[i][j] has a value between 0 and 1 if the two clusters are potential candidates for merging; the value is the similarity 
        
    vector<unsigned> eliminated(no_clusters);   // keeps track of the clusters that were eliminated due to merging

    for (int i = 0; i<no_clusters; i++)
      eliminated[i] = 0;

    bool merge = 1;
    int count_steps = 0;
    bool print_links = 1;
    bool print_words = 1;
    
    if (stage == 1)
	print_words = 0;
    
    while(merge){
	double max_sim = 0;
	
	ClusterListIt itmax1, itmax2;
	
	for (ClusterListIt it1 = C.begin(); it1 != C.end(); it1++){
	    unsigned id1 = (*it1).Id();
		    
	    if (most_similar[id1] != C.end()){
		ClusterListIt it2 = most_similar[id1];
		unsigned id2 = (*it2).Id();
				
		if (SIM[id1][id2] > max_sim){
		    max_sim = SIM[id1][id2];
		    itmax1 = it1; 
		    itmax2 = it2;	
		}
	    }
	}
		
	if (max_sim == 0)	
	    merge = 0;	
	else{	
	    count_steps++;
	    
	    unsigned id1 = (*itmax1).Id(); 
	    unsigned id2 = (*itmax2).Id();
	    
	    // id1, id2 are the ids of the clusters to be merged
	    	    
	    if (id2 < id1){
		unsigned tmp = id1;
		id1 = id2;
		id2 = tmp;
		ClusterListIt ittmp = itmax1;
		itmax1 = itmax2;
		itmax2 = ittmp;
	    }

	    cout << "==========================================\n";
	    cout << "STEP " << count_steps << endl;
	    cout << "==========================================\n";

	    cout << endl;
	    (*itmax1).print(P,print_links,print_words);
	    cout << endl;
	    (*itmax2).print(P,print_links,print_words);
	    cout << endl;
	    
	    (*itmax1).merge_with(*itmax2, stage);

	    IdsList Liv, Ljv, Lio, Ljo;
	    eliminated[id2] = 1;
	    
	    for (int i = 0; i< no_clusters; i++){
		
		if (eliminated[i] || i == id1 || i == id2)
		    continue;
		
	      // update the similarities
		
		if (SIM[i][id1]>0 && SIM[id1][i]>0 && SIM[i][id2]>0 && SIM[id2][i]>0){
		    if (mode == 0) 
			SIM[i][id1] =  maximf(SIM[i][id1], SIM[i][id2]);
		    else if (mode == 1){
			unsigned no_links1 = (*itmax1).no_links();
			unsigned no_links2 = (*itmax2).no_links();
			SIM[i][id1] = averagef(SIM[i][id1], SIM[i][id2], no_links1, no_links2);
		    }
		    else
			cerr << "ERROR in go_cluster::mode " << mode << " is not supported\n";
		    
		    SIM[id1][i] = SIM[i][id1];
		}
	      
		// update the partial order
		if (SIM[i][id1] == -1)
		    Liv.push_front(i);
		
		if (SIM[i][id2] == -1)
		    Ljv.push_front(i);
	      
		if (SIM[id1][i] == -1)
		    Lio.push_front(i);
		
		if (SIM[id2][i] == -1)
		    Ljo.push_front(i);
		
		if (SIM[i][id2] == -1) 
		    SIM[i][id1] = -1;
		
		
		if (SIM[id2][i] == -1) 
		    SIM[id1][i] = -1;
	    }
	    
	    for (IdsListConstIt i = Liv.begin(); i!= Liv.end(); ++i)
		for (IdsListConstIt j = Ljo.begin(); j!= Ljo.end(); ++j)
		    SIM[*i][*j] = -1;
		
	    
	    for (IdsListConstIt i = Lio.begin(); i!= Lio.end(); ++i)
		for (IdsListConstIt j = Ljv.begin(); j!= Ljv.end(); ++j)
		    SIM[*j][*i] = -1;
	    
	   
	    // update "most_similar"
	    if (constrained){
		for (ClusterListIt it1 = C.begin(); it1 != C.end(); it1++){
		    if (it1 != itmax2){
			unsigned id_source = (*it1).Id();		
		    
			if ((SIM[id_source][id1] == 0 && SIM[id1][id_source] > -1 && SIM[id_source][id2] > 0 && SIM[id2][id_source] > -1) || (SIM[id_source][id2] == 0 && SIM[id2][id_source] > -1 && SIM[id_source][id1] > 0 && SIM[id1][id_source] > -1)){
			    if (stage == 1)
				SIM[id_source][id1]= compute_TIME_sim(it1,itmax1,linkTimeSim,mode,constrained);	
			    else if (stage== 2 || stage ==4)     
				SIM[id_source][id1]= compute_PHON_sim(it1,itmax1, (Prons&)P,wordPhSim,mode,1, constrained); // MOD 7/3/2000 - cast added
			    else if (stage == 3)
				SIM[id_source][id1]= compute_PHON_sim(it1,itmax1, (Prons&)P,wordPhSim,mode,0, constrained); // MOD 7/3/2000 - cast added

			    if (SIM[id_source][id1] > 0 && SIM[id1][id_source] > -1){
				SIM[id1][id_source] = SIM[id_source][id1];
			    }
			}
		    }	
		}
	    }
	    
	    for (ClusterListIt it1 = C.begin(); it1 != C.end(); it1++){
		if (it1 != itmax2){
		    unsigned id_source = (*it1).Id();
				
		    if (most_similar[id_source] != C.end()){
			unsigned id_closest = (*most_similar[id_source]).Id();
			if (SIM[id_source][id_closest] == -1 || SIM[id_closest][id_source] == -1 || id_source == id1 || id_closest == id1 || id_closest == id2){
			    // if they became ordered after merging, recompute the most similar candidate for it1
			
			    double max_sim_row = 0;
			    ClusterListIt itmax_row;
			    
			    for (ClusterListIt i = C.begin(); i != C.end(); i++){
				if (i != itmax2){
				    unsigned id = (*i).Id();
				    double new_val = SIM[id_source][id];
			    
				    if (new_val >= 0 && SIM[id][id_source] >=0 && new_val > max_sim_row){
					max_sim_row = new_val;
					itmax_row = i;
				    }
				}
			    }
			    
			    if (max_sim_row > 0){
				most_similar[id_source] = itmax_row;
			    }
			    else{
				most_similar[id_source] = C.end();
			    }
			}
			else {
			    if (SIM[id_source][id_closest] < SIM[id_source][id1] && SIM[id1][id_source] > -1)
				most_similar[id_source] = itmax1;
			}
		    }
		    else{
			if (constrained && SIM[id_source][id1] > 0 && SIM[id1][id_source] > -1)
			    most_similar[id_source] = itmax1;	
		    }	
		}
	    }
	    C.erase(itmax2);
	}
    }
                
    IntIntIntMapIt b = constraints.begin();
    IntIntIntMapIt e = constraints.end();
    constraints.erase(b,e);
    
    IntIntDblMapIt bd = linkTimeSim.begin();
    IntIntDblMapIt ed = linkTimeSim.end();
    linkTimeSim.erase(bd,ed);
        
    unsigned count_row = 0;
    
    for (ClusterListIt it1 = C.begin(); it1 != C.end(); it1++){
	unsigned count_col = 0;
	unsigned id1 = (*it1).Id();
	for (ClusterListIt it2 = C.begin(); it2 != C.end(); it2++){
	    if (it2 != it1){
		unsigned id2 = (*it2).Id();
		if (SIM[id1][id2] == -1){
		    constraints[pair<int,int>(count_row,count_col)]=1;
		}
	    }
	    count_col++;
	}
	count_row++;
    }	
    
    count_row = 0;	
    
    for (ClusterListIt it1 = C.begin(); it1 != C.end(); it1++){
      (*it1).setId(count_row);
	count_row++;
    }	
    return; // MOD 7/3/2000 - 1 removed
}

// ////////////////////////////////////////////////////////////////////////////////


double    
Clustering::compute_TIME_sim(ClusterListConstIt itc1, ClusterListConstIt itc2, IntIntDblMap& linkTimeSim, const int mode, const bool constrained)
{
    double sum = 0;
    double max = 0;
    
    LnProb total_prob = ((Lattice* )lat_ptr)->Total_prob();  // MOD 7/3/2000 - cast added 
    bool has_time_info = ((Lattice* )lat_ptr)->has_time_info(); // MOD 7/3/2000 - cast added 
        
    Cluster& c1 = (Cluster&) *itc1; // MOD 7/3/2000 - cast added
    Cluster& c2 = (Cluster&) *itc2; // MOD 7/3/2000 - cast added
    
    if (constrained){
      int best_link_1 = c1.Best_link();
      int best_link_2 = c2.Best_link();

      if (has_time_info){
	if (compute_time_overlap(lat_ptr->link(best_link_1).start_time(),lat_ptr->link(best_link_1).end_time(),lat_ptr->link(best_link_2).start_time(),lat_ptr->link(best_link_2).end_time()) == 0)	
	  return 0;	
      }
      else{	
	if (compute_time_overlap_ph(lat_ptr->link(best_link_1).start_node().Max_dist(),lat_ptr->link(best_link_1).end_node().Max_dist(),lat_ptr->link(best_link_2).start_node().Max_dist(),lat_ptr->link(best_link_2).end_node().Max_dist()) <= 0)	
	  return 0;
	}	
    }
    
    for (IdsListConstIt it1 = c1.clinks.begin(); it1!= c1.clinks.end(); ++it1){
	double diff = lat_ptr->link(*it1).Pscore() - total_prob;
	if (fabs(diff) < exp(-20)) diff = -0.0001;
	double p1 =  exp(diff);
	
	for (IdsListConstIt it2 = c2.clinks.begin(); it2!= c2.clinks.end(); ++it2){
	    double sim = 1;
	    
	    if (has_time_info){
		IntInt N;
		N.first  = *it1;
		N.second = *it2;
		IntIntDblMapIt it = linkTimeSim.find(N);
		if (it == linkTimeSim.end()){
		    // the time overlap between these two links has not been computed before 
		    sim = compute_time_overlap(lat_ptr->link(*it1).start_time(),lat_ptr->link(*it1).end_time(),lat_ptr->link(*it2).start_time(),lat_ptr->link(*it2).end_time());
		    linkTimeSim[N] = sim;
		    linkTimeSim[pair<int,int>(*it2,*it1)]=sim;
		}
		else  sim = linkTimeSim[N];
	    }	
	    
	    diff = lat_ptr->link(*it2).Pscore() - total_prob;
	    if (fabs(diff) < exp(-20)) diff = -0.0001;
	    double p2 =  exp(diff);
	    
	    sim *= p1*p2;
	    
	    if (mode == 0){
		if (sim > max) max = sim;
	    }
	    else if (mode == 1){
		sum += sim;
	    }
	    else cerr << "ERROR in compute_TIME_sim_with():" << mode << " is not supported \n";  
	}
    }
    
    if (mode == 0)
	    return max;
    else {
	int total = c1.clinks.size() * c2.clinks.size();
	return sum/total;    
    }		
}


// ///////////////////////////////////////////////////////////////////////////////////

double    
Clustering::compute_PHON_sim(ClusterListConstIt cit1, ClusterListConstIt cit2, Prons& P, IntIntDblMap& wordPhSim, const int mode, bool use_phon_info, bool constrained)
{
    bool print = false;
    
    double max = 0;
    double sum = 0;
    LnProb total_prob = ((Lattice* )lat_ptr)->Total_prob(); // MOD 7/3/2000 - cast added  
   
    Cluster& c1 = (Cluster&) *cit1; // MOD 7/3/2000 - cast added  
    Cluster& c2 = (Cluster&) *cit2; // MOD 7/3/2000 - cast added 
    
    if (constrained)
      if (compute_time_overlap_ph(c1.Min_time(),c1.Max_time(),c2.Min_time(),c2.Max_time()) < 0)
        return 0;
    
    for (IntDblMapConstIt it1 = c1.cwords.begin(); it1 != c1.cwords.end(); ++it1){
      double diff = (*it1).second - total_prob;
      if (fabs(diff) < exp(-20)) diff = -0.0001;
      double p1 =  exp(diff);
      
      for (IntDblMapConstIt it2 = c2.cwords.begin(); it2 != c2.cwords.end(); ++it2){
	double diff = (*it2).second - total_prob;
	if (fabs(diff) < exp(-20)) diff = -0.0001;		
	double p2 =  exp(diff);
	
	IntInt N;
	N.first  = (*it1).first;
	N.second = (*it2).first;
	
	double sim =1;
	if (use_phon_info){
	  IntIntDblMapIt it = wordPhSim.find(N);
	  
	  if (it == wordPhSim.end()){
	    // the phonetic similarity between these two words has not been computed before 
	    
	    const string& pron1 = P.get_pron((*it1).first); // MOD 6/30/2000 - const added
	    const string& pron2 = P.get_pron((*it2).first); // MOD 6/30/2000 - const added
	    if (pron1 == "" || pron2 == "" )
	      sim = compute_word_similarity(P.get_word((*it1).first),P.get_word((*it2).first));
	    else
	      sim = compute_phonetic_similarity(pron1,pron2);
	    
	    wordPhSim[N] = sim;
	    wordPhSim[pair<int,int>((*it2).first,(*it1).first)] = sim;
	  }
	  else
	    sim = wordPhSim[N];
	}
	sim *= p1*p2;
	
	if (mode == 0){
	  if (sim > max) max = sim;
	}
	else if (mode == 1){
	  sum += sim;
	}
	else cerr << "ERROR in compute_PHON_sim_with():" << mode << " is not supported \n";  
      }
    }
    
    if (mode == 0){
      return max;
    }
    else {
      int total = c1.cwords.size()*c2.cwords.size();
      return sum/total;
    }
}

// //////////////////////////////////////////////////////////////////////////////////////////////

int
Clustering::no_words()
{	
    IntIntMap words;

    for (ClusterListIt it  = C.begin(); it != C.end(); ++it){
	Cluster& c = (*it);
	for (IdsListConstIt itl = c.clinks.begin(); itl!= c.clinks.end(); ++itl){
	    int idx = lat_ptr->link(*itl).Word_idx();
	    words[idx] = 1;
	}	
    }
    return words.size();
}

// /////////////////////////////////////////////////////////////////////////////////////////////

void
Clustering::add_EPS(float DELweight)
{
    LnProb total_prob = ((Lattice* )lat_ptr)->Total_prob(); // MOD 7/3/2000 - cast added
    cout << "TOTAL PROB : " << total_prob << endl;

    for (ClusterListIt it  = C.begin(); it != C.end(); ++it){
	Cluster& c = (*it);
	
	LnProb eps_prob;
	LnProb total_prob_cluster = 0;
	
	for (IntDblMapIt itw = c.cwords.begin(); itw != c.cwords.end(); itw++){
	    LnProb score = (*itw).second;
	    assert(fabs(score -total_prob)< exp(-20) || (score < total_prob));
	    if (score >  total_prob)
	      cout << score << " ** " << total_prob << endl;

	    score = exp(score - total_prob);
	    (*itw).second = score;
	    total_prob_cluster += score;
	}
	// put the remaining mass on EPS
	if (total_prob_cluster > 1 && fabs(total_prob_cluster - 1) < exp(-20)){
	    eps_prob = 0;
	}
	else if (total_prob_cluster < 1){
	    eps_prob = 1 - total_prob_cluster;	
	    eps_prob *= DELweight;                  
	    c.cwords.insert(pair<int,double>(-1,eps_prob));
	}
    }	
}


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
Clustering::print_sausages_FSMformat(const string& outdir, const string& outfile, const string& infile, float lowEpsT, float highEpsT, float lowWordT, float highWordT, const Prons& P, bool output_fullpath)
{
    bool std_out = false;

    string label;
    if (output_fullpath)
	label = infile;
    else {
	LineSplitter ls1("/");
	ls1.Split(infile);
	label = ls1[ls1.NoWords()-1];
    }
    LineSplitter ls(".");
    ls.Split(label);
    label = ls[0];
    
    ostream* outCons;
    if (outfile == ""){
	std_out = true;
	outCons = &cout;
    }
    else 
	outCons = new ofstream (outfile.c_str(),ios::app);
    ofstream fout;
    
    if (outdir != ""){
	string outfile = outdir + "/" + label + ".saus";
	
	fout.open(outfile.c_str());
	assert(fout);
	fout.setf(ios::left, ios::adjustfield);
    }
    
    // add SENT_START as the first cluster
    fout << setw(6) << 0 << setw(6) << 1 << setw(20) << START_WD.c_str() << setw(10) << 1 << endl;

    unsigned count = 1;
    string consensus("");
    
    for (ClusterListConstIt it  = C.begin(); it != C.end(); ++it){
	Cluster& c = (Cluster&) (*it); // MOD 7/3/2000 - cast added 
	LnProb score;
	
	// sort the words in decreasing order on their posterior probability 

	IntDblVector v(c.cwords.size());
	copy(c.cwords.begin(),c.cwords.end(),v.begin()); 
	compW comp_words;
	sort (v.begin(), v.end(), comp_words);
	
	int wordidx_best = v[0].first;
	LnProb score_best = v[0].second;
	
	if (wordidx_best == -1){
	    // eps has the highest score;
	    if (outdir != ""){
		if (score_best > highEpsT)
		    count--;
		else {
		  int i; // MOD 7/3/2000 - declaration added (otherwise invalid outside "for")
		  for (int i = 1; i< v.size(); ++i)
		    if (v[i].second > lowWordT) break;
		  if (i != v.size()){
		    fout << setw(6) << count << setw(6) << count+1 << setw(20) << "eps" << setw(10) << score_best << endl;
		    for (int i = 1; i< v.size(); ++i)
		      if (v[i].second > lowWordT){
			const Word& w = P.get_word(v[i].first); // MOD 7/3/2000 - const added
			fout << setw(6) << count << setw(6) << count+1 << setw(20) << w.c_str() << setw(10) << v[i].second << endl;
		      }
		  }
		}
	    }
	}
	else{
	    // a real word has the highest score
	    const Word& w = P.get_word(wordidx_best); // MOD 7/3/2000 - const added
	    consensus += w + " ";
	    
	    if (outdir != ""){
		fout << setw(6) << count << setw(6) << count+1 << setw(20) << w.c_str() << setw(10) << score_best << endl;
		
		if (score_best <= highWordT){
		    for (int i = 1; i< v.size(); ++i)
			if (v[i].first >=0){
			    if (v[i].second > lowWordT){
				const Word& w = P.get_word(v[i].first);  // MOD 7/3/2000 - const added
				fout << setw(6) << count << setw(6) << count+1 << setw(20) << w.c_str() << setw(10) << v[i].second << endl;
			    }
			}
			else{
			    if (v[i].second > lowEpsT)
				fout << setw(6) << count << setw(6) << count+1 << setw(20) << "eps" << setw(10) << v[i].second << endl;
			}
		}
	    }
	}
	count++;
    }
    // add SENT_END and EPS
    fout << setw(6) << count << setw(6) << count+1 << setw(20) << END_WD.c_str() << setw(10) << 1 << endl;
    fout << setw(6) << count+1 << setw(6) << count+2 << setw(20) << EPS.c_str() << setw(10) << 1 << endl;
    
    // add an end state 
    fout << count+2 << endl;

    *outCons << consensus << "\t(" << label << ")" << endl;
    
    if (outdir != ""){
	fout.close();
    }
    
    if (!std_out){	
	((ofstream *)outCons)->close();
	delete outCons;
    }	
    return; // MOD 7/3/2000 - 1 removed
}
    
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void
Clustering::TopSort()
{
    unsigned no_clusters = C.size();
    
    IdsMatrix order;
    order.resize(no_clusters);
    
    for (int i = 0; i < no_clusters; i++)
        order[i].resize(no_clusters);
    
    for (IntIntIntMapIt it = constraints.begin(); it!= constraints.end(); ++it){
      unsigned firstclust = (*it).first.first;
      unsigned secondclust = (*it).first.second;
	
      assert(firstclust < no_clusters && secondclust < no_clusters);
      order[firstclust][secondclust] = 1;
    }	
    
    IdsList l;
    IdsVector color(no_clusters);    
    for (int i=0; i<no_clusters; i++)
      color[i]=0;

    for (int i = 0; i < no_clusters; i++)	
	if (color[i] == 0)
	    DFS_visit(i,order,l,color);
    
    ClusterList C1;	
    
    for (IdsListConstIt it = l.begin(); it!= l.end(); ++it)
	for (ClusterListIt cit  = C.begin(); cit != C.end(); ++cit)
	    if ((*cit).Id() == (*it)){
		Cluster c1 = *cit;
		C1.push_back(c1);
		C.erase(cit);
		break;
	    }		
    swap(C,C1);
    
    unsigned count = 0;
    for (ClusterListIt cit  = C.begin(); cit != C.end(); ++cit){
	(*cit).setId(count);
	count++;
    }		
    return; // MOD 7/3/2000 - 1 removed
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////
    
void
Clustering::DFS_visit(const unsigned startid, IdsMatrix& order, IdsList& l, IdsVector& color)
{
    color[startid] = 1;
    IdsVector& v = order[startid];
    
    for (unsigned i = 0; i< v.size(); i++)
	if (v[i] == 1 && color[i] == 0) 
	    DFS_visit(i, order, l, color);
    
    color[startid] = 2;
    l.push_front(startid);
    return;
}	
