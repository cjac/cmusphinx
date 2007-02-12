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
 * Time: 8:49:25 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

import java.util.Arrays;
import java.util.List;

/**
 * A text record's unique identifier. Text records belong to an Item and consist of one or more
 * RegionOfText records.
 * @see ItemID
 * @see RegionOfTextID
 */
public class TextID {
    long id;
    List<RegionOfTextID> textRegionIDs;

    public TextID(long id, List<RegionOfTextID> textRegionIDs) {
        this.id = id;
        this.textRegionIDs = textRegionIDs;
    }

    public TextID() {
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public List<RegionOfTextID> getTextRegionIDs() {
        return textRegionIDs;
    }

    public void setTextRegionIDs(List<RegionOfTextID> textRegionIDs) {
        this.textRegionIDs = textRegionIDs;
    }
}
