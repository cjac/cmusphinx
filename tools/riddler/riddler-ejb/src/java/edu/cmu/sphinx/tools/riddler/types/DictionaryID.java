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
 * Time: 12:23:53 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

import java.util.Arrays;
import java.util.List;

/**
 * A Dictionary's unique identifier.  A Dictionary consists of Pronunciation records.
 * @see PronunciationID
 */
public class DictionaryID {
    long id;
    List<PronunciationID> pronIDs;

    public DictionaryID(long id, List<PronunciationID> pronIDs) {
        this.id = id;
        this.pronIDs = pronIDs;
    }

    public DictionaryID() {
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public List<PronunciationID> getPronIDs() {
        return pronIDs;
    }

    public void setPronIDs(List<PronunciationID> pronIDs) {
        this.pronIDs = pronIDs;
    }
}
