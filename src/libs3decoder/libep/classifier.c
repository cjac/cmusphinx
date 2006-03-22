
#include "classifier.h"
#include "mdef.h"
#include "logs3.h"

int
cl_init(classifier_t *_cl, char *_mdef_file, char *_means_file,  char *_vars_file,
	float64 _var_floor,  char *_mix_weights_file,
	float64 _mix_weight_floor, char *_gm_type, int _post_classify)
{
  int i;
  mdef_t *mdef;

  assert(_cl != NULL);

  mdef = mdef_init(_mdef_file, 1);
  if (mdef == NULL)
    return -1;

  _cl->max_frames = _cl->num_frames = 0;
  _cl->cached_classes = NULL;
  _cl->post_classify = _post_classify;

  for (i = 0; i < VOTING_LEN; i++)
    _cl->voting_frames[i] = CLASS_SILENCE;

  _cl->gmm = mgau_init(_means_file, _vars_file, _var_floor, _mix_weights_file,
		       _mix_weight_floor, TRUE, _gm_type, MIX_INT_FLOAT_COMP);
  if (_cl->gmm == NULL)
    return -1;

  _cl->prior_prob[CLASS_SILENCE] = logs3(CLASS_SILENCE_PRIOR);
  _cl->prior_prob[CLASS_OWNER] = logs3(CLASS_OWNER_PRIOR);
  _cl->prior_prob[CLASS_SECONDARY] = logs3(CLASS_SECONDARY_PRIOR);
  _cl->prior_prob[CLASS_NOISE] = logs3(CLASS_NOISE_PRIOR);

  _cl->class_map[CLASS_SILENCE] = mdef_ciphone_id(mdef, "SIL");
  _cl->class_map[CLASS_OWNER] = mdef_ciphone_id(mdef, "O");
  _cl->class_map[CLASS_SECONDARY] = mdef_ciphone_id(mdef, "S");
  _cl->class_map[CLASS_NOISE] = mdef_ciphone_id(mdef, "N");
  for (i = 0; i < NUM_CLASSES; i++)
    if (_cl->class_map[i] == BAD_S3CIPID)
      return -1;

  return 0;
}

void 
cl_finish(classifier_t *_cl)
{
  assert(_cl != NULL);

  if (_cl->gmm != NULL) {
    mgau_free(_cl->gmm);
    _cl->gmm = NULL;
  }
  if (_cl->cached_classes != NULL) {
    ckd_free(_cl->cached_classes);
    _cl->cached_classes = NULL;
  }
  _cl->max_frames = _cl->num_frames = 0;
}

void
cl_calc_frame_prob(classifier_t *_cl, float32 *_frame)
{
  mgau_model_t *gmm;
  int gau_index;
  int class_index;
  s3cipid_t *class_map;
  int cipid;
  int dim_index;

  mgau_t *mgau;
  float32 *means;
  float32 *vars;
  int32 *priors;

  float64 adjust;
  float64 diff;
  float64 dsum;

  int32 mixture;
  int32 gau;

  assert(_cl != NULL);
  assert(_cl->gmm != NULL);

  /* gmm - collection of Gaussian Mixture Models
   * _cl->frame_prob[i] - final GMM probablity for class i
   * adjust - adjustment term for one Gaussian or (((2*pi)^n)*det(K))^(-0.5)
   * dsum - sum of vector to mean differences times the co-variance vector
   * diff - individual terms of vector to mean difference
   * gmm->mgau[i].mixw_f[j] - mixture weights for class i's j-th Gaussian
   */
  gmm = _cl->gmm;
  class_map = _cl->class_map;
  priors = _cl->prior_prob;

  for (class_index = 0; class_index < NUM_CLASSES; class_index++) {
    mixture = S3_LOGPROB_ZERO;
    cipid = class_map[class_index];
    mgau = &gmm->mgau[cipid];

    for (gau_index = 0; gau_index < mgau->n_comp; gau_index++) {
      dsum = 0.0;
      adjust = mgau->lrd[gau_index];
      means = mgau->mean[gau_index];
      vars = mgau->var[gau_index];
      for (dim_index = 0; dim_index < CEP_LEN; dim_index++) {
	diff = _frame[dim_index] - means[dim_index];
	dsum += diff * diff * vars[dim_index];
      }

      /* yitao@cs 2005-12-21: dhuggins commented on the incorrectness of
       * multiplying dsum by -0.5 because the scaling has already been
       * taken into account in the variance.  VERIFY! */       
      gau = log_to_logs3(adjust - dsum / 2);
      mixture = logs3_add(mixture, mgau->mixw[gau_index] + gau);
    }

    _cl->frame_prob[class_index] = mixture + priors[class_index];
  }
}

