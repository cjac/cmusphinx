/* ====================================================================
 * Copyright (c) 1989-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * HISTORY
 * 11-Jun-89 Fil Alleva
 *	Fixed BUG that occured when $LPATH is not in the environment
 * 14-Oct-92 Eric Thayer (eht+@cmu.edu) Carnegie Mellon University
 *	added formal declaration of args to phone_to_id()
 * 14-Oct-92 Eric Thayer (eht+@cmu.edu) Carnegie Mellon University
 *	added type casts so that calls into hash.c use caddr_t so that
 *	code would work on an Alpha
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <fcntl.h>
#else
#include <sys/file.h>
#endif

#include "s2types.h"
#include "CM_macros.h"
#include "strfuncs.h"
#include "list.h"
#include "hash.h"
#include "phone.h"
#include "c.h"
#include "logmsg.h"

static hash_t phones;
static list_t phones_list;
static list_t phone_base_map;
static list_t phone_model_len;
static list_t phone_type_map;
static int32   numCiPhones = 0;	/* Number of context ind. phones */
static int32   numWdPhones = 0;	/* Number of word dependent phones */

static void  add_phone (char *phn, int32 id, int32 base_id, int32 type, int32 len);
static void  mk_phone_map ();
static int32 parse_triphone(char *instr, char *ciph, char *lc, char *rc, char *pc);

int
phone_read (char *filename)
/*------------------------------------------------------------*
 * read in the phone file filename
 *------------------------------------------------------------*/
{
    int32                 retval = 0;
    FILE               *fs = NULL;
    char                phone_str[1024];
    int32		phone_id = 0;
    int32		base_id = 0;
    int32		field1;
    int32		field2;
    int32		model_len, j;
    int32		tmp_num_phones;

    /*
     * Find #phones and set hash and list tables size hints.
     * (Otherwise, the simple-minded PC malloc library goes berserk.)
     */
    fs = CM_fopen (filename, "r");
    for (j = 1;; j++)
	if (fgets (phone_str, sizeof(phone_str), fs) == NULL)
	    break;
    phones.size_hint = j;
    phones_list.size_hint = j;
    phone_base_map.size_hint = j;
    phone_model_len.size_hint = j;
    phone_type_map.size_hint = j;
    rewind(fs);
    
    while (EOF != fscanf (fs, "%s%d%d%d%d\n",
			  phone_str, &field1, &field2,
			  &base_id, &phone_id))
    {
	/*
 	 * field1 is 0 for ciPhones
	 */
	if (field1 == 0)
	    numCiPhones++;

	/*
	 * Phones marked with a '-2' account for more than one model
 	 */
	if (field1 == -2) {
	    numWdPhones++;
	    model_len = field2;
	}
	else
	    model_len = 1;

	if (model_len <= 0) {
	    printf ("WARNING %s(%d): %s has length %d\n", __FILE__, __LINE__,
		    phone_str, model_len);
	}

	add_phone (phone_str, phone_id, base_id, field1, model_len);
    }

    tmp_num_phones = phone_id;

    /*
     * Next availiable phone id
     */
    phone_id++;

    for (j = 0; j < tmp_num_phones; j++) {
	/*
	 * If the phone is one of these within word phones then
	 * create model_len-1 additional phones name ...(1) ...(2) and
  	 * and so on up to ...(model_len-1). It's these phones the
	 * word refers to for it pronunciation
	 */
	if (phone_type(j) == -2) {
	    int32 i;
   	    int32 model_len = phone_len (j);

	    for (i = 1; i < model_len; i++) {
		char tmpstr[256];

		sprintf (tmpstr, "%s(%d)", phone_from_id(j), i);
		add_phone (tmpstr, phone_id, j, 1000+i, 1);

		phone_id++;
	    }
	}
    }

    mk_phone_map();

    if (fs)
	fclose (fs);

    return (retval);
}

