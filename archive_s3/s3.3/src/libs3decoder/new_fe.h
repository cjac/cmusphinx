#ifndef _NEW_FE_H_
#define _NEW_FE_H_

#define int32 int

#define int16 short

typedef struct{
    float SAMPLING_RATE;
    int32 FRAME_RATE;
    float WINDOW_LENGTH;
    int32 FB_TYPE;
    int32 NUM_CEPSTRA;
    int32 NUM_FILTERS;
    int32 FFT_SIZE;
    float LOWER_FILT_FREQ;
    float UPPER_FILT_FREQ;
    float PRE_EMPHASIS_ALPHA;

    char *wavfile;
    char *cepfile;
    char *ctlfile;
    char *wavdir;
    char *cepdir;
    char *wavext;
    char *cepext;
    int32 input_format;
    int32 is_batch;
    int32 is_single;
    int32 blocksize;
    int32 verbose;
    
    
} param_t;


typedef struct{
    float sampling_rate;
    int32 num_cepstra;
    int32 num_filters;
    int32 fft_size;
    float lower_filt_freq;
    float upper_filt_freq;
    float **filter_coeffs;
    float **mel_cosine;
    float *left_apex;
    int32 *width;
}melfb_t;


typedef struct{
    float SAMPLING_RATE;
    int32 FRAME_RATE;
    int32 FRAME_SHIFT;
    float WINDOW_LENGTH;
    int32 FRAME_SIZE;
    int32 FFT_SIZE;
    int32 FB_TYPE;
    int32 NUM_CEPSTRA;
    float PRE_EMPHASIS_ALPHA;
    int16 *OVERFLOW_SAMPS;
    int32 NUM_OVERFLOW_SAMPS;    
    melfb_t *MEL_FB;
    int32 START_FLAG;
    int16 PRIOR;
    double *HAMMING_WINDOW;
    
} fe_t;



#define MEL_SCALE 1
#define LOG_LINEAR 2

/* Default values */
#define DEFAULT_SAMPLING_RATE 16000.0
#define DEFAULT_FRAME_RATE 100
#define DEFAULT_FRAME_SHIFT 160
#define DEFAULT_WINDOW_LENGTH 0.025625
#define DEFAULT_FFT_SIZE 512
#define DEFAULT_FB_TYPE MEL_SCALE
#define DEFAULT_NUM_CEPSTRA 13
#define DEFAULT_NUM_FILTERS 40
#define DEFAULT_LOWER_FILT_FREQ 133.33334
#define DEFAULT_UPPER_FILT_FREQ 6855.4976
#define DEFAULT_PRE_EMPHASIS_ALPHA 0.97
#define DEFAULT_START_FLAG 0

#define BB_SAMPLING_RATE 16000
#define DEFAULT_BB_FRAME_SHIFT 160
#define DEFAULT_BB_NUM_FILTERS 40
#define DEFAULT_BB_LOWER_FILT_FREQ 133.33334
#define DEFAULT_BB_UPPER_FILT_FREQ 6855.4976

#define NB_SAMPLING_RATE 8000
#define DEFAULT_NB_FRAME_SHIFT 80
#define DEFAULT_NB_NUM_FILTERS 31
#define DEFAULT_NB_LOWER_FILT_FREQ 200
#define DEFAULT_NB_UPPER_FILT_FREQ 3500


#define DEFAULT_BLOCKSIZE 200000

/* Functions */

fe_t *fe_init(param_t *P);
int32 fe_start_utt(fe_t *FE);
int32 fe_end_utt(fe_t *FE, float *cepvector);
int32 fe_close(fe_t *FE);
int32 fe_process_utt(fe_t *FE, int16 *spch, int32 nsamps, float ***cep_block);
int32 fe_process(fe_t *FE, int16 *spch, int32 nsamps, float ***cep_block);



#endif
