// print-main.h
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: riley@google.com (Michael Riley)
//
// \file
// Classes and functions to print out binary FSTs in simple text
// format used by AT&T (see
// http://www.research.att.com/projects/mohri/fsm/doc4/fsm.5.html).
// Includes helper function for fstprint.cc that templates the main on
// the arc type to support multiple and extensible arc types.

#ifndef FSTPRINTER_H_H__
#define FSTPRINTER_H_H__

#include <sstream>

#include "fst/fst.h"

namespace fst {

template <class A> class FstPrinter {
 public:
  typedef A Arc;
  typedef typename A::StateId StateId;
  typedef typename A::Label Label;
  typedef typename A::Weight Weight;

  FstPrinter(const Fst<A> &fst,
             const SymbolTable *isyms,
             const SymbolTable *osyms,
             const SymbolTable *ssyms,
             bool accep)
      : fst_(fst), isyms_(isyms), osyms_(osyms), ssyms_(ssyms),
        accep_(accep && fst.Properties(kAcceptor, true)), ostrm_(0) {}

  // Print Fst to an output strm
  void Print(ostream *ostrm, const string &dest) {
    ostrm_ = ostrm;
    dest_ = dest;
    StateId start = fst_.Start();
    if (start == kNoStateId)
      return;
    // initial state first
    PrintState(start);
    for (StateIterator< Fst<A> > siter(fst_);
         !siter.Done();
         siter.Next()) {
      StateId s = siter.Value();
      if (s != start)
        PrintState(s);
    }
  }

 private:
  // Maximum line length in text file.
  static const int kLineLen = 8096;

  void PrintId(int64 id, const SymbolTable *syms,
               const char *name) const {
    if (syms) {
      string symbol = syms->Find(id);
      if (symbol == "") {
        LOG(ERROR) << "FstPrinter: Integer " << id
                   << " is not mapped to any textual symbol"
                   << ", symbol table = " << syms->Name()
                   << ", destination = " << dest_;
        exit(1);
      }
      *ostrm_ << symbol;
    } else {
      *ostrm_ << id;
    }
  }

  void PrintStateId(StateId s) const {
     PrintId(s, ssyms_, "state ID");
  }

  void PrintILabel(Label l) const {
     PrintId(l, isyms_, "arc input label");
  }

  void PrintOLabel(Label l) const {
     PrintId(l, osyms_, "arc output label");
  }

  void PrintState(StateId s) const {
    bool output = false;
    for (ArcIterator< Fst<A> > aiter(fst_, s);
         !aiter.Done();
         aiter.Next()) {
      Arc arc = aiter.Value();
      PrintStateId(s);
      *ostrm_ << "\t";
      PrintStateId(arc.nextstate);
      *ostrm_ << "\t";
      PrintILabel(arc.ilabel);
      if (!accep_) {
        *ostrm_ << "\t";
        PrintOLabel(arc.olabel);
      }
      if (arc.weight != Weight::One())
        *ostrm_ << "\t" << arc.weight;
      *ostrm_ << "\n";
      output = true;
    }
    Weight final = fst_.Final(s);
    if (final != Weight::Zero() || !output) {
      PrintStateId(s);
      if (final != Weight::One()) {
        *ostrm_ << "\t" << final;
      }
      *ostrm_ << "\n";
    }
  }

  const Fst<A> &fst_;
  const SymbolTable *isyms_;     // ilabel symbol table
  const SymbolTable *osyms_;     // olabel symbol table
  const SymbolTable *ssyms_;     // slabel symbol table
  bool accep_;                   // print as acceptor when possible
  ostream *ostrm_;                // binary FST destination
  string dest_;                  // binary FST destination name
};

}  // namespace fst

#endif  // FSTPRINTER_H_H__
