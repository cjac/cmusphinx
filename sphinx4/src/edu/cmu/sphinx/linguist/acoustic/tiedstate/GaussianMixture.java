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

package edu.cmu.sphinx.linguist.acoustic.tiedstate;

import edu.cmu.sphinx.frontend.Data;
import edu.cmu.sphinx.util.LogMath;

/**
 * 
 * Represents a concrete implementation of a simple {@link Senone senone}.
 * A simple senone is a set of probability density functions implemented
 * as a gaussian mixture.
 * <p>
 * All scores and weights are maintained in LogMath log base.
 */
public class GaussianMixture implements Senone {
    // these data element in a senone may be shared with other senones
    // and therefore should not be written to.
    private float[] logMixtureWeights;			
    private MixtureComponent[] mixtureComponents;	
    private long id;

    transient volatile private Data lastDataScored;
    transient volatile private float logLastScore;
    transient volatile private double lastScore;
    transient volatile private Data lastDataComponent;
    transient volatile private float [] lastLogComponentScore;
    transient volatile private double [] lastComponentScore;
    transient volatile private boolean transfoToCalculed=false;


    private LogMath logMath;

    /**
     * Creates a new senone from the given components.
     *
     * @param logMath the log math 
     * @param logMixtureWeights the mixture weights for this senone in
     * LogMath log base
     * @param mixtureComponents the mixture components for this
     * senone
     */
    public GaussianMixture(LogMath logMath, float[] logMixtureWeights, 
                           MixtureComponent[] mixtureComponents, long id) {

	assert mixtureComponents.length == logMixtureWeights.length;

	this.logMath = logMath;
	this.mixtureComponents = mixtureComponents;
	this.logMixtureWeights = logMixtureWeights;
        this.id = id;
    }

    public void resetTransfo() {
	transfoToCalculed=true;
	if (compteur!=0) {
	    System.err.format("%30s %d mixture transformed \n","",compteur);
	    compteur=0;
	}
    }
    /**
     * Dumps this senone.
     *
     * @param msg annotation message
     */
    public void dump(String msg) {
	System.out.println(msg + " GaussianMixture: " + logLastScore);
    }

    
    /**
     * Returns a score for the given feature based upon this senone,
     * and calculates it if not already calculated. Note that this
     * method is not thread safe and should be externally synchronized
     * if it could be potentially called from multiple threads.
     *
     * @param feature the feature to score
     *
     * @return the score, in logMath log base, for the feature
     */
    public float getScore(Data feature)  {
	float logScore;
	
	if (feature.equals(lastDataScored)) {
	    logScore = logLastScore;
	} else {
	    logScore = calculateScore(feature);
	    logLastScore = logScore;
	    lastDataScored = feature;
	    lastScore=-1.0;
	}
	return logScore;
    }

    public double getScoreDouble (Data feature) {
	if (feature.equals(lastDataScored) && lastScore>0)
	    return lastScore;
	 lastScore =calculateScoreDouble(feature);
	 if (false) {
	 double temp= logMath.logToLinear(getScore(feature));
	 if (Math.abs(temp-lastScore)/temp>0.00001)
	     System.err.format("vue %4d %7e %7e\n",id,temp,lastScore);
	 }
	 lastDataScored=feature;
	 return lastScore;

    }

    /**
     * Determines if two objects are equal
     *
     * @param o the object to compare to this.
     *
     * @return true if the objects are equal
     */
    public boolean equals(Object o) {
        if (!(o instanceof Senone)) {
            return false;
        }
        Senone other = (Senone) o;
        return this.getID() == other.getID();
    }


    /**
     * Returns the hashcode for this object
     *
     * @return the hashcode
     */
    public int hashCode() {
        long id = getID();
        int high = (int) ((id >> 32) & 0xffffffff);
        int low = (int) (id & 0xffffffff);
        return high + low;
    }

    /**
     * Gets the ID for this senone
     *
     * @return the senone id
     */
    public long getID() {
        return id;
    }

    /**
     * Retrieves a string form of this object
     *
     * @return the string representation of this object
     */
    public String toString() {
        return "senone id: " + getID();
    }

