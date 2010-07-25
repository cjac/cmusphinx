
#include <assert.h>
#include <string.h>

#include <sphinxbase/ckd_alloc.h>

#include "logs3.h"
#include "s3_endpointer.h"

enum {
    CLASS_NOISE = 0,
    CLASS_OWNER,
    CLASS_SEC,
    CLASS_SIL,
    NUM_CLASSES
};

#define PRIOR_NOISE		0.4f
#define PRIOR_OWNER		0.4f
#define PRIOR_SEC		0.1f	
#define PRIOR_SIL		0.1f

enum {
    STATE_BEGIN_STREAM,
    STATE_UTT,
    STATE_UTT_NA,
    STATE_SIL,
    STATE_SIL_NA
};

#define VOTING_LEN		5
#define CEP_LEN			13
#define FRAME_LEN		(13 * sizeof(float32))

static void
get_frame_classes(s3_endpointer_t *_ep,
		  float32 **_frames,
		  int _n_frames,
		  int *_classes);

static void
init_frame_stats(s3_endpointer_t *_ep);

static void
update_frame_stats(s3_endpointer_t *_ep);

static int
update_available(s3_endpointer_t *_ep);

void
s3_endpointer_init(s3_endpointer_t *_ep,
		   const char *_means_file,
		   const char *_vars_file,
		   float64 _var_floor,
		   const char *_mix_weights_file,
		   float64 _mix_weight_floor,
		   const char *_gm_type,
		   int _post_classify,
		   int _begin_window,
		   int _begin_threshold,
		   int _begin_pad,
		   int _end_window,
		   int _end_threshold,
		   int _end_pad,
		   logmath_t *logmath)
{
    int i;

    assert(_ep != NULL);
    assert(_begin_threshold > 0 && _begin_threshold <= _begin_window);
    assert(_end_threshold > 0 && _end_threshold <= _end_window);

    _ep->frames = NULL;
    _ep->classes = NULL;
    _ep->n_frames = 0;
    _ep->offset = 0;
    _ep->count = 0;
    _ep->eof = 0;

    _ep->gmm = mgau_init(_means_file, _vars_file, _var_floor,
			 _mix_weights_file, _mix_weight_floor, TRUE, _gm_type,
			 MIX_INT_FLOAT_COMP, logmath);

    _ep->post_classify = _post_classify;

    _ep->priors = (int32 *)ckd_calloc(sizeof(int32), NUM_CLASSES);
    _ep->priors[CLASS_SIL] = logs3(logmath, PRIOR_SIL);
    _ep->priors[CLASS_OWNER] = logs3(logmath, PRIOR_OWNER);
    _ep->priors[CLASS_SEC] = logs3(logmath, PRIOR_SEC);
    _ep->priors[CLASS_NOISE] = logs3(logmath, PRIOR_NOISE);

    _ep->voters= (int *)ckd_calloc(sizeof(int), VOTING_LEN);
    for (i = 0; i < VOTING_LEN; i++)
	_ep->voters[i] = CLASS_SIL;

    _ep->state = STATE_BEGIN_STREAM;
    _ep->begin_window = _begin_window;
    _ep->begin_threshold = _begin_threshold;
    _ep->begin_pad = _begin_pad;
    _ep->begin_count = 0;
    _ep->end_window = _end_window;
    _ep->end_threshold = _end_threshold;
    _ep->end_pad = _end_pad;
    _ep->end_count = 0;
    _ep->end_countdown = -1;
    _ep->frames_required = 
	_ep->begin_pad + _ep->begin_window > _ep->end_window + 1?
	_ep->begin_pad + _ep->begin_window : _ep->end_window + 1;
}

void
s3_endpointer_close(s3_endpointer_t *_ep)
{
    assert(_ep != NULL);
    
    mgau_free(_ep->gmm);
    ckd_free_2d(_ep->frames);
    _ep->frames = NULL;
    _ep->classes = NULL;
    _ep->n_frames = 0;
    _ep->offset = 0;
    _ep->count = 0;
    _ep->eof = 0;
    _ep->end_countdown = -1;

    ckd_free(_ep->classes);
    ckd_free(_ep->priors);
    ckd_free(_ep->voters);
}

void
s3_endpointer_reset(s3_endpointer_t *_ep)
{
    assert(_ep != NULL);

    ckd_free_2d(_ep->frames);
    _ep->frames = NULL;
    _ep->classes = NULL;
    _ep->n_frames = 0;
    _ep->offset = 0;
    _ep->count = 0;
    _ep->eof = 0;
    _ep->state = STATE_SIL;
    _ep->end_countdown = -1;
}

