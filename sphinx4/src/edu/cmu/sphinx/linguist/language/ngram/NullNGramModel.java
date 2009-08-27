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
package edu.cmu.sphinx.linguist.language.ngram;


import java.io.IOException;

import java.util.Set;

import edu.cmu.sphinx.linguist.WordSequence;
import edu.cmu.sphinx.linguist.dictionary.Dictionary;
import edu.cmu.sphinx.linguist.dictionary.Word;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;

/**
 * An ascii ARPA language model loader. This loader makes no attempt to
 * optimize storage, so it can only load very small language models
 * <p>
 * Note that all probabilites in the grammar are stored in LogMath log base
 * format. Language Probabilties in the language model file are stored in log
 * 10 base.
 */
public class NullNGramModel implements LanguageModel {
    /**
     * Sphinx property that defines the logMath component.
     */
    public final static String PROP_LOG_MATH = "logMath";
    // ----------------------------
    // Configuration data
    // ----------------------------
    private String name;
    private int desiredMaxDepth;
    private boolean allocated = false;
    private int maxNGram=1;
    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
            throws PropertyException {
        this.name = name;
        registry.register(PROP_MAX_DEPTH, PropertyType.INT);

    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {


        if (allocated) {
            throw new PropertyException(this, null, 
                    "Can't change properties after allocation");
        }
        desiredMaxDepth = ps.getInt(PROP_MAX_DEPTH, PROP_MAX_DEPTH_DEFAULT);
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.linguist.language.ngram.LanguageModel#allocate()
     */
    public void allocate() throws IOException {
        allocated = true;
        if (desiredMaxDepth > 0) {
            if (desiredMaxDepth < maxNGram) {
                maxNGram = desiredMaxDepth;
            }
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.linguist.language.ngram.LanguageModel#deallocate()
     */
    public void deallocate() {
        allocated = false;
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#getName()
     */
    public String getName() {
        return name;
    }

    /**
     * Called before a recognition
     */
    public void start() {
    }

    /**
     * Called after a recognition
     */
    public void stop() {
    }

    /**
     * Gets the ngram probability of the word sequence represented by the word
     * list
     * 
     * @param wordSequence
     *                the word sequence
     * 
     * @return the probability of the word sequence. Probability is in logMath
     *         log base
     *  
     */
    public float getProbability(WordSequence wordSequence) {
        return  0.0f;
    }

    /**
     * Gets the smear term for the given wordSequence
     * 
     * @param wordSequence
     *                the word sequence
     * @return the smear term associated with this word sequence
     */
    public float getSmear(WordSequence wordSequence) {
        return 0.0f; // TODO not implememted
    }

    /**
     * Returns the maximum depth of the language model
     * 
     * @return the maximum depth of the language mdoel
     */
    public int getMaxDepth() {
        return maxNGram;
    }

    /**
     * Returns the set of words in the lanaguage model. The set is
     * unmodifiable.
     * 
     * @return the unmodifiable set of words
     */
    public Set getVocabulary() {
        return null;
    }
}
