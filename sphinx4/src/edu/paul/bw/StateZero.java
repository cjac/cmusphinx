package edu.paul.bw;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.linguist.acoustic.HMMState;
public class StateZero extends  HMMStateState {
    public StateZero( HMMState hmmState,int nodeId,int startFrame,int endFrame) {
	super( hmmState, nodeId, startFrame, endFrame);
    }

    private float alpha=LogMath.getLogZero();
    private float beta=LogMath.getLogZero();
    private float probability=LogMath.getLogZero();
    public void  setAlpha(float alpha) {
	this.alpha=alpha;
    }
    public float getAlpha() {
	return alpha;
    }
    public void  setBeta(float beta) {
	this.beta=beta;
    }
    public float getBeta() {
	return beta;
    }
    public void  setProbability(float probability) {
	this.probability=probability;
    }
    public float getProbability() {
	return probability;
    }

    public boolean isStateZero () {
	return true;
    }
}