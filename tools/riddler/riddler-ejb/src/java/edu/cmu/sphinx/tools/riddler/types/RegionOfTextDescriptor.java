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
 * Date: Jan 28, 2007
 * Time: 10:31:40 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

/**
 * Describes a single or multiple-word region within a Text record. A RegionOfAudio may point to it.
 * @see TextID
 * @see RegionOfAudioID
 */
public class RegionOfTextDescriptor {
    int startIndex;
    int endIndex;

    public RegionOfTextDescriptor(int startIndex, int endIndex) {
        this.startIndex = startIndex;
        this.endIndex = endIndex;
    }

    public RegionOfTextDescriptor() {
    }

    public int getStartIndex() {
        return startIndex;
    }

    public void setStartIndex(int startIndex) {
        this.startIndex = startIndex;
    }
}
