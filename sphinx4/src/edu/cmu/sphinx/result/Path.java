

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

package edu.cmu.sphinx.result;

import edu.cmu.sphinx.result.WordResult;
import edu.cmu.sphinx.util.LogMath;


/**
 * Represents a path of words through the recognition result lattice.
 * 
 * All scores are maintained in the logMath log domain
 */
public interface Path {

    /**
     * Gets the total score for this path. Scores are in the LogMath
     * log domain
     *
     * @return the score for the path in the LogMath log domaain.
     */
    public double getScore();

    /**
     * Returns a log confidence score for this path.
     * Use the getLogMath().logToLinear()
     * method to convert the log confidence score to linear.
     * The linear value should be between 0.0 and 1.0 (inclusive).
     *
     * @return a log confidence score which linear value is
     * between 0.0 and 1.0 (inclusive)
     */
    public double getConfidence();

    /**
     * Returns the log math for the scores of this path.
     *
     * @return the log math
     */
    public LogMath getLogMath();

    /**
     * Gets the ordered set of words for this path
     *
     * @return an array containing zero or more words 
     */
    public WordResult[] getWords();

    /**
     * Gets the transcription of the path. 
     *
     * @return the transcription of the path.
     */
    public String getTranscription();

    /**
     * Returns a string representation of this object
     */
    public String toString();
}

