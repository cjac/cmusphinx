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
 * Time: 8:32:49 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

/**
 * wrapper around an utterance's unique identifier
 */
public class UtteranceID {
    int id;

    public UtteranceID(int id) {
        this.id = id;
    }
}
