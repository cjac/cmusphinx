
#include <iostream>
#include <fstream>
#include <strstream>

#include "PCFG.h"

int main(int argC, char* argv[]) {
  if(argc < 1) {
    cerr << "need training data" << endl;
    exit(1);
  }

  corpus trainingData;
  {
    ifstream datafile(argv[1]);
    char line[255];
    while(datafile.getline(line,255)) {
      sentence s;
      string word;
      istrstream iline(line,strlen(line));
      while(iline >> word) {
	s.push_back(word);
      }
      if(s.size() > 0)
	trainingData.push_back(s);
    } 
  }

  cerr << "reading cnf grammar..." << endl;
  PCFG aPCFG;
  cin >> aPCFG;
  aPCFG.train(trainingData);
  cout << aPCFG;
}
