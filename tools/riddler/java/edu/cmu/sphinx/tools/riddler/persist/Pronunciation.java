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

package edu.cmu.sphinx.tools.riddler.persist;

import javax.persistence.*;
import java.util.HashSet;

/**
 * A pronunciation record, containing a word and its one or more variant pronunciations.
 * @author Garrett Weinberg
 */
@Entity
@NamedQuery(name = "findPronunciationByWord",
        query = "SELECT DISTINCT p FROM Pronunciation p WHERE p.word = :word AND p.dictionary = :dictionary")
public class Pronunciation {

    @Id
    @GeneratedValue(strategy = GenerationType.TABLE)
    private String id;

    private String word;
    @Lob @Basic
    private HashSet<String> variants = new HashSet<String>();
    @ManyToOne
    private Dictionary dictionary;

    public Pronunciation(String word, HashSet<String> variants) {
        this.word = word;
        this.variants = variants;
    }

    public Pronunciation() {
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getWord() {
        return word;
    }

    public void setWord(String word) {
        this.word = word;
    }

    public HashSet<String> getVariants() {
        return variants;
    }

    public void setVariants(HashSet<String> variants) {
        this.variants = variants;
    }

    public Dictionary getDictionary() {
        return dictionary;
    }

    public void setDictionary(Dictionary dictionary) {
        this.dictionary = dictionary;
    }
}