int32
phone_to_id (char const *phone_str, int verbose)
/*------------------------------------------------------------*
 * return the phone id for phone_str
 *------------------------------------------------------------*/
{
    static char const *rname = "phone_to_id";
    caddr_t  phone_id;

    if (hash_lookup (&phones, phone_str, &phone_id)) {
	if (verbose)
	    fprintf (stdout, "%s: did not find [%s]\n", rname, phone_str);
	return (NO_PHONE);
    }
	
    return ((int32) phone_id);
}

char const *phone_from_id (int32 phone_id)
/*------------------------------------------------------------*
 * return the string coresponding to phone_id if there is one
 *------------------------------------------------------------*/
{
    return ((char *) list_lookup (&phones_list, phone_id));
}

int32
phone_id_to_base_id (int32 phone_id)
{
    return ((int32) list_lookup (&phone_base_map, phone_id));
}

int32
phone_type (int32 phone_id)
{
    return ((int32) list_lookup (&phone_type_map, phone_id));
}

int32
phone_len (int32 phone_id)
{
    return ((int32) list_lookup (&phone_model_len, phone_id));
}

int32
phone_count (void)
{
    return phones.inuse;
}

int32
phoneCiCount (void)
{
    return numCiPhones;
}

int32
phoneWdCount (void)
{
    return numWdPhones;
}

list_t *phoneList (void)
{
    return &phones_list;
}

/*
 * Implement phonological rules
 */

#ifdef VERBOSE_PHONE_MAP
static char const *voc[] = {"IY","IH","EY","EH","AE","AH","AX", "IX","UW","UH",
			    "OW","AO","AA","AW","AY","OY","AXR","ER","R"};
static char const *ust[] = {"AX","IX","AXR"};
#endif


int32 *PhoneMap = 0;

