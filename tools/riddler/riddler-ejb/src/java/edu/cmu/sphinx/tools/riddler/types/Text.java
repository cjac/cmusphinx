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

import javax.persistence.*;
import java.util.List;

/**
 * A text record's unique identifier. Text records belong to an Item and consist of one or more
 * RegionOfText records.
 * @see Item
 * @see RegionOfText
 * @author Garrett Weinberg
 */
@Entity
public class Text {
    private int id;
    private List<RegionOfText> textRegions;
    private List<String> words;
    /**
     * parent reference, for bi-directional fetching
     */
    private Item item;

    public Text(int id, List<RegionOfText> textRegions, List<String> words) {
        this.id = id;
        this.textRegions = textRegions;
        this.words = words;
    }

    public Text() {
    }

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    @OneToMany(cascade = CascadeType.ALL, fetch = FetchType.EAGER, mappedBy="text")
    public List<RegionOfText> getTextRegions() {
        return textRegions;
    }

    public void setTextRegions(List<RegionOfText> textRegions) {
        this.textRegions = textRegions;
    }

    @OneToOne
    public Item getItem() {
        return item;
    }

    public void setItem(Item item) {
        this.item = item;
    }

    /**
     * Fetch the words that this object contains. A List is returned because not all languages use
     * consistent space-delimitation.  
     * @return a list of tokens that comprise all the words in this Text
     */
    public List<String> getWords() {
        return words;
    }

    public void setWords(List<String> words) {
        this.words = words;
    }
}
