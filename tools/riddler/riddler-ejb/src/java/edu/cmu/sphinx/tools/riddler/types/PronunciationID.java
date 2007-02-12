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
 * Date: Jan 20, 2007
 * Time: 10:42:32 AM
 */

package edu.cmu.sphinx.tools.riddler.types;

/**
 * A Pronunciation record's unique identifier
 */
public class PronunciationID {
    long id;

    public PronunciationID(long id) {
        this.id = id;
    }

    public PronunciationID() {
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }
}
