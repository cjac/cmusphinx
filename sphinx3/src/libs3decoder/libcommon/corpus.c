/*
 * corpus.c -- Corpus-file related misc functions.
 * 
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 09-Dec-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added ctl_process_utt ().
 * 
 * 01-Mar-1999	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added check for already existing file extension in ctl_infile().
 * 
 * 23-Mar-1998	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added a general purpose data argument to ctl_process() and its function
 * 		argument func.
 * 
 * 22-Nov-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon
 * 		Added an optional validation function argument and an optional
 *		duplicate-resolution function argument to both corpus_load_headid() and
 * 		corpus_load_tailid().
 * 
 * 25-Oct-1997	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include "corpus.h"


corpus_t *corpus_load_headid (char *file,
			      int32 (*validate)(char *str),
			      int32 (*dup_resolve)(char *s1, char *s2))
{
    FILE *fp;
    char line[16384], wd[4096], *id;
    int32 j, k, m, n;
    corpus_t *corp;
    
    E_INFO("Loading corpus (%s)\n", file);
    
    if ((fp = fopen(file, "r")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,r) failed\n", file);

    corp = (corpus_t *) ckd_calloc (1, sizeof(corpus_t));
    
    n = 0;
    while (fgets (line, sizeof(line), fp) != NULL) {
	/* Skip empty lines */
	if (sscanf (line, "%s", wd) == 1)
	    n++;
    }
    rewind (fp);
    
    corp->ht = hash_new (n, HASH_CASE_YES);
    corp->n = 0;
    corp->str = (char **) ckd_calloc (n, sizeof(char *));
    
    n = 0;
    while (fgets (line, sizeof(line), fp) != NULL) {
	/* Skip blank lines */
	if (sscanf (line, "%s%n", wd, &k) != 1)
	    continue;
	
	/* Eliminate the line-terminating newline */
	j = strlen(line);
	if ((j > 0) && (line[j-1] == '\n'))
	    line[j-1] = '\0';

	/* Validate if a validation function is given */
	if (validate && (! (*validate)(line+k))) {
	    E_INFO("Corpus validation %s failed; skipping\n", wd);
	    continue;
	}
	
	id = ckd_salloc (wd);
	if ((m = hash_enter (corp->ht, id, n)) != n) {
	    /* Duplicate entry */
	    if (! dup_resolve)
		E_FATAL("corpus_load_headid(%s) failed; duplicate ID: %s\n", file, id);
	    else {
		/* Invoke the application provided duplicate resolver function */
		if ((j = (*dup_resolve)(corp->str[m], line+k)) < 0)
		    E_FATAL("corpus_load_headid(%s) failed; duplicate ID: %s\n", file, id);
		ckd_free (id);
		if (j > 0) {
		    /* Overwrite the original with the new entry */
		    ckd_free (corp->str[m]);
		    corp->str[m] = ckd_salloc (line+k);
		} else {
		    /* Retain the original entry, discard the new one */
		}
	    }
	} else {
	    /* Fill in new entry */
	    corp->str[n] = ckd_salloc (line+k);
	    n++;
	}
    }
    corp->n = n;
    
    fclose (fp);
    
    E_INFO("%s: %d entries\n", file, n);
    
    return corp;
}


static int32 sep_tailid (char *line, char *uttid)
{
    int32 i, k, l;
    
    l = strlen(line);
    uttid[0] = '\0';
    
    /* Find last close-paren */
    for (i = l-1;
	 (i >= 0) && ((line[i] == '\n') || (line[i] == ' ') || (line[i] == '\t'));
	 --i);
    if ((i < 0)	|| (line[i] != ')'))		/* Missing uttid */
	return -1;
    k = i;
    
    /* Find closest open-paren; no spaces allowed in uttid */
    for (--i; (i >= 0) && (line[i] != ' ') && (line[i] != '\t') && (line[i] != '('); --i);
    if ((i < 0) || (k-i < 2) || (line[i] != '('))	/* Empty or missing uttid */
	return -1;
    
    /* Remove parentheses and copy uttid */
    line[k] = '\0';
    strcpy (uttid, line+i+1);

    /* Strip uttid from line */
    line[i] = '\0';

    return 0;
}


