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
 * Date: Feb 8, 2007
 * Time: 11:20:19 PM
 */

package edu.cmu.sphinx.tools.riddler.persist.audio;

import javax.persistence.Entity;

/**
 * document me!
 * @author Garrett Weinberg
 */
@Entity
public class LongAudioDescriptor extends AudioDescriptor {

    long[] data;

    public LongAudioDescriptor(int samplesPerSecond, int channelCount, long[] data, String filename) {
        super(samplesPerSecond, channelCount, filename);
        this.data = data;
    }

    public LongAudioDescriptor() {
    }

    public long[] getData() {
        return data;
    }

    public void setData(long[] data) {
        this.data = data;
    }
}
