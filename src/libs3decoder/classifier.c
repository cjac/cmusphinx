
#include "classifier.h"

int
cl_init(classifier_t *_cl, char *_means_file,  char *_vars_file,
	float64 _var_floor,  char *_mix_weights_file,
	float64 _mix_weight_floor, char *_gm_type, int _post_classify)
{
  int i;
  
  assert(_cl != NULL);

  _cl->max_frames = _cl->num_frames = 0;
  _cl->classed_frames = NULL;
  _cl->post_classify = _post_classify;
  for (i = 0; i < VOTING_LEN; i++) {
    _cl->voting_frames[i] = CLASS_SILENCE;
  }
  if (_cl->gmm != NULL) {
    mgau_free(_cl->gmm);
  }
  _cl->gmm = mgau_init(_means_file, _vars_file, _var_floor, _mix_weights_file,
		       _mix_weight_floor, TRUE, _gm_type, FULL_FLOAT_COMP);
  if (_cl->gmm == NULL) {
    return -1;
  }
  mgau_precomp_hack_log_to_float(_cl->gmm);

  return 0;
}

void 
cl_finish(classifier_t *_cl)
{
  assert(_cl != NULL);

  if (_cl->gmm != NULL) {
    mgau_free(_cl->gmm);
  }
  if (_cl->classed_frames != NULL) {
    ckd_free(_cl->classed_frames);
  }
  _cl->max_frames = _cl->num_frames = 0;
}

static void
cl_calc_frame_gmm(classifier_t *_cl, float32 *_frame)
{
  mgau_model_t *gmm;
  int gau_index;
  int class_index;
  int i;
  float32 adjust;
  float32 diff;
  float32 dsum;

  assert(_cl != NULL);
  assert(_cl->gmm != NULL);
  assert(_cl->gmm->n_mgau == NUM_CLASSES);

  /*
   * gmm - collection of Gaussian Mixture Models
   * _cl->frame_gmm[i] - final GMM probablity for class i
   * adjust - adjustment term for one Gaussian or (((2*pi)^n)*det(K))^(-0.5)
   * dsum - sum of vector to mean differences times the co-variance vector
   * diff - individual terms of vector to mean difference
   * gmm->mgau[i].mixw_f[j] - mixture weights for class i's j-th Gaussian
   */
  gmm = _cl->gmm;
  for (class_index = gmm->n_mgau - 1; class_index >= 0; class_index--) {
    _cl->frame_gmm[class_index] = 0.0;
    for (gau_index = gmm->mgau->n_comp - 1; gau_index >= 0; gau_index--) {
      adjust = gmm->mgau[class_index].lrd[gau_index];
      dsum = 0.0;
      for (i = 0; i < CEP_LEN; i++) {
	diff = _frame[i] - gmm->mgau[class_index].mean[gau_index][i];
	dsum = diff * diff * gmm->mgau[class_index].var[gau_index][i];
      }
      _cl->frame_gmm[class_index] +=
	gmm->mgau[class_index].mixw_f[gau_index] * adjust * exp(dsum * -0.5);
    }
  }
}

static void 
cl_post_classify(classifier_t *_cl)
{
  int i, j;
  int votes[NUM_CLASSES];
  int *voting_frames;
  int *classed_frames;
  int num_frames;
  int best_class;
  int best_votes;

  voting_frames = _cl->voting_frames;
  classed_frames = _cl->classed_frames;
  num_frames = _cl->num_frames;

  /* reset the totals */
  for (i = 0; i < NUM_CLASSES; votes[i++] = 0);
  /* tally up the votes from voting frames */
  for (i = 0; i < VOTING_LEN; votes[voting_frames[i++]]);

  for (i = 0; i < num_frames; i++) {
    /* subtract the vote from the oldest frame */
    votes[voting_frames[0]]--;
    /* shift the frames down */
    for (j = 0; j < VOTING_LEN - 1; j++) {
      voting_frames[j] = voting_frames[j + 1];
    }
    /* add the vote from the newest frame */
    votes[voting_frames[VOTING_LEN - 1] = _cl->classed_frames[i]]++;

    /* see who has the best votes.  default winner == SILENCE */
    best_class = CLASS_SILENCE;
    best_votes = votes[CLASS_SILENCE];
    for (j = 1; j < NUM_CLASSES; j++) {
      if (votes[j] > best_votes) {
	best_class = j;
	best_votes = votes[j];
      }
    }
    /* save the best class */
    _cl->classed_frames[i] = best_class;
  }
}

