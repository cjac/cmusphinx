/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced
 * Research Projects Agency and the National Science Foundation of the
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 * gausubvq.c -- Sub-vector cluster Gaussian densities
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * $Log$
 * Revision 1.10  2006/02/24  03:54:43  arthchan2003
 * Merged from branch SPHINX3_5_2_RCI_IRII_BRANCH: Change commands to macro, add -logs3table.
 * 
 * Revision 1.9.4.2  2005/09/25 20:07:53  arthchan2003
 * Tied clustering arguments of gausubvq and gauvq.
 *
 * Revision 1.9.4.1  2005/07/18 23:21:23  arthchan2003
 * Tied command-line arguments with marcos
 *
 * Revision 1.9  2005/06/22 05:34:46  arthchan2003
 * Change gausubvq to use a new mdef_init interface. Add  keyword.
 *
 * Revision 1.3  2005/05/27 01:15:45  archan
 * 1, Changing the function prototypes of logs3_init to have another argument which specify whether an add table should be used. Corresponding changes have made in all executables and test programs. 2, Synchronzie how align, allphone, decode_anytopo, dag sets the default value of logbase.
 *
 * Revision 1.2  2005/03/30 00:43:40  archan
 *
 * 15-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Porting initial version to s3.2.
 */


#include <math.h>

#include <sphinxbase/info.h>
#include <sphinxbase/unlimit.h>
#include <sphinxbase/feat.h>

#include "cont_mgau.h"
#include "logs3.h"
#include "vector.h"
#include "cmdln_macro.h"

/** \file main_gausubvq.c 
 * \brief Compute the SVQ map 
 */

static arg_t arg[] = {

  /** Please see cmdln_macro.h */

    log_table_command_line_macro(),
    common_application_properties_command_line_macro(),
    gmm_command_line_macro(),        /* At here, mixw is involved, even though it is not used in the computation */
    vq_cluster_command_line_macro(),

  /** gausubvq-specific argument. */
    {"-seed",
     ARG_INT32,
     "-1",
     "User defined seed to seed the random generator of the K-mean algorithm, if it is a value smaller than 0, internal mechanism will be used."},
    {"-svspec",
     REQARG_STRING,
     NULL,
     "Subvectors specification (e.g., 24,0-11/25,12-23/26-38 or 0-12/13-25/26-38)"},
    {"-svqrows",
     ARG_INT32,
     "4096",
     "No. of codewords in output subvector codebooks"},
    {"-subvq",
     ARG_STRING,
     NULL,
     "Output subvq file (stdout if not specified)"},

    {NULL, ARG_INT32, NULL, NULL}
};


