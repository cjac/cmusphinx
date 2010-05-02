
//#include <limits>
//numeric_limits is not defined in my implementation
#include <cassert>
#include <sstream>
#include <stack>
#include <list>
#include <cmath>
#include <exception>
#include <locale>
#include <algorithm>
#include <functional>
#include <iostream>
#include <string.h>

#include <pcre.h>

#include "PCFG.h"
#include "debug.h"

PCFG::PCFG() {}

int PCFG::addNonTerm(const PCFG::LHS& x) {
  int index = ntmap[x.name] = grammar.size();
  grammar.push_back(x);
  return index; //index to item
}

int PCFG::addTerm(const string& x) {
  map<string, int>::iterator i = tmap.find(x);
  if(i == tmap.end()) {
    int index = tmap[x] = terminal.size();
    terminal.push_back(x);
    return index; //index to new item
  }
  return i->second; //index to existing item
}

int PCFG::merge(const PCFG& x) {
  //merge grammars
  int oldSize = grammar.size();
  vector<int> newIndex;
  for(vector<LHS>::const_iterator i = x.grammar.begin(); i != x.grammar.end(); i++) {
    map<string, int>::const_iterator j = ntmap.find(i->name);
    if(j == ntmap.end()) { //this is new to me
      newIndex.push_back(grammar.size());
      grammar.push_back(*i);
    } else {
      newIndex.push_back(j->second);
    }
  }

  //reindex
  for(int i = 0; i < newIndex.size(); i++)
    for(vector<RHS>::iterator j = grammar[oldSize+i].rule.begin(); j != grammar[oldSize+i].rule.end(); j++)
      for(vector<RHSe>::iterator k = j->element.begin(); k != j->element.end(); k++)
	if(!k->terminal) k->index = newIndex[k->index];
  rebuild_indexes();

  return newIndex[x.head];
}

void PCFG::redoTMap() {
  tmap.clear();
  terminal.clear();
  for(vector<LHS>::iterator i=grammar.begin(); i!=grammar.end(); i++)
    for(vector<RHS>::iterator j=i->rule.begin(); j!=i->rule.end(); j++)
      for(vector<RHSe>::iterator k=j->element.begin(); 
	  k!=j->element.end(); 
	  k++)
	if(k->terminal) k->index = addTerm(k->word);
}

PCFG PCFG::readPhoenixGrammar(istream& pGrammar, const string headname) {
  char line[255];
  PCFG aGCFG;
  pcre *net, *leaf, *macro;
  const char *error = new char[255];
  int erroroffset;
  int ovector[90];
  
  debug << "reading grammar, using head=" << headname << endl;

  net = pcre_compile("^\\[([A-Za-z0-9_='/-]*)\\]", 
		     0, 
		     &error, 
		     &erroroffset,
		     NULL);
  if(net == NULL) {
    ostringstream o;
    o << "net compilation failed at "
      << erroroffset << ": " << error << endl;
    throw o.str();
  }
  leaf = pcre_compile("^\\s+\\(\\s*(\\S+)(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?(\\s+(\\S+))?\\s*\\)", 
		      0, 
		      &error, 
		      &erroroffset,
		      NULL);
  if(leaf == NULL) {
    ostringstream o;
    o << "leaf compilation failed at "
      << erroroffset << ": " << error << endl;
    throw o.str();
  }
  macro = pcre_compile("^[A-Z][0-9A-Z_='-]*", 
		       0, 
		       &error, 
		       &erroroffset,
		       NULL);
  if(macro == NULL) {
    ostringstream o;
    o << "macro compilation failed at "
      << erroroffset << ": " << error << endl;
    throw o.str();
  }
  aGCFG.head = 0;
  LHS *currentNet = NULL;
  LHS *currentMacro = NULL;
  int anoncount = 0;
  while(pGrammar.getline(line, 255)) {
    int rc;
    if(pcre_exec(net, NULL, line, strlen(line), 0, 0, ovector, 90) >= 0) {
      if(currentMacro != NULL) {
	aGCFG.addNonTerm(*currentMacro);
	delete currentMacro;
	currentMacro = NULL;
      }
      if(currentNet != NULL) {
	if(currentNet->name == headname)
	  aGCFG.head = aGCFG.grammar.size();
	aGCFG.addNonTerm(*currentNet);
	delete currentNet;
      }
      currentNet = new LHS(string(line+ovector[2], ovector[3] - ovector[2]));
    } else if((rc = 
	       pcre_exec(leaf, NULL, line, strlen(line), 0, 0, ovector, 90))
	      >= 0) {
      RHS aRHS;
      for(int i=1; i<rc; i+=2) {
	RHS *tempRHS = NULL;
	LHS *tempLHS = NULL;
	string leaf(line+ovector[2*i], ovector[2*i+1] - ovector[2*i]);
	if(leaf.at(0) == '<') continue; //ignore <s> construct
	if(leaf.at(0) == '*') {
	  //this leaf is optional, generate an anonymous production
	  leaf.erase(0,1);
	  ostringstream anonName;
	  anonName << "Anon" << anoncount++;
	  tempLHS = new LHS(anonName.str());
	  tempLHS->rule.push_back(RHS()); //epsilon production
	  tempRHS = new RHS(); //realization production
	} else if(leaf.at(0) == '+') {
	  //this leaf is repeated one or more times
	  leaf.erase(0,1);
	  ostringstream anonName;
	  anonName << "Anon" << anoncount++;
	  tempLHS = new LHS(anonName.str());
	  RHS doubling; //doubling production
	  RHSe doublinge(anonName.str(), false);
	  doubling.element.push_back(doublinge);
	  doubling.element.push_back(doublinge);
	  tempLHS->rule.push_back(doubling);
	  tempRHS = new RHS(); //realization production
	}
	RHSe *currentRHSe;
	if(leaf.at(0) == '[') {
	  //this is a net
	  leaf.erase(leaf.size()-1,1);
	  leaf.erase(0,1);
	  currentRHSe = new RHSe(leaf, false);
	} else if(!isupper(leaf.at(0))) {
	  //this is a regular leaf terminal
	  transform(leaf.begin(), 
		    leaf.end(), 
		    leaf.begin(), 
		    (int(*)(int))toupper);
	  currentRHSe = new RHSe(aGCFG.addTerm(leaf), leaf, true);
	} else {
	  //this is a macro
	  currentRHSe = new RHSe(currentNet->name + ':' + leaf, false);
	}
	if(tempLHS != NULL) {
	  tempRHS->element.push_back(*currentRHSe);
	  tempLHS->rule.push_back(*tempRHS);
	  aGCFG.addNonTerm(*tempLHS);
	  delete currentRHSe;
	  currentRHSe = new RHSe(tempLHS->name, false);
	  delete tempRHS; tempRHS = NULL;
	  delete tempLHS; tempLHS = NULL;
	}
	aRHS.element.push_back(*currentRHSe);
	delete currentRHSe;
      }
      if(currentMacro != NULL)
	currentMacro->rule.push_back(aRHS);
      else
	currentNet->rule.push_back(aRHS);
    } else if(pcre_exec(macro, NULL, line, strlen(line), 0, 0, ovector, 90) 
	      >= 0) {
      if(currentMacro != NULL) {
	aGCFG.addNonTerm(*currentMacro);
	delete currentMacro;
      }
      currentMacro = 
	new LHS(currentNet->name + ':' +
		 string(line+ovector[0], ovector[1] - ovector[0]));
    } 
  }
  //delete net; delete leaf; delete macro;
  if(currentMacro != NULL) {
    aGCFG.addNonTerm(*currentMacro);
    delete currentMacro;
  }
  if(currentNet != NULL) {
    if(currentNet->name == headname)
      aGCFG.head = aGCFG.grammar.size();
    aGCFG.addNonTerm(*currentNet);
    delete currentNet;
  }
  
  //build nonterm indexes
  for(vector<LHS>::iterator i = aGCFG.grammar.begin(); 
      i != aGCFG.grammar.end(); i++ )
    for(vector<RHS>::iterator j = i->rule.begin(); j != i->rule.end(); j++)
      for(vector<RHSe>::iterator k = j->element.begin(); 
	  k != j->element.end();
	  k++)
	if(!k->terminal && k->index == -1) {
	  int index;
	  map<string, int>::const_iterator l = aGCFG.ntmap.find(k->word);
	  if(l == aGCFG.ntmap.end()) {
	    ostringstream o;
	    o << "Can't find " << k->word;
	    cerr << o.str() << endl;
	    throw o.str();
	  } else {
	    k->index = l->second;
	  }
	}

  debug << aGCFG.stats();
  return aGCFG;
}