int
cl_classify_frames(classifier_t *_cl, float32 **_frames, int _num_frames,
		   int **_classes)
{
  int frame_index;
  int class_index;
  int best_class = CLASS_SILENCE;
  float32 best_prob = 0.0;
  float32 prob;

  assert(_cl != NULL);
  assert(_classes != NULL);

  if (_cl->max_frames < _num_frames) {
    /* free the old frame classification array */
    if (_cl->classed_frames != NULL) {
      ckd_free(_cl->classed_frames);
    }
    _cl->max_frames = _cl->num_frames = 0;

    /* allocate a new frame classification array */
    _cl->classed_frames = (int *)ckd_calloc(sizeof(int), _num_frames);
    if (_cl->classed_frames == NULL) {
      return -1;
    }
    _cl->max_frames = _num_frames;
  }
  _cl->num_frames = _num_frames;

  for (frame_index = 0; frame_index < _num_frames; frame_index++) {
    cl_calc_frame_gmm(_cl, _frames[frame_index]);
    for (class_index = NUM_CLASSES - 1; class_index >= 0; class_index--) {
      prob = _cl->frame_gmm[class_index] * class_prior_prob[class_index];
      if (best_prob < prob) {
	best_prob = prob;
	best_class = class_index;
      }
    }
    _cl->classed_frames[frame_index] = best_class;
  }

  if (_cl->post_classify == TRUE) {
    cl_post_classify(_cl);
  }

  return 0;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

enum {
  EP_STATE_IDLE,
  EP_STATE_LEADER,
  EP_STATE_CANCEL,
  EP_STATE_SPEECH,
  EP_STATE_TRAILER,
};

static void
ep_reset(endptr_t *_ep)
{
  _ep->num_endpts = 0;
  _ep->start_counter = 0;
  _ep->cancel_counter = 0;
  _ep->end_counter = 0;
}

void
ep_init(endptr_t *_ep, int _pad_leader, int _pad_trailer, int _pad_cancel)
{
  assert(_ep != NULL);

  ep_reset(_ep);
  _ep->endpts = NULL;
  _ep->max_endpts = 0;
  _ep->pad_leader = _pad_leader;
  _ep->pad_trailer = _pad_trailer;
  _ep->pad_cancel = _pad_cancel;
}

void
ep_finish(endptr_t *_ep)
{
  assert(_ep != NULL);

  ep_reset(_ep);
  if (_ep->endpts != NULL) {
    ckd_free(_ep->endpts);
  }
  _ep->endpts = NULL;
  _ep->max_endpts = 0;
}

int
ep_endpoint(endptr_t *_ep, int *_classes, int _num_frames, int **_endpts)
{
  int i;
  int *endpts;

  assert(_ep != NULL);
  assert(_endpts != NULL);

  if (_ep->max_endpts < _num_frames) {
    if (_ep->endpts != NULL) {
      ckd_free(_ep->endpts);
    }
    _ep->endpts = NULL;
    _ep->max_endpts = _ep->num_endpts = 0;

    if ((_ep->endpts = (int *)ckd_calloc(sizeof(int), _num_frames)) == NULL) {
      return -1;
    }
    _ep->max_endpts = _num_frames;
  }
  _ep->num_endpts = _num_frames;
  endpts = _ep->endpts;

  for (i = 0; i < _num_frames; i++) {
    switch (_ep->state) {
    case EP_STATE_IDLE:
      if (_classes[i] == CLASS_OWNER) {
	_ep->start_counter = 1;
	_ep->state = EP_STATE_LEADER;
	endpts[i] = EP_SILENCE;
      }
      else {
	endpts[i] = EP_MAYBE;
      }
      break;

    case EP_STATE_LEADER:
      if (_classes[i] == CLASS_OWNER) {
	if (++_ep->start_counter >= _ep->pad_leader) {
	  _ep->state = EP_STATE_SPEECH;
	  endpts[i] = EP_SPEECH;
	}
	else {
	  endpts[i] = EP_MAYBE;
	}
      }
      else {
	_ep->cancel_counter = 1;
	_ep->state = EP_STATE_CANCEL;
	endpts[i] = EP_MAYBE;
      }
      break;

    case EP_STATE_SPEECH:
      if (_classes[i] == CLASS_OWNER) {
	endpts[i] = EP_SPEECH;
      }
      else {
	_ep->end_counter = 1;
	_ep->state = EP_STATE_TRAILER;
	endpts[i] = EP_SPEECH;
      }
      break;

    case EP_STATE_TRAILER:
      if (_classes[i] == CLASS_OWNER) {
	_ep->state = EP_STATE_SPEECH;
	endpts[i] = EP_SPEECH;
      }
      else if (++_ep->end_counter >= _ep->pad_trailer) {
	_ep->state = EP_STATE_IDLE;
	endpts[i] = EP_SILENCE;
      }
      else {
	endpts[i] = EP_SPEECH;
      }
      break;

    case EP_STATE_CANCEL:
      _ep->start_counter++;
      if (_classes[i] == CLASS_OWNER) {
	_ep->state = EP_STATE_LEADER;
	endpts[i] = EP_MAYBE;
      }
      else if (++_ep->cancel_counter >= _ep->pad_cancel) {
	_ep->state = EP_STATE_IDLE;
	endpts[i] = EP_SILENCE;
      }
      else {
	endpts[i] = EP_MAYBE;
      }
      break;
    }
  }
  *_endpts = endpts;

  return 0;
}

