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

package edu.cmu.sphinx.linguist.language.ngram.large4;


/**
 * Represents a word ID (third word of a trigram), and a trigram 
 * probability ID.
 */
class QuadrigramProbability {

    private int wordID;
    private int probabilityID;


    /**
     * Constructs a TrigramProbability
     *
     * @param wordID the ID of the third word in a trigram
     * @param probabilityID the index into the probability array
     */
    public QuadrigramProbability(int wordID, int probabilityID) {
	this.wordID = wordID;
        this.probabilityID = probabilityID;
    }

    
    /**
     * Returns the 4 word ID of this Quadrigram
     *
     * @return the third word ID
     */ 
    public int getWordID() {
        return wordID;
    }


    /**
     * Returns the quadri probability ID.
     *
     * @return the quadrigram probability ID
     */
    public int getProbabilityID() {
        return probabilityID;
    }
}
