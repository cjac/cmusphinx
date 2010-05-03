//-------------------------------------------------------------------------------------------
//  Lattice.cc  
//-------------------------------------------------------------------------------------------
// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------

// 05/31/2001    fixed bug in  DFS_visit    


#include <stdlib.h>
#include <iomanip.h>
#include <stdio.h>
#include <assert.h>
#include <iostream.h>
#include <stdiostream.h>
#include <algo.h>
#include <vector.h>
#include <math.h>

#include "Link.h"
#include "Lattice.h"
#include "LineSplitter.h"
#include "common_lattice.h"
#include "common.h"
#include "zio.h"

/* ---------------------------------------------------------------------------------- */
/* --------------------------- LINK STUFF ----------- ------------------------------- */
/* ---------------------------------------------------------------------------------- */


// ----------------------------- private ---------------------------


void 
Link::fill_link_fields(const string& idf, const string& entry)
{
  if (idf == "J")
    id = (LinkId)atol((const char*)entry.c_str());
  else if (idf == "S")
    start_node_id = (NodeId)atol((const char*)entry.c_str());
  else if (idf == "E")
    end_node_id = (NodeId)atol((const char*)entry.c_str());
  else if (idf == "a")
    ac_score = atof((const char*)entry.c_str());
  else if (idf == "n" || idf == "l")
    lm_score = atof((const char*)entry.c_str());
  else if (idf == "r")
    pr_score = atof((const char*)entry.c_str());
  else if (idf == "score")   // this is for the FSM style input 
    score = atof((const char*)entry.c_str());
  else{
    cerr << "ERROR: Link::fill_link_fields(): unknown identifier:" << endl;
    cerr << idf << " " << entry << endl;
    exit(1);
  }
  return;
}


// ---------------------------------------- public -----------------------------------------


Link::Link():
  lat_ptr((Lattice *)0), id(0), start_node_id(0), end_node_id(0), word_idx(-1), ac_score(LZERO), lm_score(LZERO), pr_score(LZERO), score(LZERO), pscore(LZERO), pruned(false){}


Link::Link(const string& line, const Lattice* from_lattice, const Prons& P): 
  lat_ptr(from_lattice), id(0), start_node_id(0), end_node_id(0), word_idx(-1), ac_score(LZERO), lm_score(LZERO), pr_score(LZERO), score(LZERO), pscore(LZERO), pruned(false)
{
  static LineSplitter fields;
  fields.Split(line);
  
  unsigned int no_fields = fields.NoWords();
  
  if (from_lattice->Type() == "SLF"){
    assert(no_fields <  MAX_NO_FIELDS_LINK);
    assert(no_fields >= MIN_NO_FIELDS_LINK);   
    assert(line.find_first_of("J") == 0);
    
    for(int i=0; i < no_fields; i++){
      int pos = fields[i].find("=");
      assert(pos != string::npos);
     const string& idf = fields[i].substr(0,pos); // MOD 6/30/2000 - const added
     const string& entry = fields[i].substr(pos+1,string::npos); // MOD 6/30/2000 - const added
      
      if (idf == "W" || idf == "WORD"){
	word_idx = ((Prons&) P).get_idx(entry); // MOD 6/30/2000 - cast added	was: word_idx = P.get_idx(entry);
      }
      else
	fill_link_fields(idf, entry);
    }
  }
  else if (from_lattice->Type() == "FSM"){
    assert(no_fields <  MAX_NO_FIELDS_FSM);
    assert(no_fields >= MIN_NO_FIELDS_FSM);  
    assert(line.find_first_of(" ") != 0);
    
    if (no_fields >= 3){
      // node 1 node2  word  [score]
      fill_link_fields("S",fields[0]);               
      fill_link_fields("E",fields[1]);               
      
      word_idx = ((Prons&) P).get_idx(fields[2]); // MOD 6/30/2000 - cast added
	
      if (no_fields == 4)
	fill_link_fields("score", fields[no_fields-1]); 
      else 
	fill_link_fields("score", "0");
    }
    else if (no_fields != 0){
      // end_node [score]
      
      fill_link_fields("S",fields[0]);               
      word_idx = ((Prons&) P).get_idx(EPS); // MOD 6/30/2000 - cast added
      
      if (no_fields == 2)
	fill_link_fields("score",fields[1]);
      else
	fill_link_fields("score","0");
      
      fill_link_fields("E",fields[0]); // i make it the same as the start node and take care of it later on  
    }
  }
  else cerr << "ERROR:Link():lattice type " << from_lattice->Type() << " not supported\n";
}

