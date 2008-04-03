
#include <strstream>
#include <functional>

#include "NGram.hpp"

NGram::NGram(const PCFG& aPCFG, int n) : PCFG(aPCFG), N(n) {
  if(N<1) {
    cerr << "Can't handle NGrams of length " << N << endl;
    exit(1);
  }

  //add utterance context
  int lContexti, rContexti, uttResti;
  {
    LHS lContext("LContext");
    RHS lContextRule;
    lContextRule.element.push_back(RHSe(addTerm("<s>"), "<s>", true));
    lContextRule.probability = 1;
    lContext.rule.push_back(lContextRule);
    lContexti = addNonTerm(lContext);
  }
  {
    LHS rContext("RContext");
    RHS rContextRule;
    rContextRule.element.push_back(RHSe(addTerm("</s>"), "</s>", true));
    rContextRule.probability = 1;
    rContext.rule.push_back(rContextRule);
    rContexti = addNonTerm(rContext);
  }
  {
    LHS uttRest("UttRest");
    RHS uttRestRule;
    uttRestRule.element.push_back(RHSe(head, grammar[head].name));
    uttRestRule.element.push_back(RHSe(rContexti, "RContext"));
    uttRestRule.probability = 1;
    uttRest.rule.push_back(uttRestRule);
    uttResti = addNonTerm(uttRest);
  }
  {
    LHS utteranceTop("utteranceTop");
    RHS topRule;
    topRule.element.push_back(RHSe(lContexti, "LContext"));
    topRule.element.push_back(RHSe(uttResti, "UttRest"));
    topRule.probability = 1;
    utteranceTop.rule.push_back(topRule);
    head = addNonTerm(utteranceTop);
  }
  
  cerr << "#terminals " << tmap.size() 
       << " = " << terminal.size() << endl;
  
  //compute initial and final substrings
  vector< map<vector<int>, double> > init;
  vector< map<vector<int>, double> > final;
  if(N>1) {
    cerr << "compute initial substrings of length 1 and 2" << endl;
    //vector< map<string, double> > init(grammar.size());
    //vector< map<string, double> > final(grammar.size());
    init = computeSubstrings(grammar);
    //vector< map<pair<string, string>, double> > init2(grammar.size());
    //vector< map<pair<string, string>, double> > final2(grammar.size());
    {
      vector<LHS> G(grammar);
      for(int i=0; i<G.size(); i++)
	for(vector<RHS>::iterator j = G[i].rule.begin();
	    j != G[i].rule.end();
	    j++)
	  if(j->element.size() > 1)
	    swap(j->element[0], j->element[1]);
      //computeSubstrings(G, final, final2);
      final = computeSubstrings(G);
    }
  }

  //compute expectations
  //compute coefficient matrix A
  cerr << "computing coefficients matrix" << endl;
  InpMtx *A = InpMtx_new();
  InpMtx_init(A, INPMTX_BY_ROWS, SPOOLES_REAL, 0, 0);
  for(int X = 0; X < grammar.size(); X++) {
    vector<double> row(grammar.size());
    row[X] = 1;
    for(vector<RHS>::const_iterator ri = grammar[X].rule.begin();
	ri != grammar[X].rule.end();
	ri++) {
      if(ri->element[0].terminal) continue;
      row[ri->element[0].index] -= ri->probability;
      row[ri->element[1].index] -= ri->probability;
    }
    for(int i=0; i<grammar.size(); i++) {
      if(row[i] != 0) InpMtx_inputRealEntry(A, X, i, row[i]);
    }
  }
  InpMtx_changeStorageMode(A, INPMTX_BY_VECTORS);

  //LU decompose A
  cerr << "size of A " << grammar.size() << endl;
  cerr << "LU decomposing A" << endl;
  //gsl_permutation *p = gsl_permutation_alloc(grammar.size());
  //int signum;
  //gsl_linalg_LU_decomp(A, p, &signum);
  Bridge *bridge = Bridge_new();
  Bridge_setMatrixParams(bridge, 
			 grammar.size(), 
			 SPOOLES_REAL, 
			 SPOOLES_NONSYMMETRIC);
  cerr << "setting up" << endl;
  int rc = Bridge_setup(bridge, A);
  int errorcode = 5;
  cerr << "factoring" << endl;
  rc = Bridge_factor(bridge, A, 1, &errorcode);
  cerr << "rc " << rc << " errorcode " << errorcode << endl; 

  //initialize expectation matricies
  //unigramc = gsl_vector_calloc(tmap.size());
  //bigramc = gsl_matrix_calloc(tmap.size(), tmap.size());
  
  //get unigrams
  if(N==1) {
    DenseMtx *b = DenseMtx_new();
    DenseMtx_init(b, SPOOLES_REAL, 0, 0, grammar.size(), 1, 1, grammar.size());
    cerr << "getting unigrams" << endl;
    for(map<string, int>::const_iterator w = tmap.begin();
	w != tmap.end();
	w++) {
      //compute rhs matrix for unigrams
      //gsl_vector *b = gsl_vector_alloc(grammar.size());
      DenseMtx_zero(b);
      for(int lh = 0; lh < grammar.size(); lh++) {
	//gsl_vector_set(b, lh, 0);
	for(vector<RHS>::const_iterator rh = grammar[lh].rule.begin();
	    rh != grammar[lh].rule.end();
	    rh++) {
	  if(rh->element[0].terminal && (rh->element[0].index == w->second)) {
	    //gsl_vector_set(b, lh, rh->probability);
	    DenseMtx_setRealEntry(b, lh, 0, rh->probability);
	    break;
	  }
	}
      }
    
      //LU solve to get unigram expectations
      //gsl_vector *c = gsl_vector_alloc(grammar.size());
      //gsl_linalg_LU_solve(A, p, b, c);
      Bridge_solve(bridge, 1, b, b);
      
      //store expectation
      //cerr << w->first << "->" << gsl_vector_get(c, head) << endl;
      //gsl_vector_set(unigramc, w->second, gsl_vector_get(c, head));
      double val;
      DenseMtx_realEntry(b, head, 0, &val);
      gram.push(Gram(val, w->second));
      if(gram.size()>MAXNGRAMS) gram.pop();
      
      //gsl_vector_free(c);
      //gsl_vector_free(b);
      DenseMtx_free(b);
    }
  } else {
    DenseMtx *b = DenseMtx_new();
    DenseMtx_init(b, 
		  SPOOLES_REAL, 
		  0, 
		  0, 
		  grammar.size(), 
		  terminal.size(), 
		  1, 
		  grammar.size());
    DenseMtx *c = DenseMtx_new();
    DenseMtx_init(c, 
		  SPOOLES_REAL, 
		  0, 
		  0, 
		  grammar.size(), 
		  terminal.size(), 
		  1, 
		  grammar.size());

    //get bigrams
    cerr << "getting " << N << "-grams" << endl;
    //gsl_vector *b = gsl_vector_alloc(grammar.size());
    //gsl_vector *c = gsl_vector_alloc(grammar.size());
    getNGram(bridge, b, c, init, final);
    DenseMtx_free(b);
    DenseMtx_free(c);
  }
  //gsl_matrix_set(bigramc, tmap["</s>"], tmap["<s>"], 1);
  cerr << endl;
  
  //gsl_vector_free(b);
  //gsl_vector_free(c);

  //dealloc temps
  //gsl_matrix_free(A);
  //gsl_permutation_free(p);
  InpMtx_free(A);
}