int32
main(int32 argc, char *argv[])
{
    FILE *fpout;
    mgau_model_t *mgau;
    int32 **subvec;
    int32 max_datarows, datarows, datacols, svqrows, svqcols;
    float32 **data, **vqmean;
    int32 *datamap, *vqmap;
    float64 sqerr;
    int32 stdev;
    int32 i, j, v, m, c;
    cmd_ln_t *config;
    logmath_t *logmath;

    print_appl_info(argv[0]);
    cmd_ln_appl_enter(argc, argv, "default.arg", arg);
    unlimit();

    config = cmd_ln_get();

    logmath = logs3_init(cmd_ln_float64_r(config, "-logbase"), 1, cmd_ln_int32_r(config, "-log3table"));      /*Report Progress, use log table */

    /* Load means/vars but DO NOT precompute variance inverses or determinants */
    mgau = mgau_init(cmd_ln_str_r(config, "-mean"),
                     cmd_ln_str_r(config, "-var"), 0.0 /* no varfloor */ ,
                     cmd_ln_str_r(config, "-mixw"), cmd_ln_float32_r(config, "-mixwfloor"), FALSE,  /* No precomputation */
                     ".cont.", MIX_INT_FLOAT_COMP, logmath);

    /* Parse subvector spec argument; subvec is null terminated; subvec[x] is -1 terminated */
    subvec = parse_subvecs(cmd_ln_str_r(config, "-svspec"));

    if (cmd_ln_str_r(config, "-subvq")) {
            if ((fpout = fopen(cmd_ln_str_r(config, "-subvq"), "w")) == NULL) {
                    E_ERROR_SYSTEM("Failed to open output file '%s'", fpout);
                    return 1;
            }
    }
    else
        fpout = stdout;

    /* Echo command line to output file */
    for (i = 0; i < argc - 1; i++)
        fprintf(fpout, "# %s \\\n", argv[i]);
    fprintf(fpout, "# %s\n#\n", argv[argc - 1]);

    /* Print input and output configurations to output file */
    for (v = 0; subvec[v]; v++);        /* No. of subvectors */
    svqrows = cmd_ln_int32_r(config, "-svqrows");
    fprintf(fpout, "VQParam %d %d -> %d %d\n",
            mgau_n_mgau(mgau), mgau_max_comp(mgau), v, svqrows);
    for (v = 0; subvec[v]; v++) {
        for (i = 0; subvec[v][i] >= 0; i++);
        fprintf(fpout, "Subvector %d length %d ", v, i);
        for (i = 0; subvec[v][i] >= 0; i++)
            fprintf(fpout, " %2d", subvec[v][i]);
        fprintf(fpout, "\n");
    }
    fflush(fpout);

    /*
     * datamap[] for identifying non-0 input vectors that take part in the clustering process:
     *     datamap[m*max_mean + c] = row index of data[][] containing the copy.
     * vqmap[] for mapping vq input data to vq output.
     */
    max_datarows = mgau_n_mgau(mgau) * mgau_max_comp(mgau);
    datamap = (int32 *) ckd_calloc(max_datarows, sizeof(int32));
    vqmap = (int32 *) ckd_calloc(max_datarows, sizeof(int32));

    stdev = cmd_ln_int32_r(config, "-stdev");

    /* Copy and cluster each subvector */
    for (v = 0; subvec[v]; v++) {
        E_INFO("Clustering subvector %d\n", v);

        for (datacols = 0; subvec[v][datacols] >= 0; datacols++);       /* Input subvec length */
        svqcols = datacols * 2; /* subvec length after concatenating mean + var */

        /* Allocate input/output data areas */
        data =
            (float32 **) ckd_calloc_2d(max_datarows, svqcols,
                                       sizeof(float32));
        vqmean =
            (float32 **) ckd_calloc_2d(svqrows, svqcols, sizeof(float32));

        /* Make a copy of the subvectors from the input data, and initialize maps */
        for (i = 0; i < max_datarows; i++)
            datamap[i] = -1;
        datarows = 0;
        for (m = 0; m < mgau_n_mgau(mgau); m++) {       /* For each mixture m */
            for (c = 0; c < mgau_n_comp(mgau, m); c++) {        /* For each component c in m */
                if (vector_is_zero
                    (mgau_var(mgau, m, c), mgau_veclen(mgau))) {
                    E_INFO("Skipping mgau %d comp %d\n", m, c);
                    continue;
                }

                for (i = 0; i < datacols; i++) {        /* Copy specified dimensions, mean+var */
                    data[datarows][i * 2] =
                        mgau->mgau[m].mean[c][subvec[v][i]];
                    data[datarows][i * 2 + 1] =
                        (!stdev) ? mgau->mgau[m].
                        var[c][subvec[v][i]] : sqrt(mgau->mgau[m].
                                                    var[c][subvec[v][i]]);
                }
                datamap[m * mgau_max_comp(mgau) + c] = datarows++;
            }
        }

        E_INFO("Sanity check: input data[0]:\n");
        vector_print(stderr, data[0], svqcols);

        for (i = 0; i < max_datarows; i++)
            vqmap[i] = -1;
#if 0
        {
            int32 **in;

            printf("Input data: %d x %d\n", datarows, svqcols);
            in = (int32 **) data;
            for (i = 0; i < datarows; i++) {
                printf("%8d:", i);
                for (j = 0; j < svqcols; j++)
                    printf(" %08x", in[i][j]);
                printf("\n");
            }
            for (i = 0; i < datarows; i++) {
                printf("%15d:", i);
                for (j = 0; j < svqcols; j++)
                    printf(" %15.7e", data[i][j]);
                printf("\n");
            }
            fflush(stdout);
        }
#endif
        /* VQ the subvector copy built above */
        sqerr = vector_vqgen(data, datarows, svqcols, svqrows,
                             cmd_ln_float64_r(config, "-eps"), cmd_ln_int32_r(config, "-iter"),
                             vqmean, vqmap, cmd_ln_int32_r(config, "-seed"));

        /* Output VQ */
        fprintf(fpout, "Codebook %d Sqerr %e\n", v, sqerr);
        for (i = 0; i < svqrows; i++) {
            if (stdev) {
                /* Convert clustered stdev back to var */
                for (j = 1; j < svqcols; j += 2)
                    vqmean[i][j] *= vqmean[i][j];
            }
            vector_print(fpout, vqmean[i], svqcols);
        }

        fprintf(fpout, "Map %d\n", v);
        for (i = 0; i < max_datarows; i += mgau_max_comp(mgau)) {
            for (j = 0; j < mgau_max_comp(mgau); j++) {
                if (datamap[i + j] < 0)
                    fprintf(fpout, " -1");
                else
                    fprintf(fpout, " %d", vqmap[datamap[i + j]]);
            }
            fprintf(fpout, "\n");
        }
        fflush(fpout);

        /* Cleanup */
        ckd_free_2d((void **) data);
        ckd_free_2d((void **) vqmean);
    }

    subvecs_free(subvec);
    ckd_free(datamap);
    ckd_free(vqmap);

    mgau_free(mgau);

    fprintf(fpout, "End\n");
    fclose(fpout);

    logmath_free(logmath);

    cmd_ln_free_r(config);
    exit(0);
}