static void 
cl_post_classify(classifier_t *_cl)
{
  int i, j;
  int votes[NUM_CLASSES];
  int *voting_frames;
  int *cached_classes;
  int num_frames;
  int best_class;
  int best_votes;

  voting_frames = _cl->voting_frames;
  cached_classes = _cl->cached_classes;
  num_frames = _cl->num_frames;

  /* reset the totals */
  for (i = 0; i < NUM_CLASSES; i++) {
    votes[i] = 0;
  }
  /* tally up the votes from voting frames */
  for (i = 0; i < VOTING_LEN; i++) {
    votes[voting_frames[i]]++;
  }

  for (i = 0; i < num_frames; i++) {
    /* subtract the vote from the oldest frame */
    votes[voting_frames[0]]--;
    /* shift the frames down */
    for (j = 0; j < VOTING_LEN - 1; j++) {
      voting_frames[j] = voting_frames[j + 1];
    }
    /* add the vote from the newest frame */
    votes[voting_frames[VOTING_LEN - 1] = _cl->cached_classes[i]]++;

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
    _cl->cached_classes[i] = best_class;
  }
}

int
cl_classify_frames(classifier_t *_cl, float32 **_frames, int _num_frames,
		   int **_classes)
{
  int frame_index;
  int class_index;
  int best_class = CLASS_SILENCE;
  int32 *frame_prob;
  int32 best_prob;
  int *cached_classes;

  assert(_cl != NULL);
  assert(_classes != NULL);
  
  frame_prob = _cl->frame_prob;
  cached_classes = _cl->cached_classes;

  if (_cl->max_frames < _num_frames) {
    /* free the old frame classification array */
    if (cached_classes != NULL) {
      ckd_free(cached_classes);
      _cl->cached_classes = NULL;
      _cl->max_frames = _cl->num_frames = 0;
    }

    /* allocate a new frame classification array */
    cached_classes = (int *)ckd_calloc(sizeof(int), _num_frames);
    if (cached_classes == NULL) {
      return -1;
    }
    _cl->max_frames = _num_frames;
    _cl->cached_classes = cached_classes;
  }
  _cl->num_frames = _num_frames;

  for (frame_index = 0; frame_index < _num_frames; frame_index++) {
    cl_calc_frame_prob(_cl, _frames[frame_index]);
    best_class = -1;
    best_prob = S3_LOGPROB_ZERO;
    for (class_index = NUM_CLASSES - 1; class_index >= 0; class_index--) {
      if (best_prob < frame_prob[class_index]) {
	best_prob = frame_prob[class_index];
	best_class = class_index;
      }
    }
    cached_classes[frame_index] = best_class;
  }

  if (_cl->post_classify == TRUE) {
    cl_post_classify(_cl);
  }

  *_classes = cached_classes;

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
  _ep->state = EP_STATE_IDLE;
  _ep->num_endpts = 0;
  _ep->counter = 0;
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
    _ep->endpts = NULL;
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
	_ep->state = EP_STATE_LEADER;
	_ep->counter = 1;
	endpts[i] = EP_MAYBE;
      }
      else {
	endpts[i] = EP_SILENCE;
      }
      break;

    case EP_STATE_LEADER:
      if (_classes[i] == CLASS_OWNER) {
	if (++_ep->counter >= _ep->pad_leader) {
	  _ep->state = EP_STATE_SPEECH;
	  endpts[i] = EP_SPEECH;
	}
	else {
	  endpts[i] = EP_MAYBE;
	}
      }
      else {
	_ep->state = EP_STATE_CANCEL;
	_ep->counter = 1;
	endpts[i] = EP_MAYBE;
      }
      break;

    case EP_STATE_SPEECH:
      if (_classes[i] == CLASS_OWNER) {
	endpts[i] = EP_SPEECH;
      }
      else {
	_ep->state = EP_STATE_TRAILER;
	_ep->counter = 1;
	endpts[i] = EP_SPEECH;
      }
      break;

    case EP_STATE_TRAILER:
      if (_classes[i] == CLASS_OWNER) {
	_ep->state = EP_STATE_SPEECH;
	endpts[i] = EP_SPEECH;
      }
      else if (++_ep->counter >= _ep->pad_trailer) {
	_ep->state = EP_STATE_IDLE;
	endpts[i] = EP_SILENCE;
      }
      else {
	endpts[i] = EP_SPEECH;
      }
      break;

    case EP_STATE_CANCEL:
      if (_classes[i] == CLASS_OWNER) {
	_ep->state = EP_STATE_LEADER;
	_ep->counter = 1;
	endpts[i] = EP_MAYBE;
      }
      else if (++_ep->counter >= _ep->pad_cancel) {
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