static void mk_phone_map (void)
{
    int32 numPhones = phone_count();
    int32 numPhoneMappings = 0; /* will be bogus without VERBOSE_PHONE_MAP! */
    int32 pid;

#ifdef VERBOSE_PHONE_MAP
    int32 pid1, pid2;
    int32 numCiPhones = phoneCiCount();
    int32 numVocPhones = sizeof(voc)/sizeof(char *);
    int32 numUstPhones = sizeof(ust)/sizeof(char *);
    int32 op_id, res_id;
    char op[32], res[32];
#endif

    /* 
     * In case we get called twice.
     */
    if (PhoneMap)
	free (PhoneMap);

    PhoneMap = (int32 *) CM_calloc (numPhones, sizeof(int32));

    for (pid = 0; pid < numPhones; pid++)
	PhoneMap[pid] = pid;

#ifdef VERBOSE_PHONE_MAP
    for (pid = 0; pid < numCiPhones; pid++) {
	sprintf (op, "TD(%s,Y)e", phone_from_id(pid));
	sprintf (res, "JH(%s,Y)e", phone_from_id(pid));
	op_id = phone_to_id (op, FALSE);
	res_id = phone_to_id (res, FALSE);
	if ((op_id != NO_PHONE) && (res_id != NO_PHONE)) {
	    PhoneMap[op_id] = res_id;
	    numPhoneMappings++;
	}

	sprintf (op, "DD(%s,Y)e", phone_from_id(pid));
	sprintf (res, "JH(%s,Y)e", phone_from_id(pid));
	op_id = phone_to_id (op, FALSE);
	res_id = phone_to_id (res, FALSE);
	if ((op_id != NO_PHONE) && (res_id != NO_PHONE)) {
	    PhoneMap[op_id] = res_id;
	    numPhoneMappings++;
	}
    }

    for (pid1 = 0; pid1 < numVocPhones; pid1++) {
	for (pid2 = 0; pid2 < numUstPhones; pid2++) {
	    sprintf (op, "TD(%s,%s)e", voc[pid1], ust[pid2]);
	    sprintf (res, "DX(%s,%s)", voc[pid1], ust[pid2]);
	    op_id = phone_to_id (op, FALSE);
	    res_id = phone_to_id (res, FALSE);
	    if ((op_id != NO_PHONE) && (res_id != NO_PHONE)) {
	        PhoneMap[op_id] = res_id;
	        numPhoneMappings++;
	    }

	    sprintf (op, "DD(%s,%s)e", voc[pid1], ust[pid2]);
	    sprintf (res, "D(%s,%s)",  voc[pid1], ust[pid2]);
	    op_id = phone_to_id (op, FALSE);
	    res_id = phone_to_id (res, FALSE);
	    if ((op_id != NO_PHONE) && (res_id != NO_PHONE)) {
	        PhoneMap[op_id] = res_id;
	        numPhoneMappings++;
	    }
	}

	for (pid2 = 0; pid2 < numCiPhones; pid2++) {
	    sprintf (op, "TD(%s,%s)e", phone_from_id(pid2), voc[pid1]);
	    sprintf (res, "T(%s,%s)",  phone_from_id(pid2), voc[pid1]);
	    op_id = phone_to_id (op, FALSE);
	    res_id = phone_to_id (res, FALSE);
	    if ((op_id != NO_PHONE) && (res_id != NO_PHONE)) {
	        PhoneMap[op_id] = res_id;
	        numPhoneMappings++;
	    }

	    sprintf (op, "DD(%s,%s)e", phone_from_id(pid2), voc[pid1]);
	    sprintf (res, "D(%s,%s)", phone_from_id(pid2), voc[pid1]);
	    op_id = phone_to_id (op, FALSE);
	    res_id = phone_to_id (res, FALSE);
	    if ((op_id != NO_PHONE) && (res_id != NO_PHONE)) {
	        PhoneMap[op_id] = res_id;
	        numPhoneMappings++;
	    }

	    sprintf (op, "BD(%s,%s)e", phone_from_id(pid2), voc[pid1]);
	    sprintf (res, "B(%s,%s)", phone_from_id(pid2), voc[pid1]);
	    op_id = phone_to_id (op, FALSE);
	    res_id = phone_to_id (res, FALSE);
	    if ((op_id != NO_PHONE) && (res_id != NO_PHONE)) {
	        PhoneMap[op_id] = res_id;
	        numPhoneMappings++;
	    }

	    sprintf (op, "GD(%s,%s)e", phone_from_id(pid2), voc[pid1]);
	    sprintf (res, "G(%s,%s)", phone_from_id(pid2), voc[pid1]);
	    op_id = phone_to_id (op, FALSE);
	    res_id = phone_to_id (res, FALSE);
	    if ((op_id != NO_PHONE) && (res_id != NO_PHONE)) {
	        PhoneMap[op_id] = res_id;
	        numPhoneMappings++;
	    }

	    sprintf (op, "KD(%s,%s)e", phone_from_id(pid2), voc[pid1]);
	    sprintf (res, "K(%s,%s)", phone_from_id(pid2), voc[pid1]);
	    op_id = phone_to_id (op, FALSE);
	    res_id = phone_to_id (res, FALSE);
	    if ((op_id != NO_PHONE) && (res_id != NO_PHONE)) {
	        PhoneMap[op_id] = res_id;
	        numPhoneMappings++;
	    }

	    sprintf (op, "PD(%s,%s)e", phone_from_id(pid2), voc[pid1]);
	    sprintf (res, "P(%s,%s)", phone_from_id(pid2), voc[pid1]);
	    op_id = phone_to_id (op, FALSE);
	    res_id = phone_to_id (res, FALSE);
	    if ((op_id != NO_PHONE) && (res_id != NO_PHONE)) {
	        PhoneMap[op_id] = res_id;
	        numPhoneMappings++;
	    }
	}
    }
#endif /* VERBOSE_PHONE_MAP */
    log_info ("Using %d phonological mappings\n", numPhoneMappings);
}