void
s3_endpointer_feed_frames(s3_endpointer_t *_ep,
			  float32 **_frames,
			  int _n_frames,
			  int _eof)
{
    float32 **fbuf;
    int *cbuf;
    int i, sz, leftover;

    assert(_ep != NULL);

    if (_ep->n_frames > _ep->offset) {
	leftover = _ep->n_frames - _ep->offset;
	sz = _n_frames + leftover;
	fbuf = (float32 **)ckd_calloc_2d(sz, CEP_LEN, sizeof(float32));
	cbuf = (int *)ckd_calloc(sizeof(int), sz);
	for (i = 0; i < leftover; i++)
	    memcpy(fbuf[i], _ep->frames[_ep->offset + i], FRAME_LEN);
	memcpy(cbuf, &_ep->classes[_ep->offset], leftover * sizeof(int));
	for (i = leftover; i < sz; i++)
	    memcpy(fbuf[i], _frames[i - leftover], FRAME_LEN);
	get_frame_classes(_ep, _frames, _n_frames, &cbuf[leftover]);

	ckd_free_2d((void **)_ep->frames);
	ckd_free(_ep->classes);
	_ep->frames = fbuf;
	_ep->classes = cbuf;
	_ep->n_frames = sz;
	_ep->offset = 0;
    }
    else {
	fbuf = (float32 **)ckd_calloc_2d(_n_frames, CEP_LEN, sizeof(float32));
	cbuf = (int *)ckd_calloc(sizeof(int), _n_frames);
	for (i = 0; i < _n_frames; i++)
	    memcpy(fbuf[i], _frames[i], FRAME_LEN);
	get_frame_classes(_ep, _frames, _n_frames, cbuf);

	ckd_free_2d((void **)_ep->frames);
	ckd_free(_ep->classes);
	_ep->frames = fbuf;
	_ep->classes = cbuf;
	_ep->n_frames = _n_frames;
	_ep->offset = 0;
    }

    if (_ep->state == STATE_BEGIN_STREAM && update_available(_ep))
	init_frame_stats(_ep);

    _ep->eof = _eof;
}

int
s3_endpointer_read_utt(s3_endpointer_t *_ep, float32 **_frames, int _n_frames)
{
    int i;

    if (_ep->state == STATE_UTT_NA && update_available(_ep))
	update_frame_stats(_ep);

    if (_ep->state != STATE_UTT && _ep->state != STATE_UTT_NA)
	return -1;

    for (i = 0; i < _n_frames && _ep->state == STATE_UTT; i++) {
	memcpy(_frames[i], _ep->frames[_ep->offset], FRAME_LEN);
	update_frame_stats(_ep);
    }

    return i;
}

int
s3_endpointer_next_utt(s3_endpointer_t *_ep)
{
    while ((_ep->state == STATE_SIL) ||
	   (update_available(_ep) && _ep->state == STATE_SIL_NA))
	update_frame_stats(_ep);

    return _ep->state == STATE_UTT ? 1 : 0;
}

int
s3_endpointer_frame_count(s3_endpointer_t *_ep)
{
    return _ep->count;
}

static int
update_available(s3_endpointer_t *_ep)
{
    if (_ep->eof)
	return _ep->n_frames > _ep->offset;
    else
	return _ep->n_frames - _ep->offset >= _ep->frames_required;
}

static void
init_frame_stats(s3_endpointer_t *_ep)
{
    int i;

    _ep->state = STATE_SIL;
    _ep->begin_count = 0;
    _ep->end_count = 0;
    for (i = 0; i < _ep->begin_window; i++)
	if (_ep->classes[i] == CLASS_OWNER)
	    _ep->begin_count++;

    for (i = 0; i < _ep->end_window; i++)
	if (_ep->classes[i] == CLASS_OWNER)
	    _ep->end_count++;

    if (_ep->begin_count >= _ep->begin_threshold) {
	_ep->state = STATE_UTT;
	_ep->begin_countdown = 0;
    }

    for (i = 0; i < _ep->begin_pad; i++) {
	if (_ep->classes[i] == CLASS_OWNER)
	    _ep->begin_count--;
	if (_ep->classes[_ep->begin_window + i] == CLASS_OWNER)
	    _ep->begin_count++;
	
	if (_ep->state != STATE_UTT && 
	    _ep->begin_count >= _ep->begin_threshold) {
	    _ep->state = STATE_UTT;
	    _ep->begin_countdown = i + 1;
	}
    }
}