    /**
     * Calculates the score for the senone.
     *
     * @param feature the feature to score
     *
     * @return the score, in logMath log base, for the feature
     */
    public float calculateScore(Data feature) {
	if (transfoToCalculed) makeTransfo();
	float logTotal =  mixtureComponents[0].getScore(feature)+
	    logMixtureWeights[0];
	 float zero= LogMath.getLogZero();
	for (int i = 1; i < mixtureComponents.length; i++) {
	    // In linear form, this would be:
	    //
	    // Total += Mixture[i].score * MixtureWeight[i]
            if (zero  !=   logMixtureWeights[i])
	    logTotal = logMath.addAsLinear(logTotal,
		 mixtureComponents[i].getScore(feature)+
					   logMixtureWeights[i]);    //paul j'ai des doutes sur ce truc 
	}

	return logTotal;
    }

    private double calculateScoreDouble(Data feature) {
	if (transfoToCalculed) makeTransfo();
	double total= logMath.logToLinear(mixtureComponents[0].getScore(feature)+
					  logMixtureWeights[0]);

	float zero= LogMath.getLogZero();
	for (int i = 1; i < mixtureComponents.length; i++) {
	    // In linear form, this would be:
	    //
	    // Total += Mixture[i].score * MixtureWeight[i]
            if (zero  !=   logMixtureWeights[i])
		total += logMath.logToLinear(mixtureComponents[i].getScore(feature)+
					     logMixtureWeights[i]);
	}
	
	return total;
    }
    





    /**
     * Calculates the scores for each component in the senone.
     *
     * @param feature the feature to score
     *
     * @return the LogMath log scores for the feature, one for each component
     */
    public float[] calculateComponentScore(Data feature) {
	if (transfoToCalculed) makeTransfo();
	float[] logComponentScore = new float[mixtureComponents.length];
	for (int i = 0; i < mixtureComponents.length; i++) {
	    // In linear form, this would be:
	    //
	    // Total += Mixture[i].score * MixtureWeight[i]
	    logComponentScore[i] = mixtureComponents[i].getScore(feature)+
		 logMixtureWeights[i];
	}

	return logComponentScore;
    }
    /**
     * Calculates the scores for each component in the senone.
     *
     * @param feature the feature to score
     *
     * @return the LogMath log scores for the feature, one for each component
     */
    public float[] calculateComponentScoreWithMemory(Data feature) {
	if (transfoToCalculed) makeTransfo();
	if (lastDataComponent== feature)
	    return lastLogComponentScore;
	float zero=logMath.getLogZero();
	float[] logComponentScore = new float[mixtureComponents.length];
	for (int i = 0; i < mixtureComponents.length; i++) {
	    // In linear form, this would be:
	    //
	    // Total += Mixture[i].score * MixtureWeight[i]
	    if (logMixtureWeights[i]!=zero)
		logComponentScore[i] = mixtureComponents[i].getScore(feature)+
		    logMixtureWeights[i];
	    else
		logComponentScore[i]=zero;	
	}
	lastDataComponent= feature;
	lastLogComponentScore=logComponentScore;
	return logComponentScore;
    }
    
    
    
    public double[] calculateComponentScoreWithMemoryDouble(Data feature) {
	if (true && lastDataComponent== feature)
	    return lastComponentScore;
	if (transfoToCalculed) makeTransfo();
	float zero=logMath.getLogZero();
	double [] componentScore = new double[mixtureComponents.length];
	for (int i = 0; i < mixtureComponents.length; i++) {
	    // In linear form, this would be:
	    //
	    // Total += Mixture[i].score * MixtureWeight[i]
	    if (logMixtureWeights[i]!=zero)
		componentScore[i] = logMath.logToLinear(mixtureComponents[i].getScore(feature)+
						    logMixtureWeights[i]);
	    else
		componentScore[i]=0.0;	
	}
	lastDataComponent= feature;
	lastComponentScore=componentScore;
	return componentScore;
    }
    /**
     * free the component memory after backward pass
     *
     *
     */

    public void clearComponentData() {
	lastDataComponent=null;
	lastLogComponentScore=null;
	lastComponentScore=null;
    }
    /**
     * Returns the mixture components associated with this Gaussian
     *
     * @return the array of mixture components
     */
    public MixtureComponent[] getMixtureComponents() {
	return mixtureComponents;
    }
    public float [] getLogMixtureWeights() {
	return logMixtureWeights;
    }

    static int compteur=0;
    private void  makeTransfo() {
	compteur++;
	for (int i =0 ; i< mixtureComponents.length; i++)
	    mixtureComponents[i].precomputeDistance();
	transfoToCalculed=false;
	
    }

}