void NGram::getNGram(Bridge* bridge, 
		     DenseMtx *b, DenseMtx *c,
		     const vector<map<vector<int>, double> >& init,
		     const vector<map<vector<int>, double> >& final,
		     vector<int> workingGram) {
  workingGram = vector<int>(N);
  int size = terminal.size();
  int space=1; for(int n=N; n; n--) space *= size;
  for(int i=0; i<space;) {
    int j=i;
    for(int index=0; index<N; index++) {
      workingGram[index] = j % size;
      j /= size;
    }
    for(int lh = 0; lh < grammar.size(); lh++) {
      double coeff = 0;
      for(vector<RHS>::const_iterator rh = grammar[lh].rule.begin();
	  rh != grammar[lh].rule.end();
	  rh++) {
	if(!rh->element[0].terminal) {
	  for(int k=1; k<N; k++) {
	    vector<int> former(workingGram.begin(), workingGram.begin()+k);
	    vector<int> latter(workingGram.begin()+k, workingGram.end());
	    map<vector<int>, double>::const_iterator r = 
	      final[rh->element[0].index].find(former);
	    if(r == final[rh->element[0].index].end())
	      continue;
	    double rcoeff = r->second;
	    r = init[rh->element[1].index].find(latter);
	    if(r == init[rh->element[1].index].end())
	      continue;
	    coeff += rcoeff * r->second * rh->probability;
	  }
	}
      }
      DenseMtx_setRealEntry(b, lh, i%size, coeff);
    }
    if(!(++i%size)) {
      cerr << '-';
      //LU solve for each bigram
      Bridge_solve(bridge, 1, c, b);    
      for(int w=0; w<size; w++) {
	double val;
	DenseMtx_realEntry(c, head, w, &val);
	if(val != 0) {
	  workingGram[0] = w;
	  gram.push(Gram(val, workingGram));      
	  if(gram.size() > MAXNGRAMS) gram.pop();
	}
      }
      cerr << "\r" << space-i;
    }
  }
  cerr << endl;
}

