#include <libutil/libutil.h>


static int32 pval[256][256];
static unsigned char pid[256][20000];


main (int32 argc, char *argv[])
{
    int32 i, c, f, n, nwt, nsen;
    char line[4096];
    FILE *fp;
    
    if (argc != 2)
	E_FATAL("Usage: %s <senfile>\n", argv[0]);
    
    if ((fp = fopen(argv[1], "rb")) == NULL)
	E_FATAL("fopen(%s,rb) failed\n", argv[1]);
    
    for (;;) {
	if (fread (&n, sizeof(int32), 1, fp) != 1)
	    E_FATAL("fread linelen failed\n");
	if (n == 0)
	    break;

	if (fread (line, sizeof(char), n, fp) != n)
	    E_FATAL("fread(line) failed\n");
    }
    
    if (fread (&nwt, sizeof(int32), 1, fp) != 1)
	E_FATAL("fread(nwt) failed\n");
    if (fread (&nsen, sizeof(int32), 1, fp) != 1)
	E_FATAL("fread(nsen) failed\n");
    
    for (f = 0; f < 4; f++) {
	for (c = 0; c < 256; c++) {
	    if (fread (pval[c], sizeof(int32), 256, fp) != 256)
		break;
	    if (fread (pid[c], 1, nsen, fp) != nsen)
		E_FATAL("fread(pid) failed\n");
	}
	
	for (i = 0; i < nsen; i++) {
	    printf ("%d %d\n", f, i);
	    
	    for (c = 0; c < 256; c++)
		printf (" %7d", pval[c][pid[c][i]]);
	    printf ("\n");
	}
    }
}
