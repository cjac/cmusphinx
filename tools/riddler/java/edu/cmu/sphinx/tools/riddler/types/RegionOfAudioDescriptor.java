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
 * Descriptor for a RegionOfAudio record.  Has start and end time (millisecond) markers.
 */
public class RegionOfAudioDescriptor {
    int beginTime;
    int endTime;

    /**
     *
     * @param beginTime millisecond timestamp at which this region of audio begins
     * @param endTime millisecond timestamp at which this region of audio begins
     */
    public RegionOfAudioDescriptor(int beginTime, int endTime) {
        this.beginTime = beginTime;
        this.endTime = endTime;
    }

    public int getBeginTime() {
        return beginTime;
    }

    public void setBeginTime(int beginTime) {
        this.beginTime = beginTime;
    }

    public int getEndTime() {
        return endTime;
    }

    public void setEndTime(int endTime) {
        this.endTime = endTime;
    }
}