PCFG PCFG::readPhoenixGrammarAndForms(istream& pGrammar, istream& forms) {
  //first go ahead and get the phoenix grammar
  PCFG aPCFG(readPhoenixGrammar(pGrammar));

  //construct an implied head
  debug << "Adding new head for forms file" << endl;
  aPCFG.head = aPCFG.addNonTerm(LHS("FORMS_IMPLIED_HEAD"));
  debug << aPCFG.stats() << endl;

  debug << "Reading forms file" << endl;
  //now we need to read the forms file and put it on top
  const char* error = new char[255];
  int erroroffset;
  int ovector[90];
  pcre *net = pcre_compile("^\\s*\\[([A-Za-z0-9_='/-]*)\\]", 
			   0, 
			   &error, 
			   &erroroffset,
			   NULL);
  if(net == NULL) {
    ostringstream o;
    o << "net compilation failed at "
      << erroroffset << ": " << error << endl;
    throw o.str();
  }
  pcre *function = pcre_compile("^FUNCTION:\\s*([A-Za-z0-9_='/-]+)(\\s*%%(0\\.\\d*)%%)?",
				0,
				&error,
				&erroroffset,
				NULL);
  if(net == NULL) {
    ostringstream o;
    o << "function compilation failed at "
      << erroroffset << ": " << error << endl;
    throw o.str();
  }

  int currentFunction = -1;
  char line[255];
  while(forms.getline(line, 255)) {
    int matches;
    if((matches = pcre_exec(function, NULL, line, strlen(line), 0, 0, ovector, 90)) >=0) {
      string functionName(line+ovector[2], ovector[3] - ovector[2]);
      currentFunction = aPCFG.addNonTerm(LHS(functionName));
      RHS rootFunctionRule;
      if(matches == 4)
	istringstream(string(line+ovector[6], ovector[7] - ovector[6])) >> rootFunctionRule.probability;
      rootFunctionRule.element.push_back(RHSe(currentFunction, functionName));
      aPCFG.grammar[aPCFG.head].rule.push_back(rootFunctionRule);
      debug << "Added FUNCTION: " << functionName << ' ' << rootFunctionRule.probability << endl;
    } else if(pcre_exec(net, NULL, line, strlen(line), 0, 0, ovector, 90) >=0) {
      string netName(line+ovector[2], ovector[3]-ovector[2]);
      map<string, int>::const_iterator i = aPCFG.ntmap.find(netName);
      if(i == aPCFG.ntmap.end()) {
	warn << "net: " << netName << " in forms file not found in grammar file." << endl;
	continue;
      } 
      RHS functionNetRule;
      functionNetRule.element.push_back(RHSe(i->second, netName));
      aPCFG.grammar[currentFunction].rule.push_back(functionNetRule);
    }
  }
  aPCFG.normalize();

  debug << aPCFG.stats() << endl;
  return aPCFG;
}

