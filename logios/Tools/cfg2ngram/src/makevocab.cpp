
#include <iostream>
#include <exception>

#include "PCFG.hpp"

int main(int argc, char* argv[]) {
  if(argc < 1) {
    cerr << "need root node" << endl;
    exit(1);
  }

  cerr << "reading phoenix and writing sorted vocab..." << endl;
  try {
    PCFG aPCFG = PCFG::readPhoenixGrammar(cin, argv[1]);
    aPCFG.writeVocab(cout);
  } catch (string err) {
    cerr << "Error in makevocab: " << err << endl;
  }
}
