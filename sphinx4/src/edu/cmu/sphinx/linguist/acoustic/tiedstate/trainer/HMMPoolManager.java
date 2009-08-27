
/*
 * Copyright 1999-2002 Carnegie Mellon University.  
 * Portions Copyright 2002 Sun Microsystems, Inc.  
 * Portions Copyright 2002 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 * 
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL 
 * WARRANTIES.
 *
 */

package edu.cmu.sphinx.linguist.acoustic.tiedstate.trainer;

import java.util.HashMap;
import java.util.Iterator;
import java.util.logging.Logger;

import edu.cmu.sphinx.frontend.Data;
import edu.cmu.sphinx.frontend.FloatData;
import edu.cmu.sphinx.linguist.acoustic.HMMState;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.GaussianMixture;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.HMMManager;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.Loader;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.MixtureComponent;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.Pool;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.SenoneHMM;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.SenoneHMMState;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.TiedStateAcousticModel;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.util.SphinxProperties;


/**
 * Manages the HMM pools.
 */
class HMMPoolManager {

    private HMMManager hmmManager;
    private HashMap indexMap;
    private Pool meansPool;
    private Pool variancePool;
    private Pool matrixPool;
    private Pool mixtureWeightsPool;
    private Pool meanTransformationMatrixPool;
    private Pool meanTransformationVectorPool;
    private Pool varianceTransformationMatrixPool;
    private Pool varianceTransformationVectorPool;

    private Pool meansBufferPool;
    private Pool varianceBufferPool;
    private Pool matrixBufferPool;
    private Pool mixtureWeightsBufferPool;

    private Pool senonePool;
    private int vectorLength;
    private LogMath logMath;

    private float logMixtureWeightFloor;
    private float logTransitionProbabilityFloor;
    private float varianceFloor;
    private float logLikelihood;
    private float currentLogLikelihood;

    /*
     * The logger for this class
     */
    private static Logger logger =
        Logger.getLogger("edu.cmu.sphinx.linguist.acoustic.HMMPoolManager");

    /**
     * Constructor for this pool manager.
     * It gets the pointers to the pools from a loader.
     *
     * @param loader the loader
     */
    protected HMMPoolManager(Loader loader, SphinxProperties props) {
        hmmManager = loader.getHMMManager();
	indexMap = new HashMap();
	meansPool = loader.getMeansPool();
	variancePool = loader.getVariancePool();
	mixtureWeightsPool = loader.getMixtureWeightPool();
	matrixPool = loader.getTransitionMatrixPool();
	senonePool = loader.getSenonePool();
	logMath = LogMath.getLogMath(props.getContext());
        float mixtureWeightFloor = 
	    props.getFloat(TiedStateAcousticModel.PROP_MW_FLOOR, 
			   TiedStateAcousticModel.PROP_MW_FLOOR_DEFAULT);
	logMixtureWeightFloor = logMath.linearToLog(mixtureWeightFloor);
        float transitionProbabilityFloor = 
	    props.getFloat(TiedStateAcousticModel.PROP_TP_FLOOR, 
			   TiedStateAcousticModel.PROP_TP_FLOOR_DEFAULT);
	logTransitionProbabilityFloor = 
	    logMath.linearToLog(transitionProbabilityFloor);
        varianceFloor = 
	    props.getFloat(TiedStateAcousticModel.PROP_VARIANCE_FLOOR, 
			   TiedStateAcousticModel.PROP_VARIANCE_FLOOR_DEFAULT);

	createBuffers();
	logLikelihood = 0.0f;
    }

    /**
     * Recreates the buffers.
     */
    protected void resetBuffers() {
	createBuffers();
	logLikelihood = 0.0f;
    }

    /**
     * Create buffers for all pools used by the trainer in this pool manager.
     */
    protected void createBuffers() {
	// the option false or true refers to whether the buffer is in
	// log scale or not, true if it is.
	meansBufferPool = create1DPoolBuffer(meansPool, false);
	varianceBufferPool = create1DPoolBuffer(variancePool, false);
	matrixBufferPool = create2DPoolBuffer(matrixPool, true);
	mixtureWeightsBufferPool = create1DPoolBuffer(mixtureWeightsPool, 
						      true);
    }

