
#include <iostream>

#include "PCFG.hpp"
#include "NGram.hpp"

int main(int argc, char* argv[]) {
  cerr << "reading probabilistic cnf grammar..." << endl;
  NGram *pNGram;
  {
    PCFG aPCFG;
    cin >> aPCFG;
    pNGram = new NGram(aPCFG, argc>1?atoi(argv[1]):2);
  }
  cout << *pNGram;
  return 0;
}
