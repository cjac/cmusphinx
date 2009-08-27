


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

package edu.paul.bw;

import edu.cmu.sphinx.linguist.acoustic.HMMState;
import edu.cmu.sphinx.frontend.Data;
import java.io.Serializable;
import java.io.FileWriter;
import java.io.IOException;

/**
 * Represents a hmmState in an SentenceHMMS
 * 
 */
public class HMMStateState extends SentenceHMMState 
    implements Serializable,SearchState {
    private HMMState hmmState;
    private boolean isEmitting;

    /**
     * Creates a HMMStateState
     *
     * @param parent the parent of this state
     * @param hmmState the hmmState associated with this state
     */
    public HMMStateState( HMMState hmmState,int nodeId) {
	super( "S",nodeId, hmmState.getState()); 
	this.hmmState = hmmState;
        this.isEmitting = hmmState.isEmitting();
    }
    public HMMStateState( HMMState hmmState,int nodeId,int startFrame,int endFrame) {
	super( "S",nodeId, hmmState.getState(),startFrame,endFrame); 
	this.hmmState = hmmState;
        this.isEmitting = hmmState.isEmitting();
    }

    /**
     * Gets the hmmState associated with this state
     *
     * @return the hmmState
     */
    public HMMState getHMMState() {
	return hmmState;
    }
    public boolean isStateZero (){
	return false;
    }

    /**
     * Determines if this state is an emitting state
     *
     * @return true if the state is an emitting state
     */
    public boolean isEmitting() {
	return isEmitting;
    }

    /**
     * Retrieves a short label describing the type of this state.
     * Typically, subclasses of SentenceHMMState will implement this
     * method and return a short (5 chars or less) label
     *
     * @return the short label.
     */
    public String getTypeLabel() {
	return "HMM";
    }


    /**
     * Calculate the acoustic score for this state
     *
     * @return the acoustic score for this state
     */
    public float getScore(Data feature) {
	return hmmState.getScore(feature);
    }

    public double getScoreDouble(Data feature) {
		return hmmState.getScoreDouble(feature);
    }
    /**
     * empty contructor
     */
    private HMMStateState() {
    }

    /**
     * Returns the state order for this state type
     *
     * @return the state order
     */
    public int getOrder() {
        return isEmitting ? 2 : 0;
    }
    public void dumpAISee(FileWriter f) throws IOException {
       f.write("node: { title: \"" + getName() + "\" label: \""
	       + getHMMState().getHMM().getBaseUnit() + "[" + getStartFrame() + "," + getEndFrame() +  "]\" }\n");
   }
}


