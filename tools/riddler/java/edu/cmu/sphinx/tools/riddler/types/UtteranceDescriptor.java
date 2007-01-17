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
 * Date: Jan 13, 2007
 * Time: 9:37:06 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

/**
 * information about a given utterance
 */
public class UtteranceDescriptor {
    int bitsPerSample;
    int samplesPerSecond;
    int channelCount;

    public UtteranceDescriptor(int bitsPerSample, int samplesPerSecond, int channelCount) {
        this.bitsPerSample = bitsPerSample;
        this.samplesPerSecond = samplesPerSecond;
        this.channelCount = channelCount;
    }
}