    /**
     * Create buffers for a given pool.
     */
    private Pool create1DPoolBuffer(Pool pool, boolean isLog) {
	Pool bufferPool = new Pool(pool.getName());

	for (int i = 0; i < pool.size(); i++) {
	    float[] element = (float [])pool.get(i);
	    indexMap.put(element, new Integer(i));
	    Buffer buffer = new Buffer(element.length, isLog, i);
	    bufferPool.put(i, buffer);
	}
	return bufferPool;
    }

    /**
     * Create buffers for a given pool.
     */
    private Pool create2DPoolBuffer(Pool pool, boolean isLog) {
	Pool bufferPool = new Pool(pool.getName());

	for (int i = 0; i < pool.size(); i++) {
	    float[][] element = (float [][])pool.get(i);
	    indexMap.put(element, new Integer(i));
	    int poolSize = element.length;
	    Buffer[] bufferArray = new Buffer[poolSize];
	    for (int j = 0; j < poolSize; j++) {
		bufferArray[j] = new Buffer(element[j].length, isLog, j);
	    }
	    bufferPool.put(i, bufferArray);
	}
	return bufferPool;
    }

    /**
     * Accumulate the TrainerScore into the buffers.
     *
     * @param index the current index into the TrainerScore vector
     * @param score the TrainerScore
     */
    protected void accumulate(int index, TrainerScore[] score) {
	accumulate(index, score, null);
    }

    /**
     * Accumulate the TrainerScore into the buffers.
     *
     * @param index the current index into the TrainerScore vector
     * @param score the TrainerScore for the current frame
     * @param nextScore the TrainerScore for the next time frame
     */
    protected void accumulate(int index, 
			      TrainerScore[] score, 
			      TrainerScore[] nextScore) {
	int senoneID;
	TrainerScore thisScore = score[index];

	Data feature = thisScore.getData();
	// We should be doing this just once per utterance...
	// currentLogLikelihood = thisScore.getLogLikelihood();

	// Since we're scaling, the loglikelihood disappears...
	currentLogLikelihood = 0;
	// And the total becomes the sum of (-) scaling factors
	logLikelihood -= score[0].getScalingFactor();

	SenoneHMMState state = (SenoneHMMState) thisScore.getState();
	if (state == null) {
	    // We only care about the case "all models"
	    senoneID = thisScore.getSenoneID();
	    if (senoneID == TrainerAcousticModel.ALL_MODELS) {
		accumulateMean(senoneID, score[index]);
		accumulateVariance(senoneID, score[index]);
		accumulateMixture(senoneID, score[index]);
		accumulateTransition(senoneID, index, score, nextScore);
	    }
	} else {
	    // If state is non-emitting, we presume there's only one
	    // transition out of it. Therefore, we only accumulate
	    // data for emitting states.
	    if (state.isEmitting()) {
		senoneID = senonePool.indexOf(state.getSenone());
		// accumulateMean(senoneID, score[index]);
		// accumulateVariance(senoneID, score[index]);
		accumulateMixture(senoneID, score[index]);
		accumulateTransition(senoneID, index, score, nextScore);
	    }
	}
    }

    /**
     * Accumulate the means.
     */
    private void accumulateMean(int senone, TrainerScore score) {
	if (senone == TrainerAcousticModel.ALL_MODELS) {
	    for (int i = 0; i < senonePool.size(); i++) {
		accumulateMean(i, score);
	    }
	} else {
	    GaussianMixture gaussian = 
		(GaussianMixture) senonePool.get(senone);
	    MixtureComponent[] mix = gaussian.getMixtureComponents();
	    for (int i = 0; i < mix.length; i++) {
		float[] mean = mix[i].getMean();
		// int indexMean = meansPool.indexOf(mean);
		Integer indexInMap = (Integer) indexMap.get(mean);
		int indexMean = indexInMap.intValue();
		assert indexMean >= 0;
		assert indexMean == senone;
		Buffer buffer = (Buffer) meansBufferPool.get(indexMean);
		float[] feature = ((FloatData) score.getData()).getValues();
		double[] data = new double[feature.length];
		float prob = score.getComponentGamma()[i];
		prob -= currentLogLikelihood;
		double dprob = logMath.logToLinear(prob);
		// prob = (float) logMath.logToLinear(prob);
		for (int j = 0; j < data.length; j++) {
		    data[j] = feature[j] * dprob;
		}
		buffer.accumulate(data, dprob);
	    }
	}
    }

