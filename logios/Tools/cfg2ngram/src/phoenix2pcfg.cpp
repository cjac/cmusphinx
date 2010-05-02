#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "PCFG.h"
#include "debug.h"

string usage(const char* program_name) {
  ostringstream s;
  s << "Usage: " << program_name << " {forms file} {grammar file}";
  return s.str();
}

int main(int argc, char* argv[]) {
  DebugStream::threashold_ = DebugStream::D;

  if(argc-1 < 2) {
    cerr << usage(argv[0]) << endl;
    exit(1);
  }

  ifstream forms(argv[1]);
  ifstream grammar(argv[2]);
  PCFG g(PCFG::readPhoenixGrammarAndForms(grammar, forms));
  info << "Got Phoenix forms and grammar file: " << endl 
       << g.stats() << endl;
  cout << g;
}
