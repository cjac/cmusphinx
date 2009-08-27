
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

package edu.cmu.sphinx.decoder.scorer;

import java.io.IOException;
import java.util.List;

import edu.cmu.sphinx.util.props.Configurable;

/**
 * Provides a mechanism for scoring a set of HMM states
 *
 */
public interface AcousticScorer  extends Configurable {
    
    /**
     * Allocates resources for this scorer
     *
     */
    public void allocate() throws IOException;
    
    
    /**
     * Deallocates resouces for this scorer
     *
     */
    public void deallocate();

    /**
     * starts the scorer
     */
    public void startRecognition();

    /**
     * Scores the given set of states
     *
     * @param scorableList a list containing Scorable objects to
     * be scored
     *
     * @return the best scoring scorable, or null if there are no
     * more frames to score
     */
    public Scoreable calculateScores(List scorableList);

    /**
     * stops the scorer
     */
    public void stopRecognition();
}