    /**
     * Accumulate the variance.
     */
    private void accumulateVariance(int senone, TrainerScore score) {
	if (senone == TrainerAcousticModel.ALL_MODELS) {
	    for (int i = 0; i < senonePool.size(); i++) {
		accumulateVariance(i, score);
	    }
	} else {
	    GaussianMixture gaussian = 
		(GaussianMixture) senonePool.get(senone);
	    MixtureComponent[] mix = gaussian.getMixtureComponents();
	    for (int i = 0; i < mix.length; i++) {
		float[] mean = mix[i].getMean();
		float[] variance = mix[i].getVariance();
		// int indexVariance = variancePool.indexOf(variance);
		Integer indexInMap = (Integer) indexMap.get(variance);
		int indexVariance = indexInMap.intValue();
		Buffer buffer = 
		    (Buffer) varianceBufferPool.get(indexVariance);
		float[] feature = ((FloatData) score.getData()).getValues();
		double[] data = new double[feature.length];
		float prob = score.getComponentGamma()[i];
		prob -= currentLogLikelihood;
		double dprob = logMath.logToLinear(prob);
		for (int j = 0; j < data.length; j++) {
		    data[j] = (feature[j] - mean[j]);
		    data[j] *= data[j] * dprob;
		}
		buffer.accumulate(data, dprob);
	    }
	}
    }


    /**
     * Accumulate the mixture weights.
     */
    private void accumulateMixture(int senone, TrainerScore score) {
	// The index into the senone pool and the mixture weight pool
	// is the same
	if (senone == TrainerAcousticModel.ALL_MODELS) {
	    for (int i = 0; i < senonePool.size(); i++) {
		accumulateMixture(i, score);
	    }
	} else {
	    Buffer buffer = (Buffer) mixtureWeightsBufferPool.get(senone);
	    float[] mixw = (float [])mixtureWeightsPool.get(senone);
	    for (int i = 0; i < mixw.length; i++) {
		float prob = score.getComponentGamma()[i];
		prob -= currentLogLikelihood;
		buffer.logAccumulate(prob, i, logMath);
	    }
	}
    }

    /**
     * Accumulate transitions from a given state.
     *
     * @param indexScore the current index into the TrainerScore
     * @param score the score information
     * @param nextScore the score information for the next frame
     */
    private void accumulateStateTransition(int indexScore,
					   TrainerScore[] score, 
					   TrainerScore[] nextScore) {
	HMMState state = score[indexScore].getState();
	if (state == null) {
	    // Non-emitting state
	    return;
	}
	int indexState = state.getState();
	SenoneHMM hmm = (SenoneHMM) state.getHMM();
	float[][] matrix = hmm.getTransitionMatrix();

	// Find the index for current matrix in the transition matrix pool
	// int indexMatrix = matrixPool.indexOf(matrix);
	Integer indexInMap = (Integer) indexMap.get(matrix);
	int indexMatrix = indexInMap.intValue();

	// Find the corresponding buffer
	Buffer[] bufferArray = 
	    (Buffer []) matrixBufferPool.get(indexMatrix);

	// Let's concentrate on the transitions *from* the current state
	float[] vector = matrix[indexState];

	for (int i = 0; i < vector.length; i++) {
	    // Make sure this is a valid transition
	    if (vector[i] != LogMath.getLogZero()) {

		// We're assuming that if the states have position "a"
		// and "b" in the HMM, they'll have positions "k+a"
		// and "k+b" in the graph, that is, their relative
		// position is the same.

		// Distance between current state and "to" state in
		// the HMM
		int dist = i - indexState;

		// "to" state in the graph
		int indexNextScore = indexScore + dist;

		// Make sure the next state is non-emitting (the last
		// in the HMM), or in the same HMM.
		assert ((nextScore[indexNextScore].getState() == null) || 
		     (nextScore[indexNextScore].getState().getHMM() == hmm));
		float alpha = score[indexScore].getAlpha();
		float beta = nextScore[indexNextScore].getBeta();
		float transitionProb = vector[i];
		float outputProb = nextScore[indexNextScore].getScore();
		float prob = alpha + beta + transitionProb + outputProb;
		prob -= currentLogLikelihood;
		// i is the index into the next state.
		bufferArray[indexState].logAccumulate(prob, i, logMath);
		/*
	if ((indexMatrix == 0) && (i == 2)) {
	    //    	    System.out.println("Out: " + outputProb);
		    //	    	    bufferArray[indexState].dump();
	}
		*/
	    }
	}
    }

