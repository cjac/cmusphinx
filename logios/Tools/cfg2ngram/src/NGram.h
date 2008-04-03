
#ifndef NGRAM_H
#define NGRAM_H

#include <map>
#include <queue>
#include <functional>
#include <set>

extern "C" {
#include <LinSol/Bridge.h>
}

#include "PCFG.hpp"

struct Gram {
  vector<int> w;
  float expectation;
  Gram(float e, vector<int> words) : w(words), expectation(e) {}
  Gram(float e, int w1) : w(1, w1), expectation(e) {}
  Gram(float e, int w1, int w2) : w(2), expectation(e) {w[0]=w1; w[1]=w2;}
  Gram(float e, int w1, int w2, int w3) : w(3), expectation(e) {
    w[0]=w1;
    w[1]=w2;
    w[2]=w3;
  }
  bool operator>(const Gram& x) const {
    return expectation > x.expectation;
  }
  bool operator<(const Gram& x) const {
    return w < x.w;
  }
};

typedef priority_queue< Gram, vector<Gram>, greater<Gram> > GramVec;
typedef priority_queue< Gram > LexGramVec;

class NGram : private PCFG {
private:
  int N;
  //gsl_vector *unigramc;
  //gsl_matrix *bigramc;
  //to save memory we only remember the x most probable ngrams
  static const long MAXNGRAMS = 20000000;
  static const long VIRTUALSIZE = 1000000000;
  GramVec gram;
  //map<string, int> terminal;
  //vector<string> rterminal;

public:
  NGram(const PCFG& aPCFG, int n = 2);
  
  ~NGram();
  
  float* logLikelihood(const vector<string>& phrase);

  friend ostream& operator<<(ostream& out, NGram& x);

  //void printUnigrams(ostream& out) const;
  //void printBigrams(ostream& out) const;
  //void printTrigrams(ostream& out) const;

  void smooth(double amt);

protected:
  struct ProbEarlyState {
    int state;
    int index;
    int nt;
    NGram::RHS rule;
    int dot;
    double alpha, gamma;
    ProbEarlyState() {}
    ProbEarlyState(int i1, 
		   int i2, 
		   int i3, 
		   RHS r1, 
		   int i4, 
		   double d1 = 0, 
		   double d2 = 0) :
      state(i1), index(i2), nt(i3), rule(r1), dot(i4), alpha(d1), gamma(d2) {}
  };

  void getNGram(Bridge* bridge, 
		DenseMtx *b, DenseMtx *c,
		const vector<map<vector<int>, double> >& init,
		const vector<map<vector<int>, double> >& final,
		vector<int> workingVec = vector<int>());
  void parseNext(int nt,
		 const vector<PCFG::LHS>& G, 
		 DenseMtx *R, 
		 vector<ProbEarlyState>& probEarlyParse,
		 vector<map<vector<int>, double> >& answer,
		 vector<int> workingGram,
		 int n);
  void predict(vector<ProbEarlyState>& parse, 
	       int state, 
	       //const gsl_matrix *R,
	       DenseMtx *R,
	       const vector<LHS>& G);
  void predict(vector<ProbEarlyState>& parse, 
	       int state, 
	       //const gsl_matrix *R,
	       DenseMtx *R,
	       const vector<LHS>& G,
	       int rnum);
  set<int> generate(const vector<ProbEarlyState>& parse, int state);
  void scan(vector<ProbEarlyState>& parse, int state, int input);
  void completion(vector<ProbEarlyState>& parse, int state);
  int findState(vector<ProbEarlyState>& parse,
		int state,
		int index,
		int nt,
		const RHS& rule,
		int dot);
  void printStates(const vector<ProbEarlyState>& parse,
		   const vector<LHS>& G) const;
  vector<map<vector<int>, double> > computeSubstrings(const vector<LHS>& G);
};

#endif
