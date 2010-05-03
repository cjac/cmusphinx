//-------------------------------------
//  Link.h stores a lattice link
//-------------------------------------

// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//--------------------------------------------------------------------------------------------------------

#ifndef _Link_h
#define _Link_h

#include "Prob.h"
#include "Prons.h"
#include "common_lattice.h"
#include "Node.h"
#include <string>

class Link
{
    friend class Lattice;
    
 public:
    Link();
    Link(const string&, const Lattice*, const Prons&);

    const Node& start_node() const;
    const Node& end_node() const;
    
    float       start_time() const;
    float       end_time() const;
    
    const NodeId  Start_node_id() const {return start_node_id;}
    const NodeId  End_node_id() const {return end_node_id;}
     
    LinkId Id() const {return id;}
    void setId(unsigned i) {id = i;}
    
    LnProb ACscore() const{ return ac_score;}
    LnProb LMscore() const{ return lm_score;}
    LnProb PRscore() const{ return pr_score;}
        
    unsigned Word_idx() const {return word_idx;}
    bool is_pruned() const {return pruned;}
    const LnProb Pscore() const { return pscore;}
    const LnProb Score() const { return score;}
    
    friend ostream&  operator<<(ostream& os, const Link& link);
    void print_it(const Prons& );

 private:
    const Lattice*      lat_ptr;
    LinkId              id;
    
    NodeId              start_node_id;
    NodeId              end_node_id;  
    
    int                 word_idx;             /* the index of the word associated with this link */
        				         
    LnProb              ac_score;             /* acoustic model log prob */
    LnProb              lm_score;             /* language model log prob */
    LnProb              pr_score;             /* pronunciation model log prob */
    
    LnProb              pscore;               /* link posterior probability */
    LnProb              score;                /* the weighted combination of the LM, AC, PR and wdpenalty */
    
    bool                pruned;               /* a flag for the status of the link (1=pruned, 0=kept) */
    
    void  fill_link_fields(const string& id, const string& entry);
};


// /////////////////////////////////////////////////////////////////////////////////////

// sort links in lexicographical order of their start/end nodes

struct compL : binary_function<Link, Link, bool> {
    bool operator () (const Link& l1, const Link& l2) {
	unsigned start1 = l1.Start_node_id();
	unsigned start2 = l2.Start_node_id();
	unsigned end1 = l1.End_node_id();
	unsigned end2 = l2.End_node_id();
	
	if (end1 < end2) return 1;
	else if (end1 == end2 && start1 < start2) return 1;
	return 0;
    }
};
// ////////////////////////////////////////////////////////////////////////////////////

// sort links in decreasing order of their scores (log posterior probabilities)

struct compLS : binary_function<Link, Link, bool> {
    bool operator () (const Link& l1, const Link& l2) {
	return (l1.Pscore() > l2.Pscore());
    }
};

// ////////////////////////////////////////////////////////////////////////////////////

#endif


