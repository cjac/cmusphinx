/* ====================================================================
 * Copyright (c) 1995-2004 Carnegie Mellon University.  All rights
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
 */
/*
 * misc.c -- Misc. routines (especially I/O) needed by many S3 applications.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * $Log$
 * Revision 1.2  2005/06/21  20:52:00  arthchan2003
 * 1, remove hyp_free, it is now in the implementation of dag.c , 2, add incomplete comments for misc.h. 3, Added $ keyword.
 * 
 * Revision 1.4  2005/06/03 05:46:19  archan
 * Refactoring across dag/astar/decode_anytopo.  Code is not fully tested.
 * There are several changes I have done to refactor the code across
 * dag/astar/decode_anyptop.  A new library called dag.c is now created
 * to include all routines that are shared by the three applications that
 * required graph operations.
 * 1, dag_link is now shared between dag and decode_anytopo. Unfortunately, astar was using a slightly different version of dag_link.  At this point, I could only rename astar'dag_link to be astar_dag_link.
 * 2, dag_update_link is shared by both dag and decode_anytopo.
 * 3, hyp_free is now shared by misc.c, dag and decode_anytopo
 * 4, filler_word will not exist anymore, dict_filler_word was used instead.
 * 5, dag_param_read were shared by both dag and astar.
 * 6, dag_destroy are now shared by dag/astar/decode_anytopo.  Though for some reasons, even the function was not called properly, it is still compiled in linux.  There must be something wrong at this point.
 * 7, dag_bestpath and dag_backtrack are now shared by dag and decode_anytopo. One important thing to notice here is that decode_anytopo's version of the two functions actually multiply the LM score or filler penalty by the language weight.  At this point, s3_dag is always using lwf=1.
 * 8, dag_chk_linkscr is shared by dag and decode_anytopo.
 * 9, decode_anytopo nows supports another three options -maxedge, -maxlmop and -maxlpf.  Their usage is similar to what one could find dag.
 *
 * Notice that the code of the best path search in dag and that of 2-nd
 * stage of decode_anytopo could still have some differences.  It could
 * be the subtle difference of handling of the option -fudge.  I am yet
 * to know what the true cause is.
 *
 * Some other small changes include
 * -removal of startwid and finishwid asstatic variables in s3_dag.c.  dict.c now hide these two variables.
 *
 * There are functions I want to merge but I couldn't and it will be
 * important to say the reasons.
 * i, dag_remove_filler_nodes.  The version in dag and decode_anytopo
 * work slightly differently. The decode_anytopo's one attached a dummy
 * predecessor after removal of the filler nodes.
 * ii, dag_search.(s3dag_dag_search and s3flat_fwd_dag_search)  The handling of fudge is differetn. Also, decode_anytopo's one  now depend on variable lattice.
 * iii, dag_load, (s3dag_dag_load and s3astar_dag_load) astar and dag seems to work in a slightly different, one required removal of arcs, one required bypass the arcs.  Don't understand them yet.
 * iv, dag_dump, it depends on the variable lattice.
 *
 * Revision 1.3  2005/03/30 01:22:47  archan
 * Fixed mistakes in last updates. Add
 *
 * 
 * 11-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sphinxbase/filename.h>

#include "misc.h"
#include "dag.h"


FILE *
ctlfile_open(char *file)
{
    FILE *fp;

    if (!file)
        E_FATAL("NULL file argument to ctlfile_open()\n");
    if ((fp = fopen(file, "r")) == NULL)
        E_FATAL("fopen(%s,r) failed\n", file);

    return fp;
}


void
ctlfile_close(FILE * ctlfp)
{
    fclose(ctlfp);
}




int32
ctlfile_next(FILE * fp, char *ctlspec, int32 * sf, int32 * ef, char *uttid)
{
    char line[1024];
    int32 k;
    char base[1024];

    *sf = 0;
    *ef = (int32) 0x7ffffff0;

    /* Read next non-comment or non-empty line */
    for (;;) {
        if (fgets(line, sizeof(line), fp) == NULL)
            return -1;

        if ((line[0] != '#') &&
            ((k =
              sscanf(line, "%s %d %d %s", ctlspec, sf, ef, uttid)) > 0))
            break;
    }


    switch (k) {
    case 1:
        path2basename(ctlspec, base);
        strcpy(uttid, base);
        break;

    case 2:
        E_FATAL("Bad control file line: %s\n", line);
        break;

    case 3:
        if ((*sf >= *ef) || (*sf < 0))
            E_FATAL("Bad control file line: %s\n", line);
        path2basename(ctlspec, base);
        sprintf(uttid, "%s_%d_%d", base, *sf, *ef);
        break;

    case 4:
        if ((*sf >= *ef) || (*sf < 0))
            E_FATAL("Bad control file line: %s\n", line);
        break;

    default:
        E_FATAL("Panic: How did I get here?\n");
        break;
    }

    return 0;
}


int32
argfile_load(char *file, char *pgm, char ***argvout)
{
    FILE *fp;
    char line[1024], word[1024], *lp, **argv;
    int32 len, n;

    E_INFO("Reading arguments from %s\n", file);

    if ((fp = fopen(file, "r")) == NULL) {
        E_ERROR("fopen(%s,r) failed\n", file);
        return -1;
    }

    /* Count #arguments */
    n = 1;                      /* Including pgm */
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] == '#')
            continue;

        lp = line;
        while (sscanf(lp, "%s%n", word, &len) == 1) {
            lp += len;
            n++;
        }
    }

    /* Allocate space for arguments */
    argv = (char **) ckd_calloc(n + 1, sizeof(char *));

    /* Create argv list */
    rewind(fp);
    argv[0] = pgm;
    n = 1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] == '#')
            continue;

        lp = line;
        while (sscanf(lp, "%s%n", word, &len) == 1) {
            lp += len;
            argv[n] = ckd_salloc(word);
            n++;
        }
    }
    argv[n] = NULL;
    *argvout = argv;

    fclose(fp);

    return n;
}




