#include "PCFG.h"

#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
  cerr << "reading cnf grammar..." << endl;
  PCFG aPCFG;
  cin >> aPCFG;
  cout << aPCFG.generateCorpus(10);
}