void PCFG::writePhoenixGrammar(ostream& pGrammar) const {
  pGrammar << "# #lhs=" << grammar.size() << endl;
  pGrammar << '[' << grammar[head].name << ']' << endl;
  pGrammar << "# #rhs=" << grammar[head].rule.size() << endl;
  for(vector<RHS>::const_iterator i = grammar[head].rule.begin(); 
      i != grammar[head].rule.end();
      i++) {
    pGrammar << "\t( ";
    for(vector<RHSe>::const_iterator j = i->element.begin(); 
	j != i->element.end();
	j++)
      pGrammar << j->word << ' ';
    pGrammar << ')' << endl;
  }
  for(int i = 0; i < grammar.size(); i++) {
    if(i == head)
      continue;
    pGrammar << grammar[i].name << endl;
    pGrammar << "# #rhs=" << grammar[i].rule.size() << endl;
    for(vector<RHS>::const_iterator j = grammar[i].rule.begin(); 
	j != grammar[i].rule.end();
	j++) {
      pGrammar << "\t( ";
      for(vector<RHSe>::const_iterator k = j->element.begin(); 
	  k != j->element.end();
	  k++)
	pGrammar << k->word << ' ';
      pGrammar << ')' << endl;
    }    
  }
  pGrammar << ';' << endl;
}

/* 
 * Given a well-formed stocastic context-free grammar, return a grammar in Chomsky Normal form. The
 * stocastic characterization of the returned CNF grammar is preserved.
 * 
 * In Chomsky normal form, each rule is either of the form:
 * A -> BC or
 * A -> x
 * where A, B, and C are non-terminals, and x is a terminal.
 *
 * Technique is to:
 * 1) remove all unit productions (i.e. rules of the form A -> B), by bubbling up
 * 2) in productions with a length > 1, replace terminals with non-terminals, e.g.:
 *       A -> x y z
 *       becomes
 *       A -> B C D
 *       B -> x
 *       C -> y
 *       D -> z
 * 3) itereatively shorten productions with new productions, e.g.:
 *       A -> B C D E
 *       becomes
 *       A -> B F
 *       F -> C G
 *       G -> D E
 */
PCFG PCFG::CNF(const PCFG& g) {
  PCFG ret = removeUnitProductions(g);
  cout << ret << "--endunit" << endl;

  cerr << "about to replace terminals in long rules" << endl;
  cerr << ret.stats() << endl;

  //step 1: replace terminals with nt for productions of length > 1
  int oldsize = ret.grammar.size();
  for(int i = 0; i < oldsize; i++) {
    for(vector<RHS>::iterator j = ret.grammar[i].rule.begin(); 
	j != ret.grammar[i].rule.end();) { //for each rule
      bool mod=false;
      if(j->element.size() > 1) {
	for(vector<RHSe>::iterator k = j->element.begin();
	    k != j->element.end();
	    k++) {
	  if(k->terminal) {
	    mod=true;
	    ostringstream newrule;
	    newrule << "R" << ret.grammar.size();
	    LHS newlhs(newrule.str());
	    RHS newrhs;
	    newrhs.element.push_back(RHSe(k->word));
	    newlhs.rule.push_back(newrhs);
	    k->terminal = false;
	    k->word = newrule.str();
	    k->index = ret.grammar.size();
	    ret.addNonTerm(newlhs);
	    break;
	  }
	}
      }
      if(mod)
	j = ret.grammar[i].rule.begin();
      else
	j++;
    }
  }
  ret.redoTMap();

  cerr << "done replacing terminals with nt" << endl;
  cerr << ret.stats() << endl;

  //step 2: shorten productions with new productions
  oldsize = ret.grammar.size();
  for(int i=0; i<oldsize; i++) {
    for(vector<RHS>::iterator j=ret.grammar[i].rule.begin(); 
	j!=ret.grammar[i].rule.end();) {
      bool mod = false;
      if(j->element.size() > 2) {
	mod = true;
	vector<RHSe> appendix(j->element);
	j->element.clear();
	for(int index=0; index<appendix.size()-1; index++) {
	  j->element.push_back(appendix[index]);
	  if(index+2 == appendix.size()) {
	    j->element.push_back(appendix[index+1]);
	    break;
	  }
	  int i = ret.grammar.size();
	  ostringstream newlname;
	  newlname << "R" << i;
	  j->element.push_back(RHSe(i, newlname.str(), false));
	  //now we just need to add the new nt and process it's rhs
	 
	  ret.addNonTerm(LHS(newlname.str()));
	  ret.grammar[i].rule.push_back(RHS());
	  j = ret.grammar[i].rule.begin();
	}
      }
      if(mod) j=ret.grammar[i].rule.begin(); else j++;
    }
  }

  ret.redoTMap();
  //cout << ret << "--end" << endl;
  cerr << "about to reduce..." << endl;
  cerr << ret.stats() << endl;
  ret.reduce();
  ret.initialize();
  return ret;
}

