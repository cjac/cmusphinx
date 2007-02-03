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
 * Time: 10:21:55 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

/**
 * Maps a word onto a set of legitimate pronunciations for a given Dictionary.
 * @see DictionaryID
 * @see PronunciationID
 */
public class PronunciationDescriptor {

    String word;
    String[] pronunciations;

    public PronunciationDescriptor(String word, String[] pronunciations) {
        this.word = word;
        this.pronunciations = pronunciations;
    }

    public String getWord() {
        return word;
    }

    public void setWord(String word) {
        this.word = word;
    }

    public String[] getPronunciations() {
        return pronunciations;
    }

    public void setPronunciations(String[] pronunciations) {
        this.pronunciations = pronunciations;
    }
}
