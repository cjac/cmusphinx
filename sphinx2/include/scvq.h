/*
 * scvq.h -- Interface to semi-continous quantization
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
 * 16-May-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from Fil Alleva's original.
 */


#ifndef __SCVQ_H__
#define __SCVQ_H__

#include <s2types.h>

#define CEP_VECLEN	13
#define POW_VECLEN	3
#define CEP_SIZE	CEP_VECLEN
#define POW_SIZE	POW_VECLEN

#define NUM_FEATURES	4

typedef enum {CEP_FEAT=0, DCEP_FEAT=1, POW_FEAT=2, DDCEP_FEAT=3} feat_t;

typedef enum {AGC_NONE=0, AGC_BETA=1, AGC_NOISE=2, AGC_EMAX=3, AGC_MAX=4} scvq_agc_t;
typedef enum {NORM_NONE=0, NORM_UTT=1, NORM_PRIOR=2} scvq_norm_t;
typedef enum {COMPRESS_NONE=0, COMPRESS_UTT=1, COMPRESS_PRIOR=2} scvq_compress_t;

#define MAX_TOPN	6	/* max number of TopN codewords */

#define NONE		-1

void SCVQInit(int32 top, int32 numModels, int32 numDist, double vFloor, int32 use20msdp);
void SCVQAgcSet(scvq_agc_t agc_type);
void SCVQAgcInit(int32 agcColdInit, int32 agcLookAhead);
int32 SCVQInitFeat(feat_t feat, char *meanPath, char *varPath, int32 *opdf);

/* used to fill input buffer */
int32 SCVQNewFrame(float *in);
int32 SCVQEndFrames();

/* used by search to compute scores */
int32  SCVQFramesAvail();
int32 SCVQGetNextScores(int32 *scores);
extern int32 SCVQGetNextScore();

int32  SCVQGetAndClearNetErr();

void setVarFloor(double aVal);
int32  readMeanCBFile(feat_t feat, float **CB, char *MeanCBFile);
int32  readVarCBFile(feat_t feat, register int32 *det, float **CB, char *VarCBFile);
int32  setPowVar(register int32 *det, float **CB, double pow_var);

#ifndef FALSE
#define FALSE		0
#endif
#ifndef TRUE
#define TRUE		1
#endif

#endif
