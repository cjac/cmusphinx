
#include <unistd.h>
#include <iostream>

#include "PCFG.h"

int main(int argc, char* argv[]) {
  if(argc+1 < 1) {
    cerr << "need a forms file" << endl;
    exit(1);
  }

  cerr << "reading phoenix and converting to CNF..." << endl;
  cout << PCFG::CNF(PCFG::readPhoenixGrammar(cin, argv[1]));
}