void PCFG::reduce() {
  cerr << "reducing grammar size..." << endl;
  //iteratively remove identical productions
  int size;
  //operate on a list to facilitate erasures
  cerr << "rebuild indexes" << endl;
  int iy=0;
  ntmap.clear();
  for(vector<LHS>::const_iterator i=grammar.begin(); i!=grammar.end(); i++) {
    ntmap[i->name] = iy++;
  }  
  for(vector<LHS>::iterator i=grammar.begin(); i!=grammar.end(); i++)
    for(vector<RHS>::iterator j=i->rule.begin(); j!=i->rule.end(); j++)
      for(vector<RHSe>::iterator k=j->element.begin();
	  k!=j->element.end();
	  k++)
	if(!k->terminal) {
	  map<string, int>::const_iterator l = ntmap.find(k->word);
	  if(l == ntmap.end())
	    cerr << k->word << " of " << i->name << " index " << k->index
		 << " != ntmap[" << k->word << "] = " << ntmap[k->word] << endl;
	  else k->index = l->second;
	}
  list<LHS> G(grammar.begin(), grammar.end());
  do {
    size = G.size();
    cerr << "size: " << size << endl;
    int ic = 0; int mod = G.size()/78;
    for(list<LHS>::iterator i=G.begin(); i!=G.end(); i++) {
      list<LHS>::iterator j=i;
      for(j++; j!=G.end();) {
	if(*i == *j) {
	  int del_ind = ntmap[j->name];
	  if(del_ind == head) head = ntmap[i->name];
	  j = G.erase(j);
	  for(list<LHS>::iterator k=G.begin(); k!=G.end(); k++) {
	    for(vector<RHS>::iterator l=k->rule.begin(); 
		l!=k->rule.end(); 
		l++) {
	      for(vector<RHSe>::iterator m=l->element.begin(); 
		  m!=l->element.end();
		  m++) {
		if(!m->terminal && m->index == del_ind) {
		  *m = RHSe(ntmap[i->name], i->name);
		}
	      }
	    }
	  }
	  if(!(ic++%mod)) cerr << '.';
	} else j++;
      }
      if(!(ic++%mod)) cerr << '.';
    }
    cerr << endl;
  } while(size>G.size());
  int ii=0;
  ntmap.clear();
  for(list<LHS>::const_iterator i=G.begin(); i!=G.end(); i++) {
    ntmap[i->name] = ii++;
  }
  for(list<LHS>::iterator i=G.begin(); i!=G.end(); i++) {
    for(vector<RHS>::iterator j = i->rule.begin(); j != i->rule.end(); j++) {
      for(vector<RHSe>::iterator k = j->element.begin(); 
	  k != j->element.end();
	  k++) {
	if(!k->terminal) {
	  map<string, int>::const_iterator l = ntmap.find(k->word);
	  if(l == ntmap.end()) 
	    cerr << "Can't find " << k->word << " of " << i->name << endl;
	  else
	    k->index = l->second;
	}
      }
    }
  }
  head = ntmap[grammar[head].name];
  grammar.clear();
  grammar.assign(G.begin(), G.end());

  cerr << "done with reduction" << endl;
  cerr << stats() << endl;
}

void PCFG::rebuild_indexes() {
  ntmap.clear();
  for(int i=0; i<grammar.size(); i++) {
    string& name = grammar[i].name;
    map<string, int>::const_iterator j = ntmap.find(name);
    if(j != ntmap.end())
      cerr << "We seem to have '" << name << "' = " << i << " = " << j->second << endl;
    ntmap[name] = i;
  }
  for(int i=0; i<grammar.size(); i++) {
    for(vector<RHS>::iterator j = grammar[i].rule.begin();
	j != grammar[i].rule.end();
	j++) {
      for(vector<RHSe>::iterator k = j->element.begin(); 
	  k != j->element.end();
	  k++) {
	if(!k->terminal) k->index = ntmap[k->word];
      }
    }
  }
}


PCFG::RHSe PCFG::shorten(const vector<RHSe>& r, int index) {
  //take a string of nodes and return a single node to represent it

  RHSe answer(r[index]);

  //if there's only one just return it
  if(r.size()-index == 1)
    return answer;

  //add new lhs
  int i = grammar.size();
  ostringstream newlname;
  newlname << "R" << i;
  addNonTerm(LHS(newlname.str()));
  RHS newrhs;
  newrhs.element.push_back(r[index]);
  newrhs.element.push_back(shorten(r, index+1));
  grammar[i].rule.push_back(newrhs);

  //return the new lhs
  answer = RHSe(i, newlname.str(), false);
  return answer;
}

PCFG PCFG::removeEpsilons(const PCFG& g) {
  cerr << "about to remove Epsilons..." << endl;
  cerr << g.stats() << endl;

  //first, find all nullable nt
  bool foundNullable;
  vector<bool> nullable(g.grammar.size(),false);
  do {
    foundNullable = false;
    for(int i = 0; i < g.grammar.size(); i++) {
      if(nullable[i])
	continue;
      for(vector<RHS>::const_iterator j = g.grammar[i].rule.begin(); 
	  j != g.grammar[i].rule.end(); 
	  j++) {
	bool pNull = true;
	for(vector<RHSe>::const_iterator k = j->element.begin(); 
	    k != j->element.end(); 
	    k++) {
	  if(k->terminal || (!nullable[k->index])) {
	    //an element of the production is not nullable
	    //so the production is also not nullable
	    pNull = false;
	    break;
	  }
	}
	if(pNull) {
	  //a production is nullable
	  //so the nt is nullable
	  nullable[i] = foundNullable = true;
	  break;
	}
      }
    }
  } while(foundNullable);
  
  //second, rewrite rules with and without nullables
  PCFG gprime;
  gprime.head = g.head;
  for(int i=0; i<g.grammar.size(); i++) {
    LHS aLHSprime(g.grammar[i].name);
    for(vector<RHS>::const_iterator j = g.grammar[i].rule.begin(); 
	j != g.grammar[i].rule.end(); 
	j++) { //for each rule of the old grammar
      stack< pair<list<RHSe>,int> > flat;
      pair<list<RHSe>,int> 
	w(list<RHSe>(j->element.begin(),j->element.end()),0);
      flat.push(w);
      while(!flat.empty()) {
	RHS aRHSprime;
	list<RHSe> buff = flat.top().first; 
	int done = flat.top().second;
	flat.pop();
	int cruise = 0;
	for(list<RHSe>::iterator k = buff.begin(); k != buff.end();k++) {
	  if(cruise >= done && !k->terminal && nullable[k->index]) {
	    w = pair<list<RHSe>,int>(buff, cruise+1);
	    flat.push(w);
	    k = buff.erase(k);
	    w = pair<list<RHSe>,int>(buff, cruise);
	    flat.push(w);
	    aRHSprime.element.clear();
	    break;
	  }
	  cruise++;
	  aRHSprime.element.push_back(*k);
	}
	if(!aRHSprime.element.empty()) {
	  aLHSprime.rule.push_back(aRHSprime);
	}
      }
    }
    gprime.addNonTerm(aLHSprime);
  }

  gprime.redoTMap();

  cerr << "removed Epsilons" << endl;
  cerr << gprime.stats() << endl;
  return gprime;
}