    /**
     * Accumulate transitions from a given state.
     *
     * @param indexState the state index
     * @param hmm the HMM
     * @param value the value to accumulate
     */
    private void accumulateStateTransition(int indexState, SenoneHMM hmm, 
					   float value) {
	// Find the transition matrix in this hmm
	float[][] matrix = hmm.getTransitionMatrix();

	// Find the vector with transitions from the current state to
	// other states.
	float[] stateVector = matrix[indexState];

	// Find the index of the current transition matrix in the
	// transition matrix pool.
	// int indexMatrix = matrixPool.indexOf(matrix);
	Integer indexInMap = (Integer) indexMap.get(matrix);
	int indexMatrix = indexInMap.intValue();

	// Find the buffer for the transition matrix.
	Buffer[] bufferArray = 
	    (Buffer []) matrixBufferPool.get(indexMatrix);

	// Accumulate for the transitions from current state
	for (int i = 0; i < stateVector.length; i++) {
	    // Make sure we're not trying to accumulate in an invalid
	    // transition.
	    if (stateVector[i] != LogMath.getLogZero()) {
		bufferArray[indexState].logAccumulate(value, i, logMath);
	    }
	}
    }

    /**
     * Accumulate the transition probabilities.
     */
    private void accumulateTransition(int indexHmm, int indexScore,
				      TrainerScore[] score,
				      TrainerScore[] nextScore) {
	if (indexHmm == TrainerAcousticModel.ALL_MODELS) {
	    // Well, special case... we want to add an amount to all
	    // the states in all models
	    for (Iterator i = hmmManager.getIterator();
		 i.hasNext(); ) {
		SenoneHMM hmm = (SenoneHMM) i.next();
		for (int j = 0; j < hmm.getOrder(); j++) {
		    accumulateStateTransition(j, hmm, 
				      score[indexScore].getScore());
		}
	    }
	} else {
	    // For transition accumulation, we don't consider the last
	    // time frame, since there's no transition from there to
	    // anywhere...
	    if (nextScore != null) {
		accumulateStateTransition(indexScore, score, nextScore);
	    }
	}
    }

    /**
     * Update the log likelihood. This method should be called for
     * every utterance.
     */
    protected void updateLogLikelihood() {
	// logLikelihood += currentLogLikelihood;
    }

    /** 
     * Normalize the buffers.
     *
     * @return the log likelihood associated with the current training set
     */
    protected float normalize() {
	normalizePool(meansBufferPool);
	normalizePool(varianceBufferPool);
	logNormalizePool(mixtureWeightsBufferPool);
	logNormalize2DPool(matrixBufferPool, matrixPool);
	return logLikelihood;
   }

    /**
     * Normalize a single buffer pool.
     *
     * @param pool the buffer pool to normalize
     */
    private void normalizePool(Pool pool) {
	assert pool != null;
	for (int i = 0; i < pool.size(); i++) {
	    Buffer buffer = (Buffer)pool.get(i);
	    if (buffer.wasUsed()) {
		buffer.normalize();
	    }
	}
    }

    /**
     * Normalize a single buffer pool in log scale.
     *
     * @param pool the buffer pool to normalize
     */
    private void logNormalizePool(Pool pool) {
	assert pool != null;
	for (int i = 0; i < pool.size(); i++) {
	    Buffer buffer = (Buffer)pool.get(i);
	    if (buffer.wasUsed()) {
		buffer.logNormalize();
	    }
	}
    }

    /**
     * Normalize a 2D buffer pool in log scale. Typically, this is the
     * case with the transition matrix, which also needs a mask for
     * values that are allowed, and therefor have to be updated, or
     * not allowed, and should be ignored.
     *
     * @param pool the buffer pool to normalize
     * @param maskPool pool containing a mask with zero/non-zero values.
     */
    private void logNormalize2DPool(Pool pool, Pool maskPool) {
	assert pool != null;
	for (int i = 0; i < pool.size(); i++) {
	    Buffer[] bufferArray = (Buffer []) pool.get(i);
	    float[][] mask = (float[][]) maskPool.get(i);
	    for (int j = 0; j < bufferArray.length; j++) {
		if (bufferArray[j].wasUsed()) {
		    bufferArray[j].logNormalizeNonZero(mask[j]);
		}
	    }
	}
    }