static srch_hyp_t *
nbestfile_parseline(char *sent)
{
    char *lp;
    srch_hyp_t *head, *tail, *hyp;
    char word[1024];
    int32 k, sf, len, lscr;

    head = tail = NULL;

    lp = sent;
    /* Parse T <score> */
    if ((sscanf(lp, "%s%d%n", word, &k, &len) != 2)
        || (strcmp(word, "T") != 0)) {
        E_ERROR("Bad sentence: %s\n", sent);
        return NULL;
    }
    lp += len;

    /* Parse A <score> */
    if ((sscanf(lp, "%s%d%n", word, &k, &len) != 2)
        || (strcmp(word, "A") != 0)) {
        E_ERROR("Bad sentence: %s\n", sent);
        return NULL;
    }
    lp += len;

    /* Parse L <score> */
    if ((sscanf(lp, "%s%d%n", word, &lscr, &len) != 2)
        || (strcmp(word, "L") != 0)) {
        E_ERROR("Bad sentence: %s\n", sent);
        return NULL;
    }
    lp += len;

    /* Parse each hyp word */
    while ((k = sscanf(lp, "%d%s%n", &sf, word, &len)) == 2) {
        lp += len;

        hyp = (srch_hyp_t *) ckd_calloc(1, sizeof(srch_hyp_t));
        hyp->word = (char *) ckd_salloc(word);
        hyp->sf = sf;
        hyp->lscr = lscr;       /* HACK!! Every entry has the TOTAL LM score */
        hyp->next = NULL;
        if (!head)
            head = hyp;
        else
            tail->next = hyp;
        tail = hyp;
    }

    if ((k > 0) || (sscanf(lp, "%s", word) > 0)) {
        E_ERROR("Bad sentence: %s\n", sent);
        hyp_free(head);
        return NULL;
    }

    return head;
}


void
nbestlist_free(srch_hyp_t ** hyplist, int32 nhyp)
{
    int32 i;

    for (i = 0; i < nhyp; i++)
        hyp_free(hyplist[i]);
    ckd_free(hyplist);
}


#define NBEST_HYP_MAX		4092

/*
 * Read an Nbest file and create an array of hypotheses (array of hyp_t lists).
 * Return value: #hypotheses read, -1 if unsuccessful.
 */
int32
nbestfile_load(char *dir, char *uttid, srch_hyp_t *** hyplist_out)
{
    char filename[1024];
    FILE *fp;
    srch_hyp_t **hyplist, *h, *nexth;
    char line[65535], *lp, word[1024];
    int32 k, nhyp, nfrm;

    *hyplist_out = NULL;        /* Initialize to illegal value */

    if ((!dir) || (!uttid) || (!hyplist_out)) {
        E_ERROR("nbestfile_load: NULL argument\n");
        return -1;
    }

    sprintf(filename, "%s/%s.nbest", dir, uttid);
    if ((fp = fopen(filename, "r")) == NULL) {
        E_ERROR("fopen(%s,r) failed\n", filename);
        return -1;
    }
    E_INFO("Reading nbest file %s\n", filename);

    hyplist =
        (srch_hyp_t **) ckd_calloc(NBEST_HYP_MAX, sizeof(srch_hyp_t *));
    nhyp = 0;

    /* Skip header comments in the file */
    nfrm = -1;
    while ((lp = fgets(line, sizeof(line), fp)) != NULL) {
        k = strlen(line) - 1;
        if (line[k] != '\n') {
            E_FATAL
                ("Line does not end with newline (increase sizeof(line)?):\n%s\n",
                 line);
        }
        if (line[0] != '#')
            break;
        if ((sscanf(line + 1, "%s%d", word, &k) == 2)
            && (strcmp(word, "frames") == 0))
            nfrm = k;
    }
    if (nfrm < 0) {
        E_ERROR("frames parameter missing in header in %s\n", filename);
        goto load_error;
    }
    if (!lp) {
        E_ERROR("Premature EOF(%s)\n", filename);
        goto load_error;
    }

    while ((line[0] == 'T') && (line[1] == ' ')) {
        if (nhyp >= NBEST_HYP_MAX)
            E_FATAL("Increase NBEST_HYP_MAX\n");

        if ((hyplist[nhyp] = nbestfile_parseline(line)) == NULL)
            goto load_error;

        nhyp++;

        /* Fill in end frame information in hyp */
        for (h = hyplist[nhyp]; h; h = nexth) {
            nexth = h->next;
            h->ef = nexth ? nexth->sf - 1 : nfrm - 1;
            if (h->ef >= nfrm) {
                E_ERROR
                    ("%s: Start frame value (%d) >= #frames in header (%d)\n",
                     filename, h->ef + 1, nfrm);
                goto load_error;
            }
        }

        if ((lp = fgets(line, sizeof(line), fp)) == NULL)
            break;
        k = strlen(line) - 1;
        if (line[k] != '\n') {
            E_FATAL
                ("Line does not end with newline (increase sizeof(line)?):\n%s\n",
                 line);
        }
    }

    if ((!lp) || (strncmp(line, "End", 3) != 0)) {
        E_ERROR("No End marker in %s\n", filename);
        goto load_error;
    }

    fclose(fp);
    *hyplist_out = hyplist;
    return nhyp;

  load_error:
    fclose(fp);
    nbestlist_free(hyplist, nhyp);
    return -1;
}
