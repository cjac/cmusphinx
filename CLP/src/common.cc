#include "common.h"
#include <cstdio>


// I used this one instead of the standard one because for some weird reason this one is faster

int 
getlineH(istream& istr, string& line) {
  static char temp[1000000];
  
  if(! istr.getline(temp, 1000000))
    return 0;
  line = temp;
  return 1;
}

// /////////////////////////////////////////////////////////////////////////

string 
itoa(int no) {
  static char s[20];
  sprintf(s, "%d", no);
  return s;
}

// /////////////////////////////////////////////////////////////////////////