PCFG PCFG::removeUnitProductions(const PCFG& g) {
  PCFG ret = removeEpsilons(g);
  cerr << "about to remove unit productions..." << endl;
  cerr << ret.stats() << endl;

  bool foundUnit;
  do {
    PCFG temp = ret;
    ret.grammar.clear();
    ret.ntmap.clear();
    foundUnit = false;
    for(int i = 0; i < temp.grammar.size(); i++) {
      LHS aLHSprime(temp.grammar[i].name);
      for(vector<RHS>::const_iterator j = temp.grammar[i].rule.begin(); 
	  j != temp.grammar[i].rule.end(); 
	  j++) {
	if(j->element.size() > 1 || j->element[0].terminal) {
	  //not a unit production
	  //add unless duplicate
	  bool dup = false;
	  for(vector<RHS>::const_iterator k = aLHSprime.rule.begin(); 
	      k != aLHSprime.rule.end();
	      k++) {
	    if(*j == *k) {
	      dup = true;
	      break;
	    }
	  }
	  if(!dup)
	    aLHSprime.rule.push_back(*j);
	} else {
	  //is a unit production
	  //add all of the child's productions
	  foundUnit = true;
	  LHS child = temp.grammar[j->element[0].index];
	  for(vector<RHS>::const_iterator l = child.rule.begin(); 
	      l != child.rule.end();
	      l++) {
	    //skip if self-referencing
	    if(l->element.size() == 1 && 
	       !l->element[0].terminal &&
	       l->element[0].index == i)
	      continue;
	    //add unless duplicate
	    bool dup = false;
	    for(vector<RHS>::const_iterator m = aLHSprime.rule.begin();
		m != aLHSprime.rule.end();
		m++) {
	      if(*l == *m) {
		dup = true;
		break;
	      }
	    }
	    if(!dup)
	      aLHSprime.rule.push_back(*l);
	  }
	}
      }
      ret.addNonTerm(aLHSprime);
    }
  } while(foundUnit);

  ret.removeUnreachables();

  cerr << "removed unit productions..." << endl;
  cerr << ret.stats() << endl;
  return ret;
}

vector<bool> PCFG::reachableNT() const {
  vector<bool> ret(grammar.size(), false);
  vector<bool> t(terminal.size(), false);
  reachable(head, ret, t);
  return ret;
}

vector<bool> PCFG::reachableT() const {
  vector<bool> nt(grammar.size(), false);
  vector<bool> t(terminal.size(), false);
  reachable(head, nt, t);
  return t;
}

void PCFG::reachable(int from, 
		     vector<bool>& already, 
		     vector<bool>& alreadyT) const {
  if(already[from])
    return;
  already[from] = true;
  for(vector<RHS>::const_iterator i = grammar[from].rule.begin(); 
      i != grammar[from].rule.end();
      i++)
    for(vector<RHSe>::const_iterator j = i->element.begin(); 
	j != i->element.end();
	j++)
      if(!j->terminal) {
	if(j->index >= grammar.size()) cerr << "nt index out of bounds" << endl;
	reachable(j->index, already, alreadyT);
      } else {
	if(j->index >= terminal.size()) cerr << "t index out of bounds" << endl;
	alreadyT[j->index] = true;
      }
  return;
}

void PCFG::removeUnreachables() {
  cerr << "about to remove unreachables" << endl;
  cerr << stats() << endl;

  //find unreachable nt
  vector<bool> r = reachableNT();

  //compute shift amt
  vector<int> shiftamt(grammar.size());
  int amt=0;
  for(int i=0; i<grammar.size(); i++) {
    shiftamt[i]=amt;
    if(!r[i]) --amt;
  }

  //shift head marker
  head += shiftamt[head];

  //remove unreachables
  for(int i=0; i < grammar.size(); i++) {
    if(!r[i]) continue;
    //redo ntindex
    /*
    for(vector<RHS>::iterator j = grammar[i].rule.begin();
	j != grammar[i].rule.end();
	j++)
      for(vector<RHSe>::iterator k = j->element.begin(); 
	  k != j->element.end();
	  k++)
	if(!k->terminal)
	  k->index += shiftamt[k->index];
    */
    //shift
    if(shiftamt[i] != 0) grammar[i+shiftamt[i]] = grammar[i];
  }
  grammar.erase(grammar.begin() + grammar.size()+amt,
		grammar.end());

  rebuild_indexes();
  redoTMap();

  cerr << "done removing unreachables" << endl;
  cerr << stats() << endl;
}