// ///////////////////////////////////////////////////////////////////////////

const Node& 
Link::start_node() const{ return lat_ptr->node(start_node_id);}

const Node& 
Link::end_node()   const{ return lat_ptr->node(end_node_id);}

float 
Link::start_time() const { return lat_ptr->node(start_node_id).Time();}

float 
Link::end_time() const { return lat_ptr->node(end_node_id).Time();}


// /////////////////////////////////////////////////////////////////////////////

void 
Link::print_it(const Prons& P)
{
  cout.setf(ios::left, ios::adjustfield);
 const string& w = P.get_word(word_idx); // MOD 6/30/2000 - const added
  string wdash = " -- "; 
  cout << " S=" << setw(5)  << start_node_id << " E=" << setw(5)  << end_node_id;
  cout << " W=" << setw(20) <<  w.c_str();
  if (((Lattice* )lat_ptr)->has_time_info()) // MOD 7/3/2000 - cast added
    cout << " Time: " << setw(5) << start_time() << setw(5) << wdash.c_str() << setw(10) << end_time();
  else{
    cout << " Approx time: " << setw(5) << this->start_node().Max_dist() << setw(4) << wdash.c_str() << setw(10) << this->end_node().Max_dist();
    if (start_time() >= 0)
      cout << " Time: " << setw(5) << start_time() << setw(5) << wdash.c_str() << setw(10) << end_time();
  }
  cout << "Prob=" << setw(10)  << Pscore();
  cout << endl;
  cout.setf(ios::internal, ios::adjustfield);
}	

// /////////////////////////////////////////////////////////////////////////////


ostream& 
operator<<(ostream& os, const Link& link)
{
  os.setf(ios::left, ios::adjustfield);
    os << " J=" << setw(6)    << link.Id() << " S=" << setw(5)  << link.Start_node_id() << " E=" << setw(5)  << link.End_node_id();
    os << " W=" << setw(20) <<  link.Word_idx() ;
    //os << " a=" << setw(10)  << link.ACscore();
    //os << " n=" << setw(10)  << link.LMscore();
    //os << " r=" << setw(10)  << link.PRscore();
    if (link.start_time() >= 0)
      os << setw(10) << link.start_time() << " -- " << setw(10) << link.end_time();
    os << "score=" << setw(10)  << link.Score();
    os << "Pscore=" << setw(10)  << link.Pscore();
    os.setf(ios::internal, ios::adjustfield);
    assert(os.good());
    return os;
}

/* ---------------------------------------------------------------------------------- */
/* --------------------------- Node stuff ----------- ------------------------------- */
/* ---------------------------------------------------------------------------------- */


/* ----------------------- private --------------------- */

void 
Node::fill_node_fields(const string& idf, const string& entry)
{ 
  if (idf == "I"){
    id = (NodeId)atol((const char *)entry.c_str());
  }else if (idf == "t" || idf == "T"){
    time = atof((const char*)entry.c_str());
  }else if (idf == "v"){
  }
  else{ 
    cerr << "ERROR: Lattice::fill_node_fields(): unknown identifier:";
    cerr << idf << " " << entry << endl;
    exit(1);
  }
  return;
}

/* --------------- public ---------------------- */

Node::Node():
  lat_ptr((Lattice *)0),id(0),word_idx(-1),time(-1.0), max_dist(0){}

