
#include <iostream>
#include <cstdlib>

#include "PCFG.hpp"

int main(int argc, char* argv[]) {
  if(argc < 1) {
    cerr << "need smoothing mass" << endl;
    exit(1);
  }

  cerr << "reading cnf grammar..." << endl;
  PCFG aPCFG;
  cin >> aPCFG;
  aPCFG.smooth(atof(argv[1]));
  cout << aPCFG;
}
