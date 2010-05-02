#include "PCFG.h"
#include "debug.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

int main(int argc, char* argv[]) {
  DebugStream::threashold_ = DebugStream::I;
  srand((unsigned)time(0)); 

  cerr << "reading cnf grammar..." << endl;
  PCFG aPCFG;
  cin >> aPCFG;
  cout << aPCFG.generateSamples(10);
}
