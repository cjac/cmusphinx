#include <iostream>
#include <fstream>
#include <sstream>

#include <stdlib.h>
#include "PCFG.h"
#include "debug.h"

string usage(const char* program_name) {
  ostringstream s;
  s << "Usage: " << program_name << " {number of samples} {forms file} {grammar file}";
  return s.str();
}

int main(int argc, char* argv[]) {
  DebugStream::threashold_ = DebugStream::I;
  srand((unsigned)time(0));

  if(argc-1 < 3) {
    cerr << usage(argv[0]) << endl;
    exit(1);
  }

  unsigned int n; istringstream(argv[1]) >> n;
  ifstream forms(argv[2]);
  ifstream grammar(argv[3]);
  PCFG g(PCFG::readPhoenixGrammarAndForms(grammar, forms));
  info << "Got Phoenix forms and grammar file: " << endl 
       << g.stats() << endl;
  cout << g.generateSamples(n);
}
