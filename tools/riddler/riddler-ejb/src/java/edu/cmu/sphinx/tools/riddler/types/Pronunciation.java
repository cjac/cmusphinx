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

import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import java.util.List;

/**
 * A pronunciation record, containing a word and its one or more variant pronunciations.
 * @author Garrett Weinberg
 */
@Entity
public class Pronunciation {
    private int id;
    private String word;
    private List<String> variants;

    public Pronunciation(int id, String word, List<String> variants) {
        this.id = id;
        this.word = word;
        this.variants = variants;
    }

    public Pronunciation() {
    }

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getWord() {
        return word;
    }

    public void setWord(String word) {
        this.word = word;
    }

    public List<String> getVariants() {
        return variants;
    }

    public void setVariants(List<String> variants) {
        this.variants = variants;
    }
}