int PCFG::train(const corpus& data, double threshhold) {
  //double logLikelihood = numeric_limits<double>::min;
  //numeric_limits is not defined in my implementation
  double logLikelihood = -1e290; 
  double oldLogLikelihood = -1e300;
  initialize();
  int iterationcount = 0;
  while((logLikelihood - oldLogLikelihood) > threshhold) {
    iterationcount++;
    oldLogLikelihood = logLikelihood;
    logLikelihood = 0;
    
    initialize_counts();

    for(corpus::const_iterator i = data.begin(); i != data.end(); i++) {
      //compute inside probabilities
      sentence W = *i;
      //the inside and outside probabilities are
      //charts filled with nt's and thier probabilities in the chart
      Chart< map<int,double> > alpha(W.size()), beta(W.size());
      for(int ai=0; ai<W.size(); ai++) {
	//compute alphaii
	for(int lhsi=0; lhsi<grammar.size(); lhsi++)
	  for(int rhsi=0; rhsi<grammar[lhsi].rule.size(); rhsi++) {
	    if(grammar[lhsi].rule[rhsi].element[0].terminal && 
	       grammar[lhsi].rule[rhsi].element[0].word == W[ai]) {
	      if(alpha[ai][ai].find(lhsi) == alpha[ai][ai].end())
		alpha[ai][ai][lhsi] = grammar[lhsi].rule[rhsi].probability;
	      else
		alpha[ai][ai][lhsi] += grammar[lhsi].rule[rhsi].probability;
	    } 
	  }
      }
      
      //compute the rest
      for(int k=1; k<W.size(); k++)
	for(int ai=0; ai<W.size()-k; ai++) {
	  int aj=ai+k;
	  //compute alphaaij
	  vector< pair<map<int, double>,map<int, double> > > BCvec =
	    alpha.constituents(ai, aj);
	  for(int A=0; A<grammar.size(); A++) {
	    //check nt A
	    for(int constituent=0; constituent < BCvec.size(); constituent++) {
	      for(vector<RHS>::const_iterator ri = grammar[A].rule.begin();
		  ri != grammar[A].rule.end();
		  ri++) {
		if(ri->element[0].terminal)
		  continue;
		int B = ri->element[0].index;
		int C = ri->element[1].index;
		//now we have A->BC for each constituent pair
		map<int, double>::const_iterator leftprob =
		  BCvec[constituent].first.find(B);
		if(leftprob == BCvec[constituent].first.end())
		  continue;
		map<int, double>::const_iterator rightprob =
		  BCvec[constituent].second.find(C);
		if(rightprob == BCvec[constituent].second.end())
		  continue;
		double prob = 
		  (ri->probability)*(leftprob->second)*(rightprob->second);
		map<int,double>::iterator insideprob = alpha[ai][aj].find(A);
		if(insideprob == alpha[ai][aj].end())
		  alpha[ai][aj][A] = prob;
		else
		  insideprob->second += prob;
	      }
	    }
	  }
	}
      
      //cerr << "checking for parse" << endl;
      //check to see if the sentence actually parsed
      if(W.size() == 0 || 
	 (alpha[0][W.size()-1].find(head) == alpha[0][W.size()-1].end())) {
	cerr << "no parse for: ";
	for(int i=0; i<W.size(); i++)
	  cerr << W[i] << ' ';
	cerr << endl;
	continue;
      }
      logLikelihood += log(alpha[0][W.size()-1][head]);
			   
      //cerr << "bout to compute betas..." << endl;
      //compute outside probabilities
      beta[0][W.size()-1][head]=1;
      for(int t=W.size()-2; t >= 0; t--)
	for(int bi=W.size()-t-1; bi >=0; bi--) {
	  int bj=bi+t;
	  //compute left outside
	  for(int k=0; k<bi; k++) {
	    //for each production B->CA
	    for(int B=0; B<grammar.size(); B++) {
	      map<int, double>::const_iterator parent = beta[k][bj].find(B);
	      if(parent == beta[k][bj].end())
		continue;
	      //found a parent
	      for(vector<RHS>::const_iterator ri = grammar[B].rule.begin();
		  ri != grammar[B].rule.end();
		  ri++) {
		if(ri->element[0].terminal)
		  continue;
		map<int, double>::const_iterator leftsib = 
		  alpha[k][bi-1].find(ri->element[0].index);
		if(leftsib == alpha[k][bi-1].end())
		  continue;
		//found sibling
		double prob = 
		  (ri->probability) * (leftsib->second) * (parent->second);
		int A = ri->element[1].index;
		map<int, double>::iterator outsideprob = beta[bi][bj].find(A);
		//cerr << "left: " << bi << ' ' << bj << ' ' 
		//    << grammar[A].name << ' '
		//   << prob << printrule(B, ri) << endl;
		if(outsideprob == beta[bi][bj].end())
		  beta[bi][bj][A] = prob;
		else
		  outsideprob->second += prob;
	      }
	    }
	  }
	  //compute right outside
	  for(int k=bj+1; k<W.size(); k++) {
	    //for each production B->AC
	    for(int B=0; B<grammar.size(); B++) {
	      map<int, double>::const_iterator parent = beta[bi][k].find(B);
	      if(parent == beta[bi][k].end())
		continue;
	      //found a parent
	      for(vector<RHS>::const_iterator ri = grammar[B].rule.begin();
		  ri != grammar[B].rule.end();
		  ri++) {
		if(ri->element[0].terminal)
		  continue;
		map<int, double>::const_iterator rightsib = 
		  alpha[bj+1][k].find(ri->element[1].index);
		if(rightsib == alpha[bj+1][k].end())
		  continue;
		//found sibling
		double prob = 
		  (ri->probability) * (rightsib->second) * (parent->second);
		int A = ri->element[0].index;
		map<int, double>::iterator outsideprob = beta[bi][bj].find(A);
		//cerr << "right: " << bi << ' ' << bj << ' ' 
		//   << grammar[A].name << ' '
		//   << prob << endl;
		if(outsideprob == beta[bi][bj].end())
		  beta[bi][bj][A] = prob;
		else
		  outsideprob->second += prob;
	      }
	    }
	  }
	}
      
      /*
      for(int w = 0; w < i->size(); w++) cerr << (*i)[w] << ' ';
      cerr << endl;
      cerr << "alpha" << endl;
      printChart(alpha);
      cerr << "beta" << endl;
      printChart(beta);
      */
      //cerr << "bout to compute cthetas" << endl;
      //accumulate expected rule counts
      for(int lh = 0; lh < grammar.size(); lh++)
	for(vector<RHS>::iterator rh = grammar[lh].rule.begin(); 
	    rh != grammar[lh].rule.end(); 
	    rh++) {
	  //compute c-theta
	  rh->ctheta=0;
	  if(rh->element[0].terminal) {
	    for(int bi=0; bi<W.size(); bi++) {
	      map<int, double>::const_iterator parent = beta[bi][bi].find(lh);
	      if(parent != beta[bi][bi].end() &&
		 rh->element[0].word == (*i)[bi]) {
		//cerr << "adding " << parent->second 
		//     << " to " << printrule(lh, rh) << endl;
		rh->ctheta += parent->second;
	      }
	    }
	  } else {
	    for(int i=0; i<W.size()-1; i++)
	      for(int k=i+1; k<W.size(); k++)
		for(int j=i; j<k; j++) {
		  map<int, double>::const_iterator parent = 
		    beta[i][k].find(lh);
		  if(parent == beta[i][k].end())
		    continue;
		  map<int, double>::const_iterator left = 
		    alpha[i][j].find(rh->element[0].index);
		  if(left == alpha[i][j].end())
		    continue;
		  map<int, double>::const_iterator right =
		    alpha[j+1][k].find(rh->element[1].index);
		  if(right == alpha[j+1][k].end())
		    continue;
		  rh->ctheta += ((parent->second)*(left->second)*(right->second));
		}
	  }
	  rh->ctheta *= ((rh->probability) / alpha[0][W.size()-1][head]);
	}
      
      //cerr << "bout to compute counts" << endl;
      //add c-theta to count
      for(vector<LHS>::iterator lh = grammar.begin(); 
	  lh != grammar.end(); 
	  lh++)
	for(vector<RHS>::iterator rh = lh->rule.begin(); 
	    rh != lh->rule.end(); 
	    rh++) {
	  lh->count += rh->ctheta;
	  rh->count += rh->ctheta;
	}
    }

    //reestimate probabilities
    for(vector<LHS>::iterator lh = grammar.begin(); 
	lh != grammar.end(); 
	lh++)
      for(vector<RHS>::iterator rh = lh->rule.begin(); 
	  rh != lh->rule.end(); 
	  rh++) {
	if(rh->count == 0)
	  rh->probability = 0;
	else
	  rh->probability = rh->count / lh->count;
      }
    
    cerr << iterationcount << ": " << logLikelihood
      // << ' ' << (logLikelihood - oldLogLikelihood) 
	 << endl;  
  }
}