NGram::~NGram() {
  //delete[] unigramc;
  //delete[] bigramc;
  //delete[] trigramc;
}

float* NGram::logLikelihood(const vector<string>& phrase) {
  if(phrase.size() != N)
    return NULL;

  int lookup=0;
  for(int i=0; i<N; i++) {
    map<string, int>::const_iterator j = tmap.find(phrase[i]);
    if(j == tmap.end())
      return NULL;
    lookup += (j->second)^(N-i);
  }
  
  //return trigramc + lookup;
  return NULL;
}
  
void NGram::predict(vector<ProbEarlyState>& parse, 
		    int state,
		    //const gsl_matrix *R,
		    DenseMtx *R,
		    const vector<LHS>& G) {
  int rnum;
  for(rnum=parse.size()-1; rnum>=0 && parse[rnum].state == state; rnum--) {
    if(parse[rnum].dot < parse[rnum].rule.element.size() && 
       !parse[rnum].rule.element[parse[rnum].dot].terminal)
      predict(parse, state, R, G, rnum);
  }
}

void NGram::predict(vector<ProbEarlyState>& parse, 
		    int state, 
		    DenseMtx *R,
		    const vector<LHS>& G,
		    int rnum) {
  int nt = parse[rnum].rule.element[parse[rnum].dot].index;
  for(int nt2=0; nt2<grammar.size(); nt2++) {
    //double rval = gsl_matrix_get(R, nt, nt2);
    double rval;
    DenseMtx_realEntry(R, nt, nt2, &rval);
    if(rval > 0)
      for(vector<RHS>::const_iterator ri = G[nt2].rule.begin();
	  ri != G[nt2].rule.end();
	  ri++) {
	int p = findState(parse, state, state, nt2, *ri, 0);
	parse[p].alpha += parse[rnum].alpha*rval*ri->probability;
	parse[p].gamma = ri->probability;
      }
  }
}

set<int> NGram::generate(const vector<ProbEarlyState>& parse, int state) {
  //produce a list of generable words
  set<int> ret;
  for(int i=parse.size()-1; i>=0 && parse[i].state == state; i--)
    if(parse[i].dot == 0 && parse[i].rule.element[0].terminal)
      ret.insert(parse[i].rule.element[0].index);
  return ret;
}

