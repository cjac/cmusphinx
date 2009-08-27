/**
 \file dmp2max.c
 \brief Tool for write maximums probabilities in a file
*/
/*
 HISTORY
 2008/07/08  N. Coetmeur, supervised by Y. Esteve
 Creating
*/


#include <stdlib.h>
#include "lm_max.h"


/**
  \brief Main function
*/
int main (int argc, char *argv[])
{
    lm_t *lm;
	int ret_val = 0;
	char * output_file;
	lm_max_t *ngrams_list;

	/* verify parameters */
	if (argc <= 1)
	{
		E_INFO (
		 "Usage: dmp2max input_DMP_file [output_MAX_file]\n\tIf no ouput file given: use input file with MAX extension.\n"
				);
		return -1;
	}
	if (argc >= 3)
		output_file = argv[2];
	else {
		/* no output file given, use the input file with 'MAX' extension */
		output_file = (char *) ckd_calloc (strlen(argv[1])+5, sizeof(char));
		if (output_file == NULL) {
			E_ERROR_SYSTEM ("Can't allocate memory for the ouput file name\n");
			return -2;
		}
		sprintf (output_file, "%s.MAX", argv[1]);
	}

	/* remove memory allocation restrictions */
    unlimit ();

    /* read language model */
	lm = lm_read_advance2(argv[1], "default", 1.0, 0.1, 1.0, 0, "DMP", 0, 1);
    if (lm == NULL) {
		if (argc<3) ckd_free (output_file);
		return -3;
	}
	if (lm->max_ng < 4) {
		E_ERROR ("You must use a quadrigram (or more) DMP file\n");
		lm_free (lm);
		if (argc<3) ckd_free (output_file);
		return -3;
	}
	if (! (lm->is32bits)) {
		/* force 32-bits version */
		E_INFOCONT ("\n");
		E_INFO ("Converting LM in 32-bits...\n\n\n");
		lm_convert_structure (lm, 1);
	}
	else
		E_INFOCONT ("\n\n");

	/* allocate memory for unigrams list */
	ngrams_list = lm_max_lists_new (lm->n_ng[0]);
	if (ngrams_list == NULL) {
		lm_free (lm);
		if (argc<3) ckd_free (output_file);
		return -2;
	}

	/* compute the maximums probabilities and back-off weights */
	if (lm_max_compute (lm, ngrams_list) != 0) {
        E_ERROR ("Fail to compute maximums\n");
		ret_val = -4;
	}
	else {
		E_INFOCONT ("\n");
		/* write maximums in output file */
		if (lm_max_write_dump (output_file, ngrams_list) != 0) {
			E_ERROR ("Fail to write maximums in %s file\n", argv[2]);
			ret_val = -5;
		}
	}

	/* free memory */
	lm_max_free (ngrams_list);
    lm_free (lm);
	if (argc < 3)
		ckd_free (output_file);

	return ret_val;
}