corpus_t *corpus_load_tailid (char *file,
			      int32 (*validate)(char *str),
			      int32 (*dup_resolve)(char *s1, char *s2))
{
    FILE *fp;
    char line[16384], uttid[4096], *id;
    int32 j, m, n;
    corpus_t *corp;
    
    E_INFO("Loading corpus (%s)\n", file);
    
    if ((fp = fopen(file, "r")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,r) failed\n", file);

    corp = (corpus_t *) ckd_calloc (1, sizeof(corpus_t));
    
    n = 0;
    while (fgets (line, sizeof(line), fp) != NULL) {
	/* Skip empty lines */
	if (sscanf (line, "%s", uttid) == 1)
	    n++;
    }
    rewind (fp);
    
    corp->ht = hash_new (n, 0 /* Not no-case */);
    corp->n = 0;
    corp->str = (char **) ckd_calloc (n, sizeof(char *));
    
    n = 0;
    while (fgets (line, sizeof(line), fp) != NULL) {
	/* Skip blank lines */
	if (sscanf (line, "%s", uttid) < 1)
	    continue;
	
	/* Look for a (uttid) at the end */
	if (sep_tailid (line, uttid) < 0)
	    E_FATAL("corpus_load_tailid(%s) failed; bad line: %s\n", file, line);
	
	/* Validate if a validation function is given */
	if (validate && (! (*validate)(line))) {
	    E_INFO("Corpus validation %s failed; skipping\n", uttid);
	    continue;
	}
	
	id = ckd_salloc (uttid);
	if ((m = hash_enter (corp->ht, id, n)) != n) {
	    /* Duplicate entry */
	    if (! dup_resolve)
		E_FATAL("corpus_load_tailid(%s) failed; duplicate ID: %s\n", file, id);
	    else {
		/* Invoke the application provided duplicate resolver function */
		if ((j = (*dup_resolve)(corp->str[m], line)) < 0)
		    E_FATAL("corpus_load(tailid(%s) failed; duplicate ID: %s\n", file, id);
		ckd_free (id);
		if (j > 0) {
		    /* Overwrite the original with the new entry */
		    ckd_free (corp->str[m]);
		    corp->str[m] = ckd_salloc (line);
		} else {
		    /* Retain the original entry, discard the new one */
		}
	    }
	} else {
	    /* Fill in new entry */
	    corp->str[n] = ckd_salloc (line);
	    n++;
	}
    }
    corp->n = n;
    
    fclose (fp);
    
    E_INFO("%s: %d entries\n", file, n);

    return corp;
}


char *corpus_lookup (corpus_t *corp, char *id)
{
    int32 n;
    
    if (hash_lookup (corp->ht, id, &n) < 0)
	return NULL;

    assert ((n >= 0) && (n < corp->n));
    return (corp->str[n]);
}


#if _CORPUS_TEST_
main (int32 argc, char *argv[])
{
    corpus_t *ch, *ct;
    char id[4096], *str;
    
    if (argc != 3)
	E_FATAL("Usage: %s headid-corpusfile tailid-corpusfile\n", argv[0]);
    
    ch = corpus_load_headid (argv[1], NULL, NULL);
    ct = corpus_load_tailid (argv[2], NULL, NULL);
    for (;;) {
	printf ("> ");
	scanf ("%s", id);
	
	str = corpus_lookup (ch, id);
	if (str == NULL)
	    printf ("%s Not found in 1\n");
	else
	    printf ("%s(1): %s\n", id, str);

	str = corpus_lookup (ct, id);
	if (str == NULL)
	    printf ("%s Not found in 2\n");
	else
	    printf ("%s(2): %s\n", id, str);
    }
}
#endif


int32 ctl_read_entry (FILE *fp, char *uttfile, int32 *sf, int32 *ef, char *uttid)
{
    char line[16384];
    char base[16384];
    int32 k;
    
    do {
	if (fgets (line, sizeof(line), fp) == NULL)
	    return -1;
	if (line[0] == '#')
	    k = 0;
	else
	    k = sscanf (line, "%s %d %d %s", uttfile, sf, ef, uttid);
    } while (k <= 0);
    
    if ((k == 2) || ( (k >= 3) && ((*sf >= *ef) || (*sf < 0))) )
	E_FATAL("Error in ctlfile: %s\n", line);
    
    if (k < 4) {
	/* Create utt-id from mfc-filename (and sf/ef if specified) */
	path2basename (uttfile, base);
	strip_fileext (base, uttid);
	
	if (k == 3) {
	    k = strlen(uttid);
	    sprintf (uttid+k, "_%d_%d", *sf, *ef);
	} else {
	    *sf = 0;
	    *ef = -1;	/* Signifies "until EOF" */
	}
    }
    
    return 0;
}


ptmr_t ctl_process (char *ctlfile, int32 nskip, int32 count,
		    void (*func) (void *kb, char *uttfile, int32 sf, int32 ef, char *uttid),
		    void *kb)
{
  FILE *fp;
  char uttfile[16384], uttid[4096];
  int32 sf, ef;
  ptmr_t tm;
  
  if (ctlfile) {
    if ((fp = fopen(ctlfile, "r")) == NULL)
      E_FATAL_SYSTEM("fopen(%s,r) failed\n", ctlfile);
  } else
    fp = stdin;
  
  ptmr_init (&tm);
  
  if (nskip > 0) {
    E_INFO("Skipping %d entries at the beginning of %s\n", nskip, ctlfile);
    
    for (; nskip > 0; --nskip) {
      if (ctl_read_entry (fp, uttfile, &sf, &ef, uttid) < 0) {
	fclose (fp);
	return tm;
      }
    }
  }
  
  for (; count > 0; --count) {
    if (ctl_read_entry (fp, uttfile, &sf, &ef, uttid) < 0)
      break;
    
    /* Process this utterance */
    ptmr_start (&tm);
    if (func)
      (*func)(kb, uttfile, sf, ef, uttid);
    ptmr_stop (&tm);
    
    E_INFO("%s: %6.1f sec CPU, %6.1f sec Clk;  TOT: %8.1f sec CPU, %8.1f sec Clk\n\n",
	   uttid, tm.t_cpu, tm.t_elapsed, tm.t_tot_cpu, tm.t_tot_elapsed);
    
    ptmr_reset (&tm);
  }
  
  fclose (fp);
  
  return tm;
}


ptmr_t ctl_process_utt (char *uttfile, int32 count,
			void (*func) (void *kb, char *uttfile, int32 sf, int32 ef, char *uttid),
			void *kb)
{
    char uttid[4096];
    char base[16384];
    int32 i, c;
    int32 ts, newts;
    ptmr_t tm;
    
    ptmr_init (&tm);
    path2basename (uttfile, base);
    strip_fileext (base, uttid);
    strcpy (base, uttid);
    
    ts = -1;
    for (c = 0; c < count; c++) {
	/* Wait for uttfile to change from previous iteration */
	for (i = 0;; i++) {
	    newts = stat_mtime (uttfile);
	    if ((newts >= 0) && (newts != ts))
		break;
	    
	    if (i == 0)
		E_INFO("Waiting for %s\n", uttfile);
	    SLEEP_SEC (1);
	}
	ts = newts;
	
	/* Form uttid */
	sprintf (uttid, "%s_%08d", base, c);
	
	/* Process this utterance */
	ptmr_start (&tm);
	if (func)
	    (*func)(kb, uttfile, 0, -1, uttid);
	ptmr_stop (&tm);
	
	E_INFO("%s: %6.1f sec CPU, %6.1f sec Clk;  TOT: %8.1f sec CPU, %8.1f sec Clk\n\n",
	       uttid, tm.t_cpu, tm.t_elapsed, tm.t_tot_cpu, tm.t_tot_elapsed);
	
	ptmr_reset (&tm);
    }
    
    return tm;
}


void ctl_infile (char *file, char *dir, char *ext, char *utt)
{
    int32 l1, l2;
    
    assert (utt);
    
    if (ext && (ext[0] != '\0')) {
	l1 = strlen(ext);
	l2 = strlen(utt);
	if ((l2 > l1) && (utt[l2-l1-1] == '.') && (strcmp (utt+(l2-l1), ext) == 0))
	    ext = NULL;		/* utt already has the desired extension */
    }
    
    if ((utt[0] != '/') && dir) {
	/* Dir specified for relative uttfile pathname */
	if (ext && (ext[0] != '\0'))
	    sprintf (file, "%s/%s.%s", dir, utt, ext);
	else
	    sprintf (file, "%s/%s", dir, utt);
    } else {
	if (ext && (ext[0] != '\0'))
	    sprintf (file, "%s.%s", utt, ext);
	else
	    strcpy (file, utt);
    }
}


void ctl_outfile (char *file, char *dir, char *ext, char *utt, char *uttid)
{
    int32 k;
    
    k = strlen(dir);
    
    if ((k > 4) && (strcmp (dir+k-4, ",CTL") == 0)) {	/* HACK!! Hardwired ,CTL */
	if (utt[0] != '/') {
	    strcpy (file, dir);
	    file[k-4] = '/';
	    strcpy (file+k-3, utt);
	} else
	    strcpy (file, utt);
    } else {
	strcpy (file, dir);
	file[k] = '/';
	strcpy (file+k+1, uttid);
    }
    
    if (ext && (ext[0] != '\0')) {
	strcat (file, ".");
	strcat (file, ext);
    }
}