void PCFG::printChart(Chart<map<int, double> >& c) const {
  for(int i=0; i<c.size(); i++)
    for(int j=i; j<c.size(); j++) {
      cout << i << ' ' << j << endl;
      for(map<int, double>::const_iterator k = c[i][j].begin();
	  k != c[i][j].end();
	  k++)
	cout << '\t' << grammar[k->first].name << ' '
	     << k->second << endl;
    }
}

void PCFG::smooth(double amount) {
  for(vector<LHS>::iterator i = grammar.begin(); i != grammar.end(); i++) {
    bool zero = true;
    for(vector<RHS>::iterator j = i->rule.begin(); j != i->rule.end(); j++)
      if(j->probability != 0) zero = false;
    for(vector<RHS>::iterator j = i->rule.begin(); j != i->rule.end(); j++)
      if(zero)
	j->probability = 1.0 / i->rule.size();
      else
	j->probability = j->probability * (1-amount) + amount / i->rule.size();
  }
}

void PCFG::writeVocab(ostream& out) const {
  out << "</s>" << endl;
  out << "<s>" << endl;
  for(map<string, int>::const_iterator i=tmap.begin(); i!=tmap.end(); i++) {
    //char *tmp = new char[i->first.size()+1];
    //int k;
    //for(k=0; k<i->first.size(); k++) tmp[k] = toupper(i->first[k]);
    //tmp[k] = '\0';
    //out << tmp << endl;
    out << uppercase << i->first << endl;
  }
}

string PCFG::printrule(int x, vector<RHS>::const_iterator y) const {
  ostringstream ret;
  ret << grammar[x].name << ' ';
  for(vector<PCFG::RHSe>::const_iterator i = y->element.begin(); 
      i != y->element.end(); 
      i++) {
    ret << (i->terminal?"t ":"nt ") << i->word << ' ';
  }
  ret << "p " << y->probability;
  return ret.str();
}
  
string PCFG::printrule(vector<LHS>::const_iterator x, 
		       vector<RHS>::const_iterator y) {
  ostringstream ret;
  ret << x->name << ' ';
  for(vector<PCFG::RHSe>::const_iterator i = y->element.begin(); 
      i != y->element.end(); 
      i++) {
    ret << (i->terminal?"t ":"nt ") << i->word << ' ';
  }
  ret << "p " << y->probability;
  return ret.str();
}
  
string PCFG::printrule(int x, int y) const {
  ostringstream ret;
  ret << grammar[x].name << ' ';
  for(vector<PCFG::RHSe>::const_iterator i = 
	grammar[x].rule[y].element.begin(); 
      i != grammar[x].rule[y].element.end(); 
      i++) {
    ret << (i->terminal?"t ":"nt ") << i->word << ' ';
  }
  ret << "p " << grammar[x].rule[y].probability;
  return ret.str();
}
  
istream& operator>>(istream& in, PCFG& x) {
  x.grammar.clear();
  x.ntmap.clear();
  in >> x.head;
  char line[255];
  in.getline(line,255);
  while(in.getline(line,255)) {
    string word;
    istringstream iline(line);
    string name;
    iline >> word;
    int index;
    map<string, int>::const_iterator i = x.ntmap.find(word);
    if(i == x.ntmap.end())
      index = x.addNonTerm(word);
    else
      index = i->second;
    PCFG::RHS arhs;
    while(iline >> word) {
      if(word == string("t")) {
	iline >> word;
	arhs.element.push_back(PCFG::RHSe(x.addTerm(word), word, true));
      } else if(word == string("nt")) {
	iline >> word;
	arhs.element.push_back(PCFG::RHSe(word, false));
      } else if(word == string("p")) {
	iline >> arhs.probability;
      } else cerr << "Unexpected token: " << word << endl;
    }
    x.grammar[index].rule.push_back(arhs);
  }

  for(vector<PCFG::LHS>::iterator i=x.grammar.begin(); 
      i!=x.grammar.end(); 
      i++) {
    for(vector<PCFG::RHS>::iterator j=i->rule.begin(); j!=i->rule.end(); j++) {
      for(vector<PCFG::RHSe>::iterator k=j->element.begin(); 
	  k!=j->element.end();
	  k++) {
	if(!k->terminal) {
	  map<string, int>::const_iterator l = x.ntmap.find(k->word);
	  if(l == x.ntmap.end()) {
	    ostringstream o;
	    o << "Couldn't find " << k->word << " of " << i->name;
	    throw o.str();
	  } else k->index = l->second;
	}
      }
    }
  }

  return in;
}