void NGram::scan(vector<ProbEarlyState>& parse, int state, int input) {
  for(int i=parse.size()-1; i>=0 && parse[i].state == state; i--)
    if(parse[i].dot == 0 && 
       parse[i].rule.element[0].terminal &&
       parse[i].rule.element[0].index == input) {
      ProbEarlyState scanned(state+1, 
			     parse[i].index, 
			     parse[i].nt, 
			     parse[i].rule,
			     1,
			     parse[i].alpha,
			     parse[i].gamma);
      parse.push_back(scanned);
    }
}

void NGram::completion(vector<ProbEarlyState>& parse, int state) {
  for(int i=parse.size()-1; i>=0 && parse[i].state == state; i--)
    //if((parse[i].rule.element[0].terminal && parse[i].dot == 1) ||
    // parse[i].dot == 2) {
    if(parse[i].dot == parse[i].rule.element.size()) {
      int j;
      for(j=i-1; j>=0 && parse[j].state > parse[i].index; j--);
      for(; j>=0 && parse[j].state == parse[i].index; j--)
	if(parse[j].dot != 2 && 
	   !parse[j].rule.element[0].terminal &&
	   parse[j].rule.element[parse[j].dot].index == parse[i].nt) {
	  int p = findState(parse,
			    state, 
			    parse[j].index, 
			    parse[j].nt, 
			    parse[j].rule,
			    parse[j].dot+1);
	  parse[p].alpha += parse[j].alpha * parse[i].gamma;
	  parse[p].gamma += parse[j].gamma * parse[i].gamma;
	}
    }
}

int NGram::findState(vector<ProbEarlyState>& parse,
		     int state, 
		     int index, 
		     int nt, 
		     const RHS& rule, 
		     int dot) {
  //returns an index to this fully specified rule
  int ret;
  for(ret=0; ret<parse.size(); ret++) {
    if(parse[ret].state == state &&
       parse[ret].index == index &&
       parse[ret].nt == nt &&
       parse[ret].rule == rule &&
       parse[ret].dot == dot)
      break;
  }
  if(ret == parse.size())
    //didn't find it, so add it
    parse.push_back(ProbEarlyState(state, index, nt, rule, dot));
  return ret;
}

void NGram::printStates(const vector<ProbEarlyState>& parse,
			const vector<LHS>& G) const {
  for(vector<ProbEarlyState>::const_iterator i = parse.begin();
      i != parse.end();
      i++) {
    cout << i->state << ": " 
	 << i->index;
    if(i->nt != -1)
      cout << G[i->nt].name; 
    cout << "->";
    for(int dotn=0; dotn <= i->rule.element.size(); dotn++) {
      if(dotn == i->dot)
	cout << ". ";
      if(dotn < i->rule.element.size())
	if(i->rule.element[dotn].terminal)
	  cout << i->rule.element[dotn].word << ' ';
	else 
	  cout << G[i->rule.element[dotn].index].name << ' ';
    }
    cout << '[' << i->alpha << ',' << i->gamma << ']' << endl;
  }
}
      
