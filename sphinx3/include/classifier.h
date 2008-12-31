/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */

#ifndef __SPHINX3_CLASSIFIER_H
#define __SPHINX3_CLASSIFIER_H

#include "s3types.h"
#include "cont_mgau.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Fool Emacs into not indenting things. */
#endif
  
#define CEP_LEN			13
#define VOTING_LEN		5
#define CLASS_LATENCY		2
#define CLASS_SILENCE_PRIOR     0.4f
#define CLASS_OWNER_PRIOR	0.4f
#define CLASS_SECONDARY_PRIOR	0.1f
#define CLASS_NOISE_PRIOR	0.1f
  
  
enum {
    CLASS_NOISE = 0,
    CLASS_OWNER,
    CLASS_SECONDARY,
    CLASS_SILENCE,
    NUM_CLASSES
};

enum {
    EP_MAYBE = 0,
    EP_SILENCE,
    EP_SPEECH
};

typedef struct {
    mgau_model_t *gmm;
    s3cipid_t class_map[NUM_CLASSES];
    int32 frame_prob[NUM_CLASSES];
    int *cached_classes;
    int num_frames;
    int max_frames;
    int voting_frames[VOTING_LEN];
    int post_classify;
    int32 prior_prob[NUM_CLASSES] ; 

} classifier_t;

typedef struct {
    int *endpts;
    int num_endpts;
    int max_endpts;
    int state;
    int start_counter;
    int cancel_counter;
    int end_counter;
    int counter;

    int pad_cancel;
    int pad_leader;
    int pad_trailer;
} endptr_t;
  
int cl_init(classifier_t *_cl, char *_mdef_file, char *_means_file,  char *_vars_file,
	    float64 _var_floor,  char *_mix_weights_file,
	    float64 _mix_weight_floor,  char *_gm_type, int _post_classify);
void cl_finish(classifier_t *_cl);
void cl_calc_frame_prob(classifier_t *_cl, float32 *_frame);
int cl_classify_frames(classifier_t *_cl, float32 **_frames, int _num_frames,
		       int **_classes);

void ep_init(endptr_t *_ep, int _pad_leader, int _pad_trailer,
	     int _pad_cancel);
void ep_finish(endptr_t *_ep);
int ep_endpoint(endptr_t *_ep, int *_classes, int _num_frames,
		int **_endpts);

#if 0
{ /* Stop indent from complaining */
#endif
#ifdef __cplusplus
}
#endif

#endif
