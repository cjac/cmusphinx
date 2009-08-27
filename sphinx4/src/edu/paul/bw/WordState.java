
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

import edu.cmu.sphinx.linguist.dictionary.Word;
import java.io.FileWriter;
import java.io.IOException;
/**
 * Represents a word in an SentenceHMMS
 * 
 */
public class WordState extends SentenceHMMState {
    private Word word;
    /**
     * Creates a WordState
     *
     */
    public WordState( Word word,int nodeId) {
	super("W",nodeId,0);
	this.word=word;
    }
    public WordState( Word word,int nodeId,int startFrame, int endFrame) {
	super("W",nodeId,0,startFrame,endFrame);
	this.word=word;
    }
    
    /**
     * Gets the word associated with this state
     *
     * @return the word
     */
    public Word getWord() {
	return word;
    }


    /**
     * Returns a pretty name for this state
     *
     * @return a pretty name for this state
     */
    public String getPrettyName() {
	return getName() + "(" + getWord().getSpelling() + ")";
    }
    /**
     * Retrieves a short label describing the type of this state.
     * Typically, subclasses of SentenceHMMState will implement this
     * method and return a short (5 chars or less) label
     *
     * @return the short label.
     */
    public String getTypeLabel() {
	return "Word";
    }

    public boolean isStateZero() {
	return false;
    }
    /**
     * Returns the state order for this state type
     *
     * @return the state order
     */
    public int getOrder() {
        return 1;
    }
    public SentenceHMMState getDirect() {
	return this;
    }
    //alpha prime du word is this : getDirect=this
    private double betaPrime=0.0;
    private double alphaPrime=0.0;
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
    public void dumpAISee(FileWriter f) throws IOException {
	f.write("node: { title: \"" + this.getName() + "\" label: \""
                + getWord() + "[" + getStartFrame() + "," + getEndFrame() + 
		"]\" }\n");
    }


}