/*
void NGram::computeSubstrings(const vector<LHS>& G,
			      vector< map<string, double> >& l1,
			      vector< map<pair<string, string>, double> >& l2) {
  //get transitive left-corner relation
  cerr << "getting left-corner relation" << endl;
  //cerr << G << endl;
  //gsl_matrix *P = gsl_matrix_alloc(G.size(), G.size());
  InpMtx *P = InpMtx_new();
  InpMtx_init(P, INPMTX_BY_ROWS, SPOOLES_REAL, 0, 0);
  //gsl_matrix_set_identity(P);
  for(int lh=0; lh<G.size(); lh++) {
    vector<double> row(G.size());
    row[lh] = 1;
    for(vector<RHS>::const_iterator rh = G[lh].rule.begin();
	rh != G[lh].rule.end();
	rh++) {
      if(!rh->element[0].terminal)
	//*(gsl_matrix_ptr(P, lh, rh->element[0].index)) -= rh->probability;
	row[rh->element[0].index] -= rh->probability;
    }
    for(int i=0; i<grammar.size(); i++) {
      if(row[i] != 0) InpMtx_inputRealEntry(P, lh, i, row[i]);
    }
  }
  //gsl_permutation *p = gsl_permutation_alloc(G.size());
  //int signum;
  //gsl_linalg_LU_decomp(P, p, &signum);
  //gsl_matrix *R = gsl_matrix_alloc(G.size(), G.size());
  //gsl_linalg_LU_invert(P, p, R);
  //gsl_matrix_free(P);
  //gsl_permutation_free(p);
  Bridge *bridge = Bridge_new();
  Bridge_setMatrixParams(bridge, 
			 grammar.size(), 
			 SPOOLES_REAL, 
			 SPOOLES_NONSYMMETRIC);
  cerr << "setting up" << endl;
  int rc = Bridge_setup(bridge, P);
  Bridge_setMessageInfo(bridge, 0, stderr);
  int errorcode = 5;
  cerr << "factoring" << endl;
  rc = Bridge_factor(bridge, P, 1, &errorcode);
  cerr << "rc " << rc << " errorcode " << errorcode << endl; 
  DenseMtx *R = DenseMtx_new();
  cerr << "size is " << G.size() << endl;
  DenseMtx_init(R, SPOOLES_REAL, 0, 0, G.size(), G.size(), 1, G.size());
  DenseMtx_zero(R);
  DenseMtx *I = DenseMtx_new();
  DenseMtx_init(I, SPOOLES_REAL, 0, 0, G.size(), 1, 1, G.size());
  cerr << "allocated" << endl;
  int mod = G.size()/78;
  for(int i=0; i<G.size(); i++) {
    for(int j=0; j<G.size(); j++)
      if(i==j) DenseMtx_setRealEntry(I, j, 0, 1);
      else DenseMtx_setRealEntry(I, j, 0, 0);
    rc = Bridge_solve(bridge, 1, I, I);
    if (!(i%mod)) cerr << '.';
    for(int j=0; j<G.size(); j++) {
      double val;
      DenseMtx_realEntry(I, j, 0, &val);
      DenseMtx_setRealEntry(R, j, i, val);
    }
  }
  cerr << endl;
  cerr << "inverse rc " << rc << " errorcode " << errorcode << endl; 
  InpMtx_free(P);

  cerr << "probabilistic Early parse for initial/final substrings..." << endl;
  //compute initial substrings of length 2
  for(int nt=0; nt<G.size(); nt++) {
    vector<ProbEarlyState> probEarlyParse;
    RHS dotS;
    dotS.element.push_back(RHSe(nt, G[nt].name));
    probEarlyParse.push_back(ProbEarlyState(0, 0, -1, dotS, 0, 1, 1));
    //printStates(probEarlyParse, G);
    //cerr << "predict" << endl;
    predict(probEarlyParse, 0, R, G);
    //printStates(probEarlyParse, G);
    set<string> firstWord = generate(probEarlyParse, 0);
    int branch = probEarlyParse.size();
    for(set<string>::const_iterator i = firstWord.begin(); 
	i != firstWord.end(); 
	i++) {
      //cerr << "scan " << *i << endl;
      scan(probEarlyParse, 0, *i);
      //printStates(probEarlyParse, G);
      l1[nt][*i] = 0;
      for(int ii=probEarlyParse.size()-1; probEarlyParse[ii].state == 1; ii--)
	l1[nt][*i] += probEarlyParse[ii].alpha;
      //cerr << "completion:1" << endl;
      completion(probEarlyParse, 1);
      //printStates(probEarlyParse, G);
      //cerr << "predict:1" << endl;
      predict(probEarlyParse, 1, R, G);
      //printStates(probEarlyParse, G);
      set<string> secondWord = generate(probEarlyParse, 1);
      int branch2 = probEarlyParse.size();
      for(set<string>::const_iterator j = secondWord.begin(); 
	  j != secondWord.end(); 
	  j++) {
	//cerr << "scan:1 " << *j << endl;
	scan(probEarlyParse, 1, *j);
	//printStates(probEarlyParse, G);
	pair<string, string> first2(*i, *j);
	l2[nt][first2] = 0;
	for(int ii=probEarlyParse.size()-1; 
	    probEarlyParse[ii].state == 2; 
	    ii--)
	  l2[nt][first2] += probEarlyParse[ii].alpha;
	probEarlyParse.resize(branch2);
      }
      probEarlyParse.resize(branch);
    }
    if (!(nt%mod)) cerr << '.';
  }
  cerr << endl;

  //gsl_matrix_free(R);
  DenseMtx_free(R);

}
*/

