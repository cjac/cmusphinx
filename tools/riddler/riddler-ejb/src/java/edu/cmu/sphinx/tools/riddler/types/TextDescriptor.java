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
 * Date: Feb 3, 2007
 * Time: 12:14:26 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

import java.util.List;

/**
 * The "meat" of a text entry. Contains the actual String contents in one long list (not assuming
 * spaces as delimiters for better non-Western language compatibility). All tokens in the words list
 * should be present in the dictionary(ies) assigned to this Corpus.
 * @see DictionaryID
 * @see TextID
 */
public class TextDescriptor {
    List<String> words;

    public TextDescriptor(List<String> words) {
        this.words = words;
    }

    public TextDescriptor() {
    }

    public List<String> getWords() {
        return words;
    }

    public void setWords(List<String> words) {
        this.words = words;
    }
}
