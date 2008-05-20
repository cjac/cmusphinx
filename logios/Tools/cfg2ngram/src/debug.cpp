#include "debug.h"

DebugStream::Level DebugStream::threashold_ = DebugStream::F;
set<string> DebugStream::types_ = set<string>();
DebugStream debug = DebugStream();
DebugStream info = DebugStream(DebugStream::I);
DebugStream warn = DebugStream(DebugStream::W);
DebugStream error = DebugStream(DebugStream::E);
DebugStream fatal = DebugStream(DebugStream::F);

