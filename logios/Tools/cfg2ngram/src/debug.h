#ifndef DEBUG_H
#define DEBUG_H

#include <set>
#include <string>
#include <iostream>
using namespace std;

class DebugStream {
public: 
  enum Level {D, I, W, E, F};
protected:
  static set<string> types_;
  Level level_;
  string type_;
public:
  static Level threashold_;
  DebugStream(Level level=D, const string s=string()) : level_(level), type_(s) {};
  bool on(const string& s) {return types_.insert(s).second;}
  bool off(const string& s) {return types_.erase(s) > 0;}
  DebugStream operator()(const string& s) {
    DebugStream debug(*this);
    debug.type_ = s;
    if(!s.empty() && level_ >= threashold_ && types_.find(s) != types_.end()) 
      cerr << s << ": ";
    return debug;
  }
  template<class C> DebugStream& operator<<(C val) {
    if(level_ >= threashold_) 
      if(type_.empty() || types_.find(type_) != types_.end()) 
        cerr << val;
    return *this;
  } 
  DebugStream& operator<<(ostream& (*pfn)(ostream&)) {
    if(level_ >= threashold_)
      if(type_.empty() || types_.find(type_) != types_.end())
        cerr << pfn;
    return *this;
  }
};
extern DebugStream debug, info, warn, error, fatal;

#endif
    