vector<map<vector<int>, double> > 
NGram::computeSubstrings(const vector<LHS>& G) {
  vector<map<vector<int>, double> > answer(G.size());
  //get transitive left-corner relation
  cerr << "getting left-corner relation" << endl;
  //cerr << G << endl;
  //gsl_matrix *P = gsl_matrix_alloc(G.size(), G.size());
  InpMtx *P = InpMtx_new();
  InpMtx_init(P, INPMTX_BY_ROWS, SPOOLES_REAL, 0, 0);
  //gsl_matrix_set_identity(P);
  for(int lh=0; lh<G.size(); lh++) {
    vector<double> row(G.size());
    row[lh] = 1;
    for(vector<RHS>::const_iterator rh = G[lh].rule.begin();
	rh != G[lh].rule.end();
	rh++) {
      if(!rh->element[0].terminal)
	//*(gsl_matrix_ptr(P, lh, rh->element[0].index)) -= rh->probability;
	row[rh->element[0].index] -= rh->probability;
    }
    for(int i=0; i<G.size(); i++) {
      if(row[i] != 0) InpMtx_inputRealEntry(P, lh, i, row[i]);
    }
  }
  //gsl_permutation *p = gsl_permutation_alloc(G.size());
  //int signum;
  //gsl_linalg_LU_decomp(P, p, &signum);
  //gsl_matrix *R = gsl_matrix_alloc(G.size(), G.size());
  //gsl_linalg_LU_invert(P, p, R);
  //gsl_matrix_free(P);
  //gsl_permutation_free(p);
  Bridge *bridge = Bridge_new();
  Bridge_setMatrixParams(bridge, 
			 grammar.size(), 
			 SPOOLES_REAL, 
			 SPOOLES_NONSYMMETRIC);
  cerr << "setting up" << endl;
  int rc = Bridge_setup(bridge, P);
  Bridge_setMessageInfo(bridge, 0, stderr);
  int errorcode = 5;
  cerr << "factoring" << endl;
  rc = Bridge_factor(bridge, P, 1, &errorcode);
  cerr << "rc " << rc << " errorcode " << errorcode << endl; 
  DenseMtx *R = DenseMtx_new();
  cerr << "size is " << G.size() << endl;
  DenseMtx_init(R, SPOOLES_REAL, 0, 0, G.size(), G.size(), 1, G.size());
  DenseMtx_zero(R);
  DenseMtx *I = DenseMtx_new();
  DenseMtx_init(I, SPOOLES_REAL, 0, 0, G.size(), 1, 1, G.size());
  cerr << "allocated" << endl;
  int mod = G.size()/78;
  for(int i=0; i<G.size(); i++) {
    for(int j=0; j<G.size(); j++)
      if(i==j) DenseMtx_setRealEntry(I, j, 0, 1);
      else DenseMtx_setRealEntry(I, j, 0, 0);
    rc = Bridge_solve(bridge, 1, I, I);
    if (!(i%mod)) cerr << '.';
    for(int j=0; j<G.size(); j++) {
      double val;
      DenseMtx_realEntry(I, j, 0, &val);
      DenseMtx_setRealEntry(R, j, i, val);
    }
  }
  cerr << endl;
  cerr << "inverse rc " << rc << " errorcode " << errorcode << endl; 
  InpMtx_free(P);

  cerr << "probabilistic Early parse for initial/final substrings..." << endl;
  //compute initial substrings of length N-1
  for(int nt=0; nt<G.size(); nt++) {
    vector<ProbEarlyState> probEarlyParse;
    RHS dotS;
    dotS.element.push_back(RHSe(nt, G[nt].name));
    probEarlyParse.push_back(ProbEarlyState(0, 0, -1, dotS, 0, 1, 1));
    parseNext(nt, G, R, probEarlyParse, answer, vector<int>(), 0);
  }
  DenseMtx_free(R);
  return answer;
}

