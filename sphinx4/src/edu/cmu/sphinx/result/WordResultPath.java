/*
 * Copyright 1999-2004 Carnegie Mellon University.
 * Portions Copyright 2004 Sun Microsystems, Inc.
 * Portions Copyright 2004 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 *
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 *
 * Created on Aug 31, 2004
 */
 
package edu.cmu.sphinx.result;

import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import edu.cmu.sphinx.util.LogMath;

/**
 * An implementation of a result Path that computes scores and confidences on the fly.
 * 
 * @author pgorniak
 */
public class WordResultPath extends Vector implements Path {

    /**
     * Constructs a WordResultPath with the given list of WordResults
     * and LogMath.
     *
     * @param wordResults the list of WordResults
     */
    WordResultPath(List wordResults) {
        this.addAll(wordResults);
    }

    /**
     * Constructs an empty WordResultPath.
     */
    WordResultPath() {}
    
    /**
     * @see edu.cmu.sphinx.result.Path#getScore()
     */
    public double getScore() {
        double score = LogMath.getLogOne();
        Iterator i = iterator();
        while (i.hasNext()) {
            WordResult wr = (WordResult)i.next();
            score += wr.getScore();
        }
        return score;
    }

    /**
     * @see edu.cmu.sphinx.result.Path#getConfidence()
     */
    public double getConfidence() {
        double confidence = LogMath.getLogOne();
        Iterator i = iterator();
        while (i.hasNext()) {
            WordResult wr = (WordResult)i.next();
            confidence += wr.getConfidence();
        }
        return confidence;
    }

    /**
     * Returns the LogMath of the scores.
     *
     * @return the LogMath of the scores
     */
    public LogMath getLogMath() {
        Iterator i = iterator();
        if (i.hasNext()) {
            WordResult wr = (WordResult) i.next();
            return wr.getLogMath();
        } else {
            return null;
        }
    }

    /**
     * @see edu.cmu.sphinx.result.Path#getWords()
     */
    public WordResult[] getWords() {
        return (WordResult[])this.toArray(new WordResult[0]);
    }

    /**
     * @see edu.cmu.sphinx.result.Path#getTranscription()
     */
    public String getTranscription() {
        StringBuffer sb = new StringBuffer();
        Iterator i = iterator();
        while (i.hasNext()) {
            WordResult wr = (WordResult)i.next();
            sb.append(wr.toString());
            if (i.hasNext()) {
                sb.append(" ");
            }
        }
        return sb.toString();
    }

}
