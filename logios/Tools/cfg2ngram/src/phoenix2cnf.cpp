
#include <iostream>
#include <fstream>

#include "PCFG.h"

int main(int argc, char* argv[]) {
  if(argc-1 < 1) {
    cerr << "need a forms file" << endl;
    exit(1);
  }

  cerr << "reading phoenix and converting to CNF..." << endl;
  ifstream forms(argv[1]);
  PCFG g(PCFG::readPhoenixGrammarAndForms(cin, forms));
  cout << PCFG::CNF(g);
}
