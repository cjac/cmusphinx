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

package edu.cmu.sphinx.linguist.dictionary;
import java.io.Serializable;

/**
 *  Provides a classification of words
 */
public class WordClassification implements Serializable {

    private String classificationName;

    /**
     * Unconstructable...
     */
    private WordClassification(String classificationName) {
	this.classificationName = classificationName;
    }

}

