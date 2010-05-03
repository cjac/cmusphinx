//-------------------------------------------------------------------------------------------
// Similarities.cc
//-------------------------------------------------------------------------------------------
// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//-------------------------------------------------------------------------------------------

#include "Similarities.h"

inline int 
minim(int a, int b)
{
  if (a<b)
    return a;
  else
    return b;
}

// //////////////////////////////////////////

double 
compute_phonetic_similarity(const string&  A1, const string& B1) 
{
  static vector<vector<unsigned short> > D;

  int AL;
  int BL;

  LineSplitter A, B;
  A.Split(A1);
  B.Split(B1);

  AL = A.NoWords();
  BL = B.NoWords();

  D.resize(AL+1);
  
  // initialize both axes

  for (int i = 0; i <= AL; i++) {
    D[i].resize(BL+1);
    D[i][0] = i;
  }
    
  for (int i = 0; i <= BL; i++) {
    D[0][i] = i;
  }

  // dynamic programming
  
  for (int i = 1; i <= AL; i++)
    for (int j = 1; j <= BL; j++){
      if (A[i-1] == B[j-1]){
	D[i][j] = minim(minim(D[i-1][j]+1, D[i][j-1]+1),D[i-1][j-1]);
      }
      else
        D[i][j] = minim(minim(D[i-1][j]+1,D[i][j-1]+1),D[i-1][j-1]+1);
    }
  
  double result = D[AL][BL];
  double final_result = 1 - result/(AL + BL);
  if (final_result == 0)
    final_result = 0.00001;

  return final_result;
}

// //////////////////////////////////////////////////////////////

double 
compute_word_similarity(const string&  A, const string& B) 
{
  static vector<vector<unsigned short> > D;

  int AL = A.size();
  int BL = B.size();

  D.resize(AL+1);
  for (int i = 0; i <= AL; i++) {
    D[i].resize(BL+1);
    D[i][0] = i;
  }
    
  for (int i = 0; i <= BL; i++) {
    D[0][i] = i;
  }
  
  for (int i = 1; i <= AL; i++)
    for (int j = 1; j <= BL; j++){
      if (A[i-1] == B[j-1]){
        D[i][j] = minim(minim(D[i-1][j]+1, D[i][j-1]+1),D[i-1][j-1]);
      }
      else
        D[i][j] = minim(minim(D[i-1][j]+1,D[i][j-1]+1),D[i-1][j-1]+1);
    }
  double result = D[AL][BL];
  
  double final_result = 1 - result/(AL + BL);
  if (final_result == 0)
    final_result = 0.00001;

  return final_result;
}

// //////////////////////////////////////////////////////////////


float 
compute_time_overlap(float p1, float p2, float q1, float q2){
    float e;
  
    if (p1 <= q1){
	if (p2 <= q1){
	  // p1 p2 q1 q2
	    e = 0;
	}
	else if (p2 <= q2){
	    // p1 q1 p2 q2
	    e = p2 - q1;
	}
	else{
	    // p1 q1 q2 p2
	    e = q2 - q1;
	}
    }
    else{
	if (q2 <= p1){
	  //q1 q2 p1 p2
	    e = 0;
	}	
	else if (q2 <= p2){
	  // q1 p1 q2 p2
	  e = q2 - p1;
	}
	else {
	  // q1 p1 p2 q2
	  e = p2 - p1;
	}
    }
    return 2*e / ( p2-p1 + q2-q1);
}
// ///////////////////////////////////////////////////////////////////////

// when there is no time info


float 
compute_time_overlap_ph(float p1, float p2, float q1, float q2){
  float e;
  
  if (p1 <= q1){
    if (p2 <= q1){
      // p1 p2 q1 q2
      return p2-q1;
    }
    else if (p2 <= q2){
      // p1 q1 p2 q2
      e = p2 - q1;
    }
    else{
      // p1 q1 q2 p2
      e = q2 - q1;
    }
  }
  else{
    if (q2 <= p1){
      //q1 q2 p1 p2
      return q2 - p1;
    }
    else if (q2 <= p2){
      // q1 p1 q2 p2
      e = q2 - p1;
    }
    else {
      // q1 p1 p2 q2
      e = p2 - p1;
    }
  }
  return 2*e / ( p2-p1 + q2-q1);
}
