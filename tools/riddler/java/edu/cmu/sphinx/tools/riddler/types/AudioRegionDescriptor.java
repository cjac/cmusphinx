/**
 * Copyright 1999-2007 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * <p/>
 * User: Garrett Weinberg
 * Date: Jan 16, 2007
 * Time: 11:11:00 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

/**
 * represents one or more words with a known beginning and end time within a region of audio;
 * meaningless until associated with an utterance
 *
 * @see edu.cmu.sphinx.tools.riddler.Riddler#createAudioRegion(AudioRegionDescriptor)
 * @see edu.cmu.sphinx.tools.riddler.Riddler#addAudioRegionToUtterance(AudioRegionID, UtteranceID)
 */
public class AudioRegionDescriptor {
    int beginTime;
    int endTime;
    String words;

    public AudioRegionDescriptor(int beginTime, int endTime, String words) {
        this.beginTime = beginTime;
        this.endTime = endTime;
        this.words = words;
    }
}
