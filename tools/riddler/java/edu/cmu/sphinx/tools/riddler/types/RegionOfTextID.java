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
 * Time: 10:30:54 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

/**
 * Unique identifier for a text region.  Text records consist of one or more RegionOfText records.
 * @see TextID
 */
public class RegionOfTextID {
    int id;

    public RegionOfTextID(int id) {
        this.id = id;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }
}