ostream& operator<<(ostream& out, const sentence& x) {
  out << "<s>";
  for(sentence::const_iterator i = x.begin(); i != x.end(); i++)
    out << ' ' << *i;
  return out << " </s>";
}

ostream& operator<<(ostream& out, const corpus& x) {
  for(corpus::const_iterator i = x.begin(); i != x.end(); i++)
    out << *i << endl;
  return out;
}

ostream& operator<<(ostream& out, const PCFG& x) {
  out << x.head << endl;
  out << x.grammar;
  return out;
}

ostream& operator<<(ostream& out, const vector<PCFG::LHS>& x) {
  for(vector<PCFG::LHS>::const_iterator i = x.begin();
      i != x.end(); 
      i++)
    for(vector<PCFG::RHS>::const_iterator j = i->rule.begin(); 
	j != i->rule.end(); 
	j++)
      out << PCFG::printrule(i, j) << endl;
  return out;
}

void PCFG::initialize_counts() {
  for(vector<LHS>::iterator i = grammar.begin(); i != grammar.end(); i++) {
    i->count = 0;
    for(vector<RHS>::iterator j = i->rule.begin(); j != i->rule.end(); j++)
      j->count = 0;
  }
}

void PCFG::initialize() {
  for(vector<LHS>::iterator i = grammar.begin(); i != grammar.end(); i++) {
    for(vector<RHS>::iterator j = i->rule.begin(); j != i->rule.end(); j++) {
      j->probability = 1/(double)i->rule.size();
    }
  }
}

void PCFG::normalize() {
  for(vector<LHS>::iterator j = grammar.begin(); j != grammar.end(); j++) {
    double assigned_total = 0;
    if(j->rule.empty()) throw "empty rule";
    for(vector<RHS>::const_iterator i = j->rule.begin(); i != j->rule.end(); i++) {
      assigned_total += i->probability;
    }
    if(assigned_total > 1) throw "assigned rule probability total exceeds 1";
    for(vector<RHS>::iterator i = j->rule.begin(); i != j->rule.end(); i++) {
      i->probability += (1-assigned_total)/j->rule.size();
    }
  }
}

sentence PCFG::generateSample() const {
  /* 
   * Algorithm: 
   * 1. start with RHSe list consisting of just the head
   * 2. find a NT in the vector and expand it with a random RHS's RHSe list,
   *    in accordance with the distribution
   * 3. do 2 until there are no more nt
   *
   * This is slower than it needs to be.
   */

  sentence s;

  //start with the head
  stack<RHSe> u;
  u.push(RHSe(head, grammar[head].name));

  while(!u.empty()) {
    RHSe top(u.top()); u.pop();
    if(top.terminal) {
      s.push_back(top.word);
    } else {
      //find lhs rule
      LHS lhs = grammar[top.index];

      //pick a rhs, j
      /*
       * OK, so we need MY_RAND_MAX because gcc -mno-cygwin has an inconsistent
       * view of rand_max, i.e. Windiws rand() is 16 bit and gcc RAND_MAX 
       * is 15 bit. Ultimately I think this is a bug in the cygwin-mingw 
       * headers. Using the lower 15 bits of randomness should work regardless,
       * and is plenty for this application.
       */
      const unsigned int MY_RAND_MAX = 0x7FFFU; 
      double p = ((double)(rand()%MY_RAND_MAX) / ((double)(MY_RAND_MAX)+1.0L));
      vector<RHS>::const_iterator j;
      double r = 0;
      for(j = lhs.rule.begin(); j != lhs.rule.end(); j++) {
	if((r += j->probability) > p) break;
      }

      //push the rhs
      //assert(j != lhs.rule.end());
      for(vector<RHSe>::const_reverse_iterator i = j->element.rbegin(); i != j->element.rend(); i++)
	u.push(*i);
    }
  }

  //post-processing to remove <s> construct

  return s;
}
  
corpus PCFG::generateSamples(unsigned int n) const {
  corpus c;
  for(int i=0; i<n; i++) {
    c.push_back(generateSample());
  }
  return c;
}

string PCFG::stats() const {
  ostringstream s;
  if(grammar.size() != ntmap.size()) 
    s << "inconsistent nt map! |ntmap| = " << ntmap.size() << endl;
  if(terminal.size() != tmap.size())
    s << "inconsistent t map! |tmap| = " << tmap.size() << endl;
  int rules = 0;
  for(vector<LHS>::const_iterator i = grammar.begin(); i != grammar.end(); i++)
    rules += i->rule.size();
  s << "Head Rule: " << grammar[head].name << '[' << head << ']' << endl;
  vector<bool> rnt_vec = reachableNT();
  int num_unreachable = count_if(rnt_vec.begin(), rnt_vec.end(), logical_not<bool>());
  vector<bool> rt_vec = reachableT();
  int num_unreachableT = count_if(rt_vec.begin(), rt_vec.end(), logical_not<bool>());
  s << "#NT: " << grammar.size() << " (" << num_unreachable << " unreachable)" << endl
    << "#T: " << terminal.size() << " (" << num_unreachableT << " unreachable)" << endl
    << "#rules: " << rules;
  return s.str();
}
