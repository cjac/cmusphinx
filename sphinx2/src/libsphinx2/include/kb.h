/* 
 *------------------------------------------------------------*
 * DESCRIPTION
 *	Interface to Sphinx-II global knowledge base
 *-------------------------------------------------------------*
 * HISTORY
 * 
 * 27-May-97	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added Bob Brennan's declarations kbAddGrammar() and kb_get_personaldic().
 * 
 * 02-Apr-97	M K Ravishankar (rkm@cs) at Carnegie Mellon University
 * 		Added dict_maxsize().
 * 
 * Spring 89, Fil Alleva (faa) at Carnegie Mellon
 *	Created.
 */

#ifndef _KB_EXPORTS_H_
#define _KB_EXPORTS_H_

#include "lm_3g.h"
#include "msd.h"
#include "dict.h"

int32 kb_get_total_dists(void);
int32 kb_get_aw_tprob(void);
int32 kb_get_num_models(void);
int32 kb_get_num_dist(void);
int32 kb_get_num_model_instances(void);
int32 kb_get_num_words(void);
int32 kb_get_num_codebooks(void);
SMD  *kb_get_models(void);
char **kb_get_phone_list(void);
int32 *kb_get_codebook_0_dist(void);
int32 *kb_get_codebook_1_dist(void);
int32 *kb_get_codebook_2_dist(void);
int32 *kb_get_codebook_3_dist(void);
int32 kb_get_dist_prob_bytes(void);
int32 kb_get_start_word_id(void);
int32 kb_get_finish_word_id(void);
int32 kb_get_silence_word_id(void);
int32 **kb_get_word_transitions(void);
dictT *kb_get_word_dict(void);
LM     kb_get_lang_model(void);
int   kb_get_darpa_lm_flag(void);
int   kb_get_no_lm_flag(void);
char const *kb_get_lm_start_sym(void);
char const *kb_get_lm_end_sym(void);
char  *kb_get_word_str(int32 wid);
int32  kb_get_word_id(char const *word);
int32  dict_maxsize(void);
void   kbAddGrammar(char const *fileName, char const *grammarName);
char  *kb_get_dump_dir(void);
char  *kb_get_senprob_dump_file(void);
char  *kb_get_startsym_file(void);
int32  kb_get_senprob_size(void);
char  *kb_get_oovdic(void);
char  *kb_get_personaldic(void);
double kb_get_oov_ugprob(void);
int32  kb_get_max_new_oov(void);

void kb (int argc, char *argv[], float ip, float lw, float pip);


#endif /* _KB_EXPORTS_H_ */