Node::Node(const string& line, const Lattice* from_lattice, const Prons& P): 
  lat_ptr(from_lattice),id(0),word_idx(-1),time(-1.0),max_dist(0)
{
  static LineSplitter fields;
  fields.Split(line);
  
  unsigned int no_fields = fields.NoWords();
  
  assert(no_fields <  MAX_NO_FIELDS_NODE);
  assert(no_fields >= MIN_NO_FIELDS_NODE);
  assert(line.find_first_of("I") == 0);
  
  for(int i=0; i < no_fields; i++){
    int pos = fields[i].find("=");
    assert(pos != string::npos);
    const string& idf = fields[i].substr(0,pos);  // MOD 6/30/2000 - const added
    const string& entry = fields[i].substr(pos+1,string::npos); // MOD 6/30/2000 - const added
    
    if (idf == "W" || idf == "WORD")
      word_idx = ((Prons&) P).get_idx(entry); // MOD 6/30/2000 - cast added
    else
      fill_node_fields(idf, entry);
  }	
}

// /////////////////////////////////////////////////////////////

ostream& operator<<(ostream& os, const Node& node)
{
  os.setf(ios::left, ios::adjustfield);
  os << "I=" << setw(5) << node.id;
  if (node.time >= 0)
    os << "t=" << setw(6) << node.time;
  if (node.word_idx != -1) 
    os << "W=" << setw(20) << node.word_idx;
  os.setf(ios::internal, ios::adjustfield);
  assert(os.good());
  return os;
}

// ///////////////////////////////////////////////////////////////

/* ---------------------------------------------------------------------------------- */
/* --------------------------- Lattice Info Stuff ----------------------------------- */
/* ---------------------------------------------------------------------------------- */


void 
LatticeInfo::fill_info_fields(const string& idf, const string& entry)
{
  if (idf == "UTTERANCE")
    utterance = entry;
  else if (idf == "wdpenalty")
    wdpenalty = atof((const char*)entry.c_str());
  else if (idf == "lmscale" || idf == "ngscale")
    lmscale = atof((const char*)entry.c_str());
  else if (idf == "prscale")
    prscale = atof((const char*)entry.c_str());
  else if (idf == "NODES" || idf == "N")
    no_nodes = (unsigned int)atol((const char*)entry.c_str());
  else if (idf == "LINKS" || idf == "L")
    no_links = (unsigned int)atol((const char*)entry.c_str());
  else if (idf == "VERSION"){        
  }
  else if (idf == "ngname" || idf == "lmname"){
  }
  else if (idf == "vocab"){
  }
  else if (idf == "hmms"){
  }
  else if (idf == "tscale"){}
  //else{
  //	cerr << "ERROR: LatticeInfo::fill_info_fields(): unknown identifier:" << endl;
  //	cerr << idf << " " << entry << endl;
  //	exit(1);
  //   }
  return;
}      

// //////////////////////////////////////////////////////////////////////////////////////////