    /** 
     * Update the models.
     */
    protected void update() {
	updateMeans();
	updateVariances();
	recomputeMixtureComponents();
	updateMixtureWeights();
	updateTransitionMatrices();
    }

    /**
     * Copy one vector onto another.
     *
     * @param in the source vector
     * @param out the destination vector
     */
    private void copyVector(float[] in, float[] out) {
	assert in.length == out.length;
	for (int i = 0; i < in.length; i++) {
	    out[i] = in[i];
	}
    }

    /**
     * Update the means.
     */
    private void updateMeans() {
	assert meansPool.size() == meansBufferPool.size();
	for (int i = 0; i < meansPool.size(); i++) {
	    float[] means = (float [])meansPool.get(i);
	    Buffer buffer = (Buffer) meansBufferPool.get(i);
	    if (buffer.wasUsed()) {
		float[] meansBuffer = buffer.getValues();
		copyVector(meansBuffer, means);
	    } else {
		logger.info("Senone " + i + " not used.");
	    }
	}
    }

    /**
     * Update the variances.
     */
    private void updateVariances() {
	assert variancePool.size() == varianceBufferPool.size();
	for (int i = 0; i < variancePool.size(); i++) {
	    float[] means = (float [])meansPool.get(i);
	    float[] variance = (float [])variancePool.get(i);
	    Buffer buffer = (Buffer) varianceBufferPool.get(i);
	    if (buffer.wasUsed()) {
		float[] varianceBuffer = (float [])buffer.getValues();
		assert means.length == varianceBuffer.length;
		for (int j = 0; j < means.length; j++) {
		    varianceBuffer[j] -= means[j] * means[j];
		    if (varianceBuffer[j] < varianceFloor) {
			varianceBuffer[j] = varianceFloor;
		    }
		}
		copyVector(varianceBuffer, variance);
	    }
	}
    }

    /**
     * Recompute the precomputed values in all mixture components.
     */
    private void recomputeMixtureComponents() {
	for (int i = 0; i < senonePool.size(); i++) {
	    GaussianMixture gMix = (GaussianMixture) senonePool.get(i);
	    MixtureComponent[] mixComponent = gMix.getMixtureComponents();
	    for (int j = 0; j < mixComponent.length; j++) {
		mixComponent[j].precomputeDistance();
	    }
	}
    }

    /**
     * Update the mixture weights.
     */
    private void updateMixtureWeights() {
	assert mixtureWeightsPool.size() == mixtureWeightsBufferPool.size();
	for (int i = 0; i < mixtureWeightsPool.size(); i++) {
	    float[] mixtureWeights = (float [])mixtureWeightsPool.get(i);
	    Buffer buffer = (Buffer) mixtureWeightsBufferPool.get(i);
	    if (buffer.wasUsed()) {
		if (buffer.logFloor(logMixtureWeightFloor)) {
		    buffer.logNormalizeToSum(logMath);
		}
		float[] mixtureWeightsBuffer = (float [])buffer.getValues();
		copyVector(mixtureWeightsBuffer, mixtureWeights);
	    }
	}
    }

    /**
     * Update the transition matrices.
     */
    private void updateTransitionMatrices() {
	assert matrixPool.size() == matrixBufferPool.size();
	for (int i = 0; i < matrixPool.size(); i++) {
	    float[][] matrix = (float [][])matrixPool.get(i);
	    Buffer[] bufferArray = (Buffer []) matrixBufferPool.get(i);
	    for (int j = 0; j < matrix.length; j++) {
		Buffer buffer = bufferArray[j];
		if (buffer.wasUsed()) {
		    for (int k = 0; k < matrix[j].length; k++) {
			float bufferValue = buffer.getValue(k);
			if (bufferValue != LogMath.getLogZero()) {
			    assert matrix[j][k] != LogMath.getLogZero();
			    if (bufferValue < logTransitionProbabilityFloor) {
				buffer.setValue(k, 
					logTransitionProbabilityFloor);
			    }
			}
		    }
		    buffer.logNormalizeToSum(logMath);
		    copyVector(buffer.getValues(), matrix[j]);
		}
	    }
	}
    }
}
