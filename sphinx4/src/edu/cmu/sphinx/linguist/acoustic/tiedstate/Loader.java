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

import java.io.IOException;
import java.util.Map;

import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.SphinxProperties;

/**
 * Generic interface for a loader of acoustic models
 */
public interface Loader extends Configurable {
    
    /**
     * Loads the acoustic model
     * @throws IOException if an error occurs while loading the model
     */
    public void load() throws IOException;
    
    /**
     * Gets the pool of means for this loader
     *
     * @return the pool
     */
    public Pool getMeansPool();


    /**
     * Gets the pool of means transformation matrices for this loader
     *
     * @return the pool
     */
    public Pool getMeansTransformationMatrixPool();


    /**
     * Gets the pool of means transformation vectors for this loader
     *
     * @return the pool
     */
    public Pool getMeansTransformationVectorPool();


    /**
     * Gets the variance pool
     *
     * @return the pool
     */
    public Pool getVariancePool();

    /**
     * Gets the variance transformation matrix pool
     *
     * @return the pool
     */
    public Pool getVarianceTransformationMatrixPool();


    /**
     * Gets the mixture weight pool
     *
     * @return the pool
     */
    public Pool getMixtureWeightPool();

    /**
     * Gets the transition matrix pool
     *
     * @return the pool
     */
    public Pool getTransitionMatrixPool();


    /**
     * Gets the senone pool for this loader
     *
     * @return the pool
     */
    public Pool getSenonePool();


    /**
     * Returns the HMM Manager for this loader
     *
     * @return the HMM Manager
     */
    public HMMManager getHMMManager();

    /**
     * Returns the map of context indepent units. The map can be
     * accessed by unit name.
     *
     * @return the map of context independent units.
     */
    public Map getContextIndependentUnits();



    /**
     * logs information about this loader
     */
    public void logInfo(); 


   /**
    * Returns the size of the left context for context dependent
    * units
    *
    * @return the left context size
    */
    public int getLeftContextSize();
    
    /**
     * Returns the size of the right context for context dependent
     * units
     *
     * @return the left context size
     */
    public int getRightContextSize();

}


