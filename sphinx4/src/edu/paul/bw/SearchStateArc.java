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



/**
 * Represents a single state in a language search space
 */
public interface  SearchStateArc {
    /**
     * Gets a successor to this search state
     *
     * @return the sucessor state
     */
    SearchState  getToState();
    SearchState getFromState();

     /**
      * Gets the composite probability of entering this state
      *
      * @return the log probability
      */
     float getProbability();

     /**
      * Gets the language probability of entering this state
      *
      * @return the log probability
      */
     float getLanguageProbability();

     /**
      * Gets the language probability of entering this state
      *
      * @return the log probability
      */
     float getAcousticProbability();


     /**
      * Gets the insertion probability of entering this state
      *
      * @return the log probability
      */
     float getInsertionProbability();
}
