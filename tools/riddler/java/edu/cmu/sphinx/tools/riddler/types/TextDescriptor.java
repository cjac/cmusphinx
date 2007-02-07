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

/**
 * The "meat" of a text entry. Contains the actual String contents in one long array (not assuming
 * spaces as delimiters for better non-Western language compatibility). All tokens in the words array
 * should be present in the dictionary(ies) assigned to this Corpus.
 * @see DictionaryID
 * @see TextID
 */
public class TextDescriptor {
    String[] words;

    public TextDescriptor(String[] words) {
        this.words = words;
    }

    public String[] getWords() {
        return words;
    }

    public void setWords(String[] words) {
        this.words = words;
    }
}