void phone_add_diphones (void)
/*------------------------*
 * DESCRIPTION - figure out what the word begin/end diphones of intrest are
 *	         and add them to the phone set.
 */
{
    int32 	phone_cnt = phone_count();
    int32	pid;
    int32	ret;
    int32	new_phone_id = phone_cnt;
    char	tp[64];
    char 	ci[32], lc[64], rc[64], pc[64];


    for (pid = 0; pid < phone_cnt; pid++) {
	strcpy (tp, phone_from_id (pid));
	ret = parse_triphone (tp, ci, lc, rc, pc);

	if (ret == 4) {
	    switch (pc[0]) {
	    case 'b':
		sprintf (tp, "%s(%%s,%s)b", ci, rc);
		if (phone_to_id (tp, FALSE) == NO_PHONE) {
		    add_phone (tp, new_phone_id, phone_to_id(ci, TRUE),
			       PT_DIPHONE, 1);
		    new_phone_id++;
		}
		break;
	    case 'e':
		sprintf (tp, "%s(%s,%%s)e", ci, lc);
		if (phone_to_id (tp, FALSE) == NO_PHONE) {
		    add_phone (tp, new_phone_id, phone_to_id(ci, TRUE),
			       PT_DIPHONE, 1);
		    new_phone_id++;
		}
		break;
	    case 's':
		sprintf (tp, "%s(%%s,%%s)s", ci);
		if (phone_to_id (tp, FALSE) == NO_PHONE) {
		    add_phone (tp, new_phone_id, phone_to_id(ci, TRUE),
			       PT_DIPHONE_S, 1);
		    new_phone_id++;
		}
		break;
	    case '\0':
		break;
	    default:
		printf ("%s(%d): Unknown position context in %s == '%c'\n",
			__FILE__, __LINE__, tp, pc[0]);
		exit (-1);
	    }
	}
    }
    /*
     * Remake the phone map to account for the diphones
     */
    mk_phone_map ();

    printf ("%s(%d): added %d new begin/end word diphones\n", __FILE__,
	    __LINE__, new_phone_id - phone_cnt);
}

static void add_phone (char *phn, int32 id, int32 base_id, int32 type, int32 len)
{
    char *diphn = (char *) salloc (phn);

    hash_add (&phones, diphn, (caddr_t )id);
    list_add (&phones_list, (caddr_t )diphn, id);
    list_add (&phone_base_map, (caddr_t )base_id, id);
    list_add (&phone_model_len, (caddr_t )len, id);
    list_add (&phone_type_map, (caddr_t )type, id);
}

static int32 parse_triphone(char *instr, char *ciph, char *lc, char *rc, char *pc)
/*------------------------------------------------------------*
 * The ANSI standard scanf can't deal with empty field matches
 * so we have this routine.
 */
{
    char *cp, *lp;

    ciph[0] = '\0';
    lc[0] = '\0';
    rc[0] = '\0';
    pc[0] = '\0';

    /* parse ci-phone */
    for (lp = instr, cp = ciph; (*lp != '(') && (*lp != '\0'); lp++, cp++)
        *cp = *lp;
    *cp = '\0';
    if (*lp == '\0') {
	return 1;
    }

    /* parse leftcontext */
    for (lp++, cp = lc; (*lp != ',') && (*lp != '\0'); lp++, cp++)
        *cp = *lp;
    *cp = '\0';
    if (*lp == '\0') {
	return 2;
    }

    /* parse rightcontext */
    for (lp++, cp = rc; (*lp != ')') && (*lp != '\0'); lp++, cp++)
        *cp = *lp;
    *cp = '\0';
    if (*lp == '\0') {
	return 3;
    }

    /* parse positioncontext */
    for (lp++, cp = pc; (*lp != '\0'); lp++, cp++)
        *cp = *lp;
    *cp = '\0';
    return 4;    
}

int32 phone_map (int32 pid)
{
    return (PhoneMap[pid]);
}