void NGram::parseNext(int nt,
		      const vector<LHS>& G, 
		      DenseMtx *R,
		      vector<ProbEarlyState>& probEarlyParse,
		      vector<map<vector<int>, double> >& answer,
		      vector<int> workingGram,
		      int n) {
  predict(probEarlyParse, n, R, G);
  set<int> word = generate(probEarlyParse, n);
  int branch = probEarlyParse.size();
  for(set<int>::const_iterator i=word.begin(); i!=word.end(); i++) {
    workingGram.push_back(*i);
    scan(probEarlyParse, n, *i);
    double alpha = 0;
    for(int ii=probEarlyParse.size()-1; probEarlyParse[ii].state == n+1; ii--)
      alpha += probEarlyParse[ii].alpha;
    answer[nt][workingGram] = alpha;
    if(N>n+2) {
      completion(probEarlyParse, n+1);
      parseNext(nt, G, R, probEarlyParse, answer, workingGram, n+1);
    }
    workingGram.pop_back();
    probEarlyParse.resize(branch);
  }
}

/*
void NGram::printUnigrams(ostream& out) const {
  double sum = 0;
  for(int i=0; i<tmap.size(); i++)
    sum += gsl_vector_get(unigramc, i);
  for(map<string, int>::const_iterator i = tmap.begin();
      i != tmap.end(); 
      i++)
    out << gsl_vector_get(unigramc, i->second)/sum << " -> [" 
	<< i->first << ']' << endl;
}

void NGram::printBigrams(ostream& out) const {
  //we need to write integer counts, so we have to find a scaling factor
  double smallest, largest;
  smallest = largest = gsl_matrix_get(bigramc, 0, 0);
  for(int i=0; i<tmap.size(); i++)
    for(int j=0; j<tmap.size(); j++) {
      double test = gsl_matrix_get(bigramc, i, j);
      if(test < smallest)
	smallest = test;
      if(test > largest)
	largest = test;
    }
  
  double scale = 200000/largest;
  for(map<string, int>::const_iterator i = tmap.begin();
      i != tmap.end();
      i++)
    for(map<string, int>::const_iterator j = tmap.begin();
	j != tmap.end();
	j++) {
      int pcount = (int) (gsl_matrix_get(bigramc, i->second, j->second) 
			  * scale);
      if(pcount != 0)
	out << i->first << ' ' << j->first << ' ' << pcount << endl;
    }
}
*/

ostream& operator<<(ostream& out, NGram& x) {
  cerr << "#grams=" << x.gram.size() << endl;
  queue<Gram> tmp;
  if(x.gram.empty()) return out;
  double sum = 0;
  while(!x.gram.empty()) {
    sum += x.gram.top().expectation;
    tmp.push(x.gram.top());
    x.gram.pop();
  }
  double scale = x.VIRTUALSIZE/sum;
  while(!tmp.empty()) {
    int virtualcount = (int) (tmp.front().expectation * scale);
    if(virtualcount) {
      for(vector<int>::const_iterator i=tmp.front().w.begin(); 
	  i!=tmp.front().w.end(); 
	  i++) {
	out << x.terminal[*i] << ' ';
      }
      out << virtualcount << endl;
    }
    x.gram.push(tmp.front());
    tmp.pop();
  }
  return out;
}
