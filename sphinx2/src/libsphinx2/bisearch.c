/*
 * bisearch.c
 * 
 * HISTORY
 * 
 * 01-Jan-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created by an ANONYMOUS.
 */


/* You know, there's a bsearch() in the standard library. */
int 
bisearch(char *table, int num_entries, int unit,
	 int (*cmp_routine)(char const *, char const *),
	 char *item)
{
  register int low = 0, high = num_entries, mid, c;

  while (low < high)
  {
    mid = (low+high)/2;
    c  = cmp_routine (table + unit*mid, item);
    if (c == 0) return (mid);
    else if (c < 0) low = mid + 1;
    else high = mid;
  }
  return(0);
}


