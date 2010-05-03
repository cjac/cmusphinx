#ifndef _Prob_h_
#define _Prob_h_

#include <math.h>
#include <assert.h>

#define LSMALL  (-0.5E10)
#define LZERO   (-1.0E10)
#define MINEARG (-708.3)

typedef double LnProb;

/*
 * The sum of two log probs: LogPlus(x,y) = log(e^x + e^y)
 */

inline double LogPlus(double x,  double y)
{
  if (x<y)
    {
      double tmp = x;
      x=y;
      y=tmp;
    }
  double diff = y-x;
  
  if (diff< MINEARG)
    if (x < 0) return (x< LSMALL)? LZERO:x;
    else return x;
  else
    {
      double z = exp(diff);
      return x + log(1.0+z);
    }
}

///////////////////////////////////////////////////////

/* The difference of two log probs: LogMinus(x,y) = log(e^x - e^y)
 *
 */


inline double LogMinus(double x, double y)
{
 
  assert(x >= y);
  if (x=y){
    return LZERO;
  }
  else{
    double diff = y-x;
    if (diff< MINEARG)
      if (x < 0) return (x<LSMALL)?LZERO:x;
      else return x;
    else{
      double z = exp(diff);
      return x + log(1.0 - z);
    }
  }
}

#endif



