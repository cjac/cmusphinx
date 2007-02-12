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
 * Time: 8:49:37 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

/**
 * Unique identifier for an audio region.  Audio records consist of one or more RegionOfAudio records, and
 * may point to a corresponding RegionOfText.
 * @see AudioID
 * @see RegionOfTextID
 */
public class RegionOfAudioID {
    long id;
    RegionOfTextID textRegion;

    /**
     *
     * @param id
     * @param textRegion identifier of a text region corresponding to this audio region
     */
    public RegionOfAudioID(long id, RegionOfTextID textRegion) {
        this.id = id;
        this.textRegion = textRegion;
    }

    public RegionOfAudioID() {
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public RegionOfTextID getTextRegion() {
        return textRegion;
    }

    public void setTextRegion(RegionOfTextID textRegion) {
        this.textRegion = textRegion;
    }

}
