/* prime.c
 */

#include <math.h>
#include <s2types.h>

int32 primeNext (val)
/*--------------------------------------*
 * Decsription
 *	Return a prime number greater than or equal to val
 */
int32 val;
{
    int32                maxFactor;
    int32		i;

    do {
	maxFactor = sqrt ((double) val);

	for (i = 2; i <= maxFactor; i++)
	    if (((val / i) * i) == val)
		break;
	if (i > maxFactor)
	    break;
	val++;
    } while (1);

    return (val);
}

#ifdef MAIN

main ()
{
    int32                i = 0;

    while (1) {
	printf ("%d \n", i = primeNext (i));
	i++;
    }
}

#endif MAIN

