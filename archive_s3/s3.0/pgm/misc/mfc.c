#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#if (! WIN32)
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/param.h>
#else
#include <fcntl.h>
#endif

#include <libutil/libutil.h>
#include <libio/libio.h>

main (int argc, char *argv[])
{
    struct stat statbuf;
    char *file;
    FILE *fp;
    int32 byterev;
    int32 n, n_float32, fr;
    float32 cep[13];
    
    if (argc < 2) {
	printf ("Usage: %s <mfcfile> [<numbered>]\n", argv[0]);
	exit(-1);
    }

    file = argv[1];
    if (stat (file, &statbuf) != 0) {
	E_ERROR("stat(%s) failed\n", file);
	return -1;
    }
    
    if ((fp = fopen(file, "rb")) == NULL) {
	E_ERROR("fopen(%s,rb) failed\n", file);
	return -1;
    }
    
    /* Read #floats in header */
    if (fread (&n_float32, sizeof(int32), 1, fp) != 1) {
	fclose (fp);
	return -1;
    }
    
    /* Check of n_float32 matches file size */
    byterev = FALSE;
    if ((n_float32*sizeof(float32) + 4) != statbuf.st_size) {
	n = n_float32;
	SWAP_INT32(&n);

	if ((n*sizeof(float32) + 4) != statbuf.st_size) {
	    E_ERROR("Header size field: %d(%08x); filesize: %d(%08x)\n",
		    n_float32, n_float32, statbuf.st_size, statbuf.st_size);
	    fclose (fp);
	    return -1;
	}

	n_float32 = n;
	byterev = TRUE;
    }
    if (n_float32 <= 0) {
	E_ERROR("Header size field: %d\n",  n_float32);
	fclose (fp);
	return -1;
    }
    if (byterev && (argc < 3))
	E_INFO("Byte-reversing %s\n", file);
    
    fr = 0;
    while (fread (cep, sizeof(float32), 13, fp) == 13) {
	if (byterev)
	    SWAP_FLOAT32(cep);
	if (argc > 2)
	    printf ("%4d ", fr);
	
	printf ("%8.3f\n", cep[0]);
	
	fr++;
    }
}
