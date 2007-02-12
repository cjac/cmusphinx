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

package edu.cmu.sphinx.tools.riddler.types.audio;

/**
 * Describes an Audio record and holds its data.
 * @see edu.cmu.sphinx.tools.riddler.types.CorpusID
 */
public abstract class AudioDescriptor {
    int samplesPerSecond;
    int channelCount;

    protected AudioDescriptor() {
    }

    public AudioDescriptor(int samplesPerSecond, int channelCount) {
        this.samplesPerSecond = samplesPerSecond;
        this.channelCount = channelCount;
    }

    public int getSamplesPerSecond() {
        return samplesPerSecond;
    }

    public void setSamplesPerSecond(int samplesPerSecond) {
        this.samplesPerSecond = samplesPerSecond;
    }

    public int getChannelCount() {
        return channelCount;
    }

    public void setChannelCount(int channelCount) {
        this.channelCount = channelCount;
    }
}
