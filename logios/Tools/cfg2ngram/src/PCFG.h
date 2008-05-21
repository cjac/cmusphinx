
#ifndef PCFG_H
#define PCFG_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <sstream>

using namespace std;

typedef vector<string> sentence;
typedef vector<sentence> corpus;

/*
class pcfgException : private exception {
private:
  string errstr;
public:
  pcfgException(const string& x) : exception(), errstr(x) {}
  string operator()() {return errstr;}
};
*/

template<class Elem> class Chart {
private:
  vector<Elem> buf;
  int N;
  
public:
  struct row {
    int i;
    Chart& c;
    row(int x, Chart& y): i(x), c(y) {}
    Elem& operator[](int j) {
      return c.buf[c.buf.size() - (((c.N-i)*(c.N-i+1))/2) + j-i];
    }
  };
  Chart<Elem>(int size = 0): buf((size*(size+1))/2), N(size) {}
  row operator[](int i) {
    row ret(i,*this);
    return ret;
  }
  vector< pair<Elem, Elem> > constituents(int i, int j) {
    vector< pair<Elem, Elem> > ret;
    for(int k=i; k<j; k++) {
      Elem a = (*this)[i][k];
      Elem b = (*this)[k+1][j];
      ret.push_back(pair<Elem, Elem>(a,b));
    }
    return ret;
  }
  int size() const {return N;}
};

class PCFG {
public:
  struct RHSe {
    bool terminal;
    string word;
    int index;
    RHSe(const string& x, bool y = true): terminal(y), word(x), index(-1)  {}
    RHSe(int x, const string& y, bool t = false)
      : terminal(t), word(y), index(x) {}
    bool operator==(const RHSe& x) const {
      return terminal == x.terminal && index == x.index;
    }
  };
  
  typedef iterator<input_iterator_tag, RHSe> rhs_iterator;

  struct RHS {
    double probability;
    double count;
    double ctheta;
    vector<RHSe> element;
    RHS() : probability(0) {}
    bool operator==(const RHS& x) const {return x.element == element;}
  };  
  
  struct LHS {
    string name;
    double count;
    vector<RHS> rule;
    LHS(const string& x): name(x) {}
    bool operator==(const LHS& x) const {return x.rule == rule;}
  };
  
protected:
  static const double defaultTH = 0.001;
  map<string, int> ntmap;
  vector<LHS> grammar;
  map<string, int> tmap;
  vector<string> terminal;
  int head;

public:
  /*** Constructors ****************************************/
  PCFG();

  /*** Factories/Adapters **********************************/
  static PCFG readPhoenixGrammar(istream& pGgrammar, const string headname=string());
  static PCFG readPhoenixGrammarAndForms(istream& pGrammar, istream& pForms);
  static PCFG CNF(const PCFG& g);
  static PCFG removeEpsilons(const PCFG& g);
  static PCFG removeUnitProductions(const PCFG& g);
  friend istream& operator>>(istream& in, PCFG& x);

  /*** Mutators ********************************************/
  int addNonTerm(const LHS& x);
  int addTerm(const string& x);
  int merge(const PCFG& x);
  int train(const corpus& traingData, double threshhold = defaultTH);
  void smooth(double amount);

  /*** Accessors *******************************************/
  friend ostream& operator<<(ostream& out, const PCFG& x);
  void writePhoenixGrammar(ostream& pGgrammar) const;
  static string printrule(vector<LHS>::const_iterator x, 
			  vector<RHS>::const_iterator y);
  void writeVocab(ostream& out) const;
  vector<bool> reachableNT() const; 
  vector<bool> reachableT() const;
  void reachable(int from, vector<bool>& already, vector<bool>& alreadyT) const;
  sentence generateSample() const;
  corpus generateSamples(unsigned int n) const;
  string stats() const; //returns statistics like grammar size, etc.
  
protected:
  void redoTMap();
  void rebuild_indexes();
  void reduce();
  void removeUnreachables();
  void printChart(Chart<map<int, double> >& c) const;
  template<class InIt>
    string printrule(InIt a,InIt b) const {
    ostringstream o;
    for(InIt i = a; i != b; i++) {
      o << (i->terminal? "t ": "nt ") << i->word << ' ';
    }
    return o.str();
  }

  //string printrule(rhs_iterator a, rhs_iterator b) const;
  string printrule(int i, int j) const;
  string printrule(int x, vector<RHS>::const_iterator y) const;
  RHSe shorten(const vector<RHSe>& r, int index=0);
  void initialize_counts();
  void initialize();
  void normalize(); //clean up and propagate probability from hand-written
};

ostream& operator<<(ostream& out, const sentence& x);
ostream& operator<<(ostream& out, const corpus& x);
ostream& operator<<(ostream& out, const vector<PCFG::LHS>& x);

#endif
