
#ifndef __SPHINX3_CLASSIFIER_H
#define __SPHINX3_CLASSIFIER_H

#include "libutil/libutil.h"
#include "cont_mgau.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEP_LEN			13
#define VOTING_LEN		5
#define CLASS_LATENCY		2
#define CLASS_SILENCE_PROB      0.4f
#define CLASS_OWNER_PROB	0.4f
#define CLASS_SECONDARY_PROB	0.1f
#define CLASS_NOISE_PROB	0.1f


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
  float32 frame_gmm[NUM_CLASSES];
  int *classed_frames;
  int num_frames;
  int max_frames;
  int voting_frames[VOTING_LEN];
  int post_classify;
  float32 class_prior_prob[NUM_CLASSES] ; 

} classifier_t;

typedef struct {
  int *endpts;
  int num_endpts;
  int max_endpts;
  int state;
  int start_counter;
  int cancel_counter;
  int end_counter;
  int pad_cancel;
  int pad_leader;
  int pad_trailer;
} endptr_t;

int cl_init(classifier_t *_cl, char *_means_file,  char *_vars_file,
	    float64 _var_floor,  char *_mix_weights_file,
	    float64 _mix_weight_floor,  char *_gm_type, int _post_classify);
void cl_finish(classifier_t *_cl);
int cl_classify_frames(classifier_t *_cl, float32 **_frames, int _num_frames,
		       int **_classes);

void ep_init(endptr_t *_ep, int _pad_leader, int _pad_trailer,
	     int _pad_cancel);
void ep_finish(endptr_t *_ep);
int ep_endpoint(endptr_t *_ep, int *_classes, int _num_frames,
		int **_endpts);

#ifdef __cplusplus
}
#endif

#endif
