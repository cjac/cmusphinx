package edu.paul.bw;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.linguist.acoustic.HMMState;
public class StatePhoneDouble extends  HMMStateState {// cela peut se discuter
    // with extends StateZero but je prefere comme cela quitte à dupliquer le code de State zero
    public StatePhoneDouble( HMMState hmmState,int nodeId,int startFrame,int endFrame) {
	super( hmmState, nodeId, startFrame, endFrame);
    }
    // code stateZero modifie for linear double
    private double alpha=0.0;
    private  double beta=0.0;
    private double probability=0.0;
    public void  setAlpha(double alpha) {
	this.alpha=alpha;
    }
    public double getAlphaDouble() {
	return alpha;
    }
    public void  setBeta(double beta) {
	this.beta=beta;
    }
    public double getBetaDouble() {
	return beta;
    }
    public void  setProbability(double probability) {
	this.probability=probability;
    }
    public double getProbabilityDouble() {
	return probability;
    }

    public boolean isStateZero () {
	return true;
    }

    //code specifique
    //povey 2003
    private float phoneAcc;
    private double betaPrime=0.0;
    private double alphaPrime=0.0;
      public float getPhoneAcc() {
	return phoneAcc;
    }
    public void  setPhoneAcc(float phoneAcc) {
	this.phoneAcc=phoneAcc;
    }
    public double getAlphaPrime() {
	return alphaPrime;
    }
    public void  setAlphaPrime(double alphaPrime) {
	this.alphaPrime=alphaPrime;
    }
    public double getBetaPrime() {
	return betaPrime;
    }
    public void  setBetaPrime(double betaPrime) {
	this.betaPrime=betaPrime;
    } 
    
    private Double gammaMpe;

     public double getGammaMpe() {
	return gammaMpe;
    }
    public void  setGammaMpe(double gammaMpe) {
	this.gammaMpe=gammaMpe;
    }
   
}