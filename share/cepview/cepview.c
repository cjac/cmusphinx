#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h> 

#define TRUE (1)
#define FALSE (0)
#define IO_ERR  (-1)

/* Macro to byteswap an int variable.  x = ptr to variable */
#define SWAP_INT(x)   *(x) = ((0x000000ff & (*(x))>>24) | \
                                (0x0000ff00 & (*(x))>>8) | \
                                (0x00ff0000 & (*(x))<<8) | \
                                (0xff000000 & (*(x))<<24))
/* Macro to byteswap a float variable.  x = ptr to variable */
#define SWAP_FLOAT(x) SWAP_INT((int *) x)

#define NUM_COEFF  (13)
#define DISPLAY_SIZE (10)
#define QUIT(X)  {printf X; fflush(stdout); exit(1);}

#define CMD_USAGE "Usage: %s [-i <# input coeff>] [-d <# display coeff>] <cepfile>\n"

char **alloc2d(int dim1, int dim2, int size);

int read_cep(char *file, float ***cep, int *nframes, int numcep);


main(int argc, char *argv[])
{

   int i, j, offset;
   int len, vsize, dsize, column;
   float *z, **cep;
   char cepfile[80];

   if (argc == 1)
      QUIT((CMD_USAGE, argv[0]))

   vsize = NUM_COEFF;
   dsize = DISPLAY_SIZE;
   for (i = 1; i < argc - 1; ++i)
   {
      if (argv[i][0] != '-')
         QUIT((CMD_USAGE, argv[0]))
      switch (argv[i][1])
      {
       case 'i': 
       case 'I':
	 vsize = atoi (argv[++i]);
	 break;
       case 'd':
       case 'D':
	 dsize = atoi (argv[++i]);
	 break;
       default:
         QUIT((CMD_USAGE, argv[0]))
	 break;
      }
   }

   strcpy(cepfile, argv[argc-1]);

   if (read_cep(cepfile,&cep,&len,vsize) == IO_ERR)
	QUIT(("ERROR opening %s for reading\n",cepfile))

   z = cep[0];
   printf("%d frames\n", len);

   offset = 0;
   column = (vsize > dsize) ? dsize : vsize;
   for (i = 0; i < len; ++i)
   {
      for ( j =0 ; j < column; ++j)
	 printf("%7.3f ", z[offset + j]);
      printf("\n");
      offset += vsize;
   }
}

int read_cep(char *file, float***cep, int *numframes, int cepsize)
{
    FILE *fp;
    int n_float;
    struct stat statbuf;
    int i, n, byterev, sf, ef;
    float **mfcbuf;

    if (stat(file, &statbuf) < 0) {
        printf("stat_retry(%s) failed\n", file);
        return -1;
    }

    if ((fp = fopen(file, "rb")) == NULL) {
	printf("fopen(%s,rb) failed\n", file);
	return -1;
    }
    
    /* Read #floats in header */
    if (fread(&n_float, sizeof(int), 1, fp) != 1) {
	fclose (fp);
	return -1;
    }
    
    /* Check if n_float matches file size */
    byterev = FALSE;
    if ((n_float*sizeof(float) + 4) != statbuf.st_size) {
	n = n_float;
	SWAP_INT(&n);

	if ((n*sizeof(float) + 4) != statbuf.st_size) {
	    printf("Header size field: %d(%08x); filesize: %d(%08x)\n",
		    n_float,n_float,(int)statbuf.st_size,(int)statbuf.st_size);
	    fclose (fp);
	    return -1;
	}

	n_float = n;
	byterev = TRUE;
    }
    if (n_float <= 0) {
	printf("Header size field: %d\n",  n_float);
	fclose (fp);
	return -1;
    }
    
    /* n = #frames of input */
    n = n_float/cepsize;
    if (n * cepsize != n_float) {
	printf("Header size field: %d; not multiple of %d\n", n_float, cepsize);
	fclose (fp);
	return -1;
    }
    sf = 0;
    ef = n;

    mfcbuf = (float **) alloc2d (n, cepsize, sizeof(float));
    
    /* Read mfc data and byteswap if necessary */
    n_float = n * cepsize;
    if (fread (mfcbuf[0], sizeof(float), n_float, fp) != n_float) {
	printf("Error reading mfc data\n");
	fclose (fp);
	return -1;
    }
    if (byterev) {
	for (i = 0; i < n_float; i++)
	    SWAP_FLOAT(&(mfcbuf[0][i]));
    }
    fclose (fp);

    *numframes = n;
    *cep = mfcbuf;
    return (1);
}

char          **alloc2d(int dim1,	/* "x" dimension */ 
			int dim2,	/* "y" dimension */
			int size	/* number of bytes each entry takes */
			)
{
	int             i;		/* loop control variable */
	unsigned        nelem;		/* total number of elements */

        char           *calloc();                                   
	char	       *p,		/* pointer to matrix memory */
		      **pp;		/* pointer to matrix mem table */

	/*
	 * Compute total number of elements needed for the two-dimensional
	 * matrix
	 */
	nelem = (unsigned) dim1 * dim2;

	/*
	 * Allocate the memory needed for the matrix
	 */
	p = calloc(nelem, (unsigned) size);

	/*
	 * If the allocation were not successful, return a NULL pointer
	 */
	if (p == NULL) return (NULL);

	/*
	 * Now allocate a table for referencing the matrix memory
	 */
	pp = (char **) calloc((unsigned) dim1, (unsigned) sizeof(char *));

	/*
	 * If the allocation were not successful, return a NULL pointer
	 * and free the previously allocated memory
	 */
	if (pp == NULL) {
		free(p);
		return (NULL);
	}

	/*
	 * Fill the table with locations to where each row begins
	 */
	for (i = 0; i < dim1; i++)
		pp[i] = p + (i * dim2 * size);

	return (pp);
}

void free2d(void **p)
{
   if (p!=NULL) {
      free(p[0]);
      free(p);
   }
   return;
}