static void
update_frame_stats(s3_endpointer_t *_ep)
{
    int bb, be, eb, ee;

    bb = _ep->offset + _ep->begin_pad - 1;
    be = _ep->offset + _ep->begin_pad + _ep->begin_window - 1;
    eb = _ep->offset;
    ee = _ep->offset + _ep->end_window;

    if (update_available(_ep)) {
	if (_ep->classes[bb] == CLASS_OWNER)
	    _ep->begin_count--;
	if (_ep->classes[be] == CLASS_OWNER)
	    _ep->begin_count++;
	if (_ep->classes[eb] == CLASS_OWNER)
	    _ep->end_count--;
	if (_ep->classes[ee] == CLASS_OWNER)
	    _ep->end_count++;
    }
    else {
	if (_ep->state == STATE_UTT)
	    _ep->state = STATE_UTT_NA;
	else if (_ep->state == STATE_SIL)
	    _ep->state = STATE_SIL_NA;
	return;
    }

    if (_ep->end_count < 0 || _ep->end_count > _ep->end_window ||
	_ep->begin_count < 0 ||	_ep->begin_count > _ep->begin_window) {
	if (_ep->end_count < 0)
	    E_FATAL("End count less than zero\n");
	if (_ep->end_count > _ep->end_window)
	    E_FATAL("End count greater than end window\n");
	if (_ep->begin_count < 0)
	    E_FATAL("Begin count less than 0\n");
	if (_ep->begin_count > _ep->begin_window)
	    E_FATAL("Begin count greather than begin window\n");
    }
		
    _ep->offset++;
    _ep->count++;

    switch (_ep->state) {
    case STATE_UTT_NA:
	_ep->state = STATE_UTT;

    case STATE_UTT:
	if (_ep->begin_countdown > 0)
	    _ep->begin_countdown--;
	else if (_ep->end_countdown > 0)
	    _ep->end_countdown--;
	else if (_ep->end_countdown == 0 ||
		 (_ep->eof && _ep->offset == _ep->n_frames))
	    _ep->state = STATE_SIL;
	else if (_ep->end_count < _ep->end_threshold) {
	    if (_ep->eof && _ep->offset + _ep->end_pad >= _ep->n_frames)
		_ep->end_countdown = _ep->n_frames - _ep->offset - 1;
	    else
		_ep->end_countdown = _ep->end_pad;
	}

	break;

    case STATE_SIL_NA:
	_ep->state = STATE_SIL;

    case STATE_SIL:
	if (_ep->begin_count >= _ep->begin_threshold) {
	    _ep->state = STATE_UTT;
	    _ep->end_countdown = -1;
	    if (_ep->eof && _ep->offset + _ep->begin_pad >= _ep->n_frames)
		_ep->begin_countdown = _ep->n_frames - _ep->offset - 1;
	    else
		_ep->begin_countdown = _ep->begin_pad;
	}

	break;
}

    }
static void
get_frame_classes(s3_endpointer_t *_ep,
		  float32 **_frames,
		  int _n_frames,
		  int *_classes)
{
    int i, c, k;
    int32 best_class, best_votes, best_score, score;
    int votes[NUM_CLASSES];
    int *voters;

    assert(_ep != NULL);
    assert(_classes != NULL);

    for (i = 0; i < _n_frames; i++) {
	best_score = S3_LOGPROB_ZERO;
	best_class = -1;
	for (c = 0; c < NUM_CLASSES; c++) {
	    score = _ep->priors[c];
	    score += mgau_eval(_ep->gmm, c, NULL, _frames[i], i, 0);
	    if (best_score < score) {
		best_score = score;
		best_class = c;
	    }
	}

	_classes[i] = best_class;
    }

    if (_ep->post_classify) {
	voters = _ep->voters;

	/* reset the totals and tally up the votes from the frames*/
	for (i = 0; i < NUM_CLASSES; votes[i++] = 0);
	for (i = 0; i < VOTING_LEN; i++)
	    votes[voters[i]]++;

	for (i = 0; i < _n_frames; i++) {
	    /* subtract the vote from the oldest frame */
	    votes[voters[0]]--;
	    /* shift the frames down */
	    for (k = 0; k < VOTING_LEN - 1; k++)
		voters[k] = voters[k + 1];

	    /* add the vote from the newest frame */
	    votes[voters[VOTING_LEN - 1] = _classes[i]]++;

	    /* re-tally the votes */
	    best_class = 0;
	    best_votes = votes[0];
	    for (k = 1; k < NUM_CLASSES; k++) {
		if (votes[k] > best_votes) {
		    best_class = k;
		    best_votes = votes[k];
		}
	    }

	    _classes[i] = best_class;
	}
    }

#if 0
    printf("%d:\t", count * 10); count++;
    for (i = 0; i < _n_frames; i++)
	printf(_classes[i] == CLASS_OWNER ? "O" : ".");
    printf("\n");
#endif

}