LatticeInfo::LatticeInfo(const string& infile, const string& graph_type):
  file_name(infile), type(graph_type), utterance(""), lmscale(0), prscale(0), wdpenalty(0), no_nodes(0), no_links(0)
{
  //istdiostream f(zopen(file_name.c_str(),"rt"));
 ifstream  f;
 f.open(infile.c_str());
 

 if (!f){
    cerr << "ERROR:: File " << file_name << " doesn't exist !!" << endl;
    exit(1);
  }
  string line;
  
  LineSplitter fields;
  
  if (type == "SLF"){
    string info_line;
    
    while(f.peek() != EOF){
      getlineH(f, line); assert(f.good());
      
      if(line.find_first_of("#") == 0)
	continue;
      else if(line.find_first_of("I") == 0)
	continue;
      else if(line.find_first_of("J") == 0)
	continue;
      else
	info_line += line + " ";
    }	
    fields.Split(info_line);
    
    unsigned int no_fields = fields.NoWords();
    
    assert(no_fields <  MAX_NO_FIELDS_INFO_SLF);
    assert(no_fields >= MIN_NO_FIELDS_INFO_SLF);
    
    LineSplitter comp("=");
    for(int i=0; i < no_fields; i++){
      assert(fields[i].find("=") != string::npos);
      comp.Split(fields[i]);
      assert(comp.NoWords() == 2);
      fill_info_fields(comp[0], comp[1]);
    }
  }
  else if (type == "FSM"){
    unsigned count_lines = 0;
    map<string,int> m;
    bool add_node = false;
    
    while(f.peek() != EOF){
      getlineH(f, line); assert(f.good());
      if(line.find_first_of("#") == 0)
	continue;
      else{	
	fields.Split(line);
	unsigned no_fields = fields.NoWords();
	
	assert(no_fields <  MAX_NO_FIELDS_FSM);
	assert(no_fields >= MIN_NO_FIELDS_FSM);
	
	if (no_fields != 0)
	  count_lines++;
	
	if (no_fields >= 3){
	  m[fields[0]] = 1;
	  m[fields[1]] = 1;
	}
      }	
    }
    no_links = count_lines;
    no_nodes = m.size() +1 ; 
    // I will add a new end-node; this is mainly for the case with
    // multiple end nodes 
  }
  else 
    cerr << "ERROR:LatticeInfo():lattice type " << type << " not supported\n";
 
 f.close(); 
 //f.sync();
// zclose(f.rdbuf()->stdiofile());
  
  assert(no_nodes >0);
  assert(no_links >0);
}
    

// ////////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& os, const LatticeInfo& info)
{
  os << "Type="       <<      info.type                         << endl;
  os << "File name="  <<      info.file_name                    << endl;
  os << "UTTERANCE="  <<      info.utterance                    << endl;
  
  os << "wdpenalty="  <<      info.wdpenalty                    << endl;
  os << "lmscale="    <<      info.lmscale                      << endl;
  os << "prscale="    <<      info.prscale                      << endl;
  os << "NODES="      <<      info.no_nodes                     << endl;
  os << "LINKS="      <<      info.no_links                     << endl;
  assert(os.good());
  return os;
}

/* ---------------------------------------------------------------------------------- */
/* --------------------------- Lattice: general stuff ------------------------------- */
/* ---------------------------------------------------------------------------------- */

Lattice::Lattice(const LatticeInfo& from_info, const Prons& P):
  info(from_info), nodes(from_info.no_nodes), links(from_info.no_links),total_prob(LZERO),time_info(true)
{
  //istdiostream h(zopen(info.file_name.c_str(),"rt"));
  //assert(h);
  ifstream h;
  h.open(info.file_name.c_str());

  string line;
  
  if (info.type == "SLF"){
    // go over the header lines
    while((h.peek() != EOF) && (h.peek() != 'I')){
      getlineH(h, line); assert(h.good());
      continue;
    }
    
    // read the nodes
    while((h.peek() != EOF) && (h.peek() == 'I')){
      getlineH(h, line); 
      assert(h.good());
      Node this_node(line, this, P);
      assert(this_node.id < nodes.size());
      nodes[this_node.id] = this_node;
    }	
    assert (nodes.size() == info.no_nodes);
    
    if (nodes[0].time < 0) time_info = false;  // this is an SLF without time information
    
    while((h.peek() != EOF) && (h.peek() != 'J')){
      getlineH(h, line); assert(h.good());
      continue;
    }
    
    // read the links
    while((h.peek() != EOF) && (h.peek() == 'J')){
      getlineH(h, line); assert(h.good());
      Link this_link(line, this, P);
      assert(this_link.id < links.size());
      
      if (this_link.word_idx == -1){
	Node& n = this->node(this_link.end_node_id);
	this_link.word_idx = n.word_idx;
      }
      links[this_link.id] = this_link;
    }
    assert(links.size() == info.no_links);
    }
  else if (info.type == "FSM"){
    time_info = false;                        // the FSMs don't have time information
    
    unsigned count_nodes = 0;
    
    for (int i = 0; i< info.no_nodes; ++i){
      Node this_node;
      this_node.setId(i);
      nodes[this_node.Id()] = this_node;     
      // we make the assumption that the nodes have ids 
      // between 0 and info.no_nodes; info.no_nodes-1 is the new node
    }		
	
    unsigned count_links = 0;
    LineSplitter ls;
    
    while(h.peek() != EOF){
      getlineH(h, line); 
      assert(h.good());
      
      ls.Split(line);
      
      if (ls.NoWords() != 0){
	// read the links
	Link this_link(line, this, P);
	if (this_link.start_node_id == this_link.end_node_id)
	  this_link.end_node_id = info.no_nodes-1;
	links[count_links] = this_link; 
	count_links++;	
      }	
    }
    assert(info.no_links == links.size());
  }
  else 
    cerr << "ERROR:Lattice():lattice type " << info.type << " not supported\n";
  
 h.close();
// h.sync();	
 // zclose(h.rdbuf()->stdiofile());
  
  fill_outgoing_links(); // fill in the list of the outgoing nodes for each node
  
  bool nodes_OK = false;
  bool links_OK = false;
  
  if (time_info){
    nodes_OK = this->check_nodes();
    
    if (nodes_OK)
      links_OK = this->check_links();
  }
  
  if (time_info == false || nodes_OK == false || links_OK == false){
    do_TopSort();   
  }
}


