#include <libutil/libutil.h>

main (int32 argc, char *argv[])
{
    int32 r, c;
    int32 i, j;
    
    if (argc != 3)
	E_FATAL("Usage: %s #rows #cols\n", argv[0]);
    
    sscanf (argv[1], "%d", &r);
    sscanf (argv[2], "%d", &c);
    
    printf ("%d %d\n", r, c);
    for (i = 0; i < r; i++) {
	for (j = 0; j < c; j++)
	    printf (" %.6f", drand48());
	printf ("\n");
    }
}