// ////////////////////////////////////////////////////////////////////////////////////////////////////

bool 
Lattice::check_nodes(){
  int no_nodes = this->no_nodes();
  for (int i = 1; i< no_nodes; ++i)
    if (nodes[i].time < nodes[i-1].time) return false; 
  return true;
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////

bool 
Lattice::check_links(){
  int no_links = this->no_links();
  for (int i = 0; i< no_links; ++i)
    if ((i > 0 && links[i].end_time() < links[i-1].end_time()) || (links[i].start_node_id > links[i].end_node_id)) return false; 
  return true;
}

// /////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned 
Lattice::put_uniform_pron_prob(const Prons& P){
    if (links[0].pr_score == LZERO || fabs(links[0].pr_score)< exp(-20)){
      // either the 'r'  field didn't exist or it did but was 0.000
      int no_links = this->no_links();
      for (int i = 0; i< no_links; ++i){
	int wd_idx = links[i].Word_idx();
	assert(wd_idx != -1);
	unsigned no_prons = P.get_no_prons(wd_idx);
	assert(no_prons != 0);
	LnProb unif_pron_score = -log(no_prons);
	links[i].pr_score = unif_pron_score;
      }
      return 1;
    }
    else return 0;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void 
Lattice::add_prons(const Prons& P, float PRweight)
{
  unsigned no_links = this->no_links();
  for (int i = 0; i< no_links; ++i){
    
    int wd_idx = links[i].Word_idx();
    assert(wd_idx != -1);
    unsigned no_prons = P.get_no_prons(wd_idx);
    assert(no_prons != 0);
    LnProb unif_pron_score = -log(no_prons);
    links[i].score += PRweight * unif_pron_score;
  }
}


// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void 
Lattice::scale_link_scores(const float scale)
{
  unsigned no_links = this->no_links();
  for (int i = 0; i< no_links; ++i)
    links[i].score /= scale; 
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void 
Lattice::add_WIP(const float WIP)
{
  unsigned no_links = this->no_links();
  for (int i = 0; i< no_links; ++i)
    links[i].score -= WIP; 
}


// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

void 
Lattice::compute_link_scores(const float lmscale, const float prscale, const float WIP, const float allscale)
{	
  unsigned no_links = this->no_links();
  for (int i = 0; i< no_links; ++i)
    links[i].score = (links[i].ac_score + lmscale * links[i].lm_score + prscale * links[i].pr_score - WIP)/allscale;
}		

// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void 
Lattice::do_ForwardBackward()
{
  int no_links = this->no_links();
  int no_nodes = this->no_nodes();
  
  unsigned lat_start_node = links[0].start_node_id;
  unsigned lat_end_node = links[no_links-1].end_node_id;
  
  vector<double> alpha(no_nodes,100000.0);
  alpha[lat_start_node] = 0.0;
    
  for (int j = 0; j < no_links; j++){
    double new_score = alpha[links[j].start_node_id] + links[j].score;
    unsigned endnode_id = links[j].end_node_id;
    if (alpha[endnode_id] == 100000.0){
      alpha[endnode_id] = new_score;
    }
    else{
      double dtmp = alpha[endnode_id];
      alpha[endnode_id] = LogPlus(new_score, dtmp);
    }
  }
  
  vector<double> beta(no_nodes, 100000.0);
  beta[lat_end_node] = 0.0;
  
  for (int j = no_links-1; j >= 0; j--){
    double new_beta =  beta[links[j].end_node_id] + links[j].score;
    unsigned startnode_id = links[j].start_node_id;
    
    if (beta[startnode_id] == 100000.0)
      beta[startnode_id] = new_beta;
    else{
      double dtmp = beta[startnode_id];
      beta[startnode_id] = LogPlus(new_beta,dtmp);
    }
  }
  assert(fabs(alpha[lat_end_node] - beta[lat_start_node]) < exp(-20));
  total_prob = alpha[lat_end_node];
  
  for (int j = 0; j < no_links; j++)
    links[j].pscore = alpha[links[j].start_node_id] + links[j].score + beta[links[j].end_node_id];
}

// //////////////////////////////////////////////////////////////////

void 
Lattice::fill_outgoing_links()
{
  int no_links = this->no_links();
  
  for(unsigned i=0; i<no_links; i++){
    Link& curr_link = links[i];
    Node& start_node = nodes[curr_link.start_node_id];
    start_node.outgoing_links[curr_link.end_node_id] = 1;	 
  }
}

// //////////////////////////////////////////////////////////////////////////////

void 
Lattice::mark_pruned_percentage(const float thresh){
  unsigned no_links = this->no_links();
  
  vector<Link> v = links;
  
  compLS comp_link_scores;
  sort(v.begin(), v.end(), comp_link_scores); 
  
  for (unsigned i = 0; i< no_links; i++)
    if  (i > thresh/100 * no_links)
      links[v[i].Id()].pruned = 1;
}

// ///////////////////////////////////////////////////////////////////////////

void 
Lattice::mark_pruned_score(const float thresh){
  unsigned no_links = this->no_links();
  
  for (unsigned i = 0; i < no_links; i++)
    if (exp(total_prob - links[i].Pscore()) > thresh)
      links[i].pruned = 1;
}

// ////////////////////////////////////////////////////////////////////////////////////////     

void 
Lattice::do_TopSort()
{
  IdsList l;
  unsigned no_nodes = this->no_nodes();
  unsigned no_links = this->no_links();
  
  vector<int> color(no_nodes);
  for (int i = 0; i<no_nodes; i++)
    color[i]=0;
  
  for (int i = no_nodes-1; i>=0; --i)
    if (color[i] == 0) DFS_visit(i,l,color);
  
  // node l[i] is mapped to i
  unsigned count = 0;
  vector<Node> nodestmp(no_nodes);
  
  for (IdsListIt it = l.begin(); it != l.end(); it++){
    if (*it != count)
      nodestmp[count] = nodes[*it];
    else 
      nodestmp[count].setId(count);
    count++;
  }
  for (unsigned i = 0; i< no_nodes; i++)
    if (nodestmp[i].id != i){	
      nodes[i] = nodestmp[i];	
      nodes[i].setId(i);
    }		
  IdsVector invl(no_nodes);
  count = 0;
  
  for (IdsListIt it = l.begin(); it != l.end(); it++)
    invl[(*it)] = count++;
  // node i is mapped to invl[i]
  
  for (unsigned i = 0; i< no_links; i++){
    links[i].start_node_id = invl[links[i].start_node_id];
    links[i].end_node_id = invl[links[i].end_node_id];
  }
  
  compL comp_links;    
  sort(links.begin(), links.end(), comp_links);
  
  for (unsigned i = 0; i< no_nodes; i++){
    IntIntMap& v = nodes[i].outgoing_links;
    IntIntMap v1;
    for (IntIntMapIt it = v.begin(); it != v.end(); ++it){
      v1[invl[(*it).first]]=1;
    }
    v.erase(v.begin(),v.end());
    nodes[i].outgoing_links=v1;
  }
  
  for (unsigned i = 0; i< no_links; i++)
    links[i].setId(i);
}

// /////////////////////////////////////////////////////////////////////////////

bool Lattice::less_nodes(NodeId id1, NodeId id2, unsigned MAX_DIST )
{
  /* we did topological sort on the links and renamed the nodes
     therefore, there could be paths only between nodes with increasingly ordered ids */
  
  if (MAX_DIST == 0){
    if (id1 != id2) return 0;
    else return 1;
  }
  IntIntMap& v = nodes[id1].outgoing_links;
  
  for (IntIntMapIt it = v.begin(); it != v.end(); ++it){
    unsigned next_node_id = (*it).first;
    
    if (next_node_id > id2)
      return 0;                              
    else {
      if (next_node_id == id2) return 1;
      else {
        if (MAX_DIST <= 1) continue;
	if (less_nodes(next_node_id, id2, MAX_DIST - 1)) return 1;
      }
    }
  }
  return 0;
}
    
// ////////////////////////////////////////////////////////////////////

void 
Lattice::DFS_visit(int nodeid, IdsList& l, vector<int>& color)
{
  color[nodeid] = 1;
  IntIntMap& v = nodes[nodeid].outgoing_links;
  
  for (IntIntMapIt it = v.begin(); it != v.end(); ++it){
    int next_node_id = (*it).first;
    if (color[next_node_id] == 0) DFS_visit(next_node_id, l, color);
  }
  color[nodeid] = 2;
  l.push_front(nodeid);
  
  return;
}

// //////////////////////////////////////////////////////////////////

void 
Lattice::put_max_dist(const Prons& P)
{
  unsigned no_links = this->no_links();
  LineSplitter ls;
  int length;
  for (int j = 0; j < no_links; j++){
   const  string& ss = P.get_pron(links[j].word_idx); // MOD 6/30/2000 - const added
    if (ss != ""){
      ls.Split(ss);
      length = ls.NoWords();
    }
    else
      length = P.get_word(links[j].word_idx).size();
    
    nodes[links[j].end_node_id].max_dist = max( nodes[links[j].end_node_id].max_dist, nodes[links[j].start_node_id].max_dist + length); 
  }	
}


// //////////////////////////////////////////////////////////////////////

int 
Lattice::no_words()
{
  IntIntMap words;
  unsigned no_links = this->no_links();  
  for (int j = 0; j < no_links; j++){
    int idx = links[j].word_idx;
    if (idx >= 0) words[idx] = 1;	
  }	
  return words.size();
}


// ///////////////////////////////////////////////////////////////////////

ostream& 
operator<<(ostream& os, const Lattice& lat)
{
  os << "#" << endl << "#nodes" << endl << "#" << endl;
  for(NodeId k=0; k< lat.nodes.size(); k++)
    os << lat.nodes[k] << endl;
  os << "#" << endl << "#links" << endl << "#" << endl;
  for(LinkId k=0; k<lat.links.size(); k++)
    os << lat.links[k] << endl;
  assert(os.good());
  return os;
}

// ////////////////////////////////////////////////////////////////////

/*  -------------------------------    THE END  --------------------------------------- */
 




