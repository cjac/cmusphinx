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

package edu.cmu.sphinx.tools.riddler.persist;

import javax.persistence.*;
import java.util.List;
import java.util.ArrayList;

/**
 * A text record's unique identifier. Text records belong to an Item and consist of one or more
 * RegionOfText records.
 * @see Item
 * @see RegionOfText
 * @author Garrett Weinberg
 */
@Entity
public class Text {
    @Id
    @GeneratedValue(strategy = GenerationType.TABLE)
    private String id;

    @OneToMany(cascade = CascadeType.ALL, fetch = FetchType.EAGER, mappedBy="text")
    private List<RegionOfText> textRegions = new ArrayList<RegionOfText>();

    /**
     * has to be an array list (Serializable type) since it's a Blob
     */
    @Lob @Basic
    private ArrayList<String> words = new ArrayList<String>();
    /**
     * parent reference, for bi-directional fetching
     */
    @OneToOne
    private Item item;

    public Text(List<RegionOfText> textRegions, ArrayList<String> words) {
        this.textRegions = textRegions;
        this.words = words;
    }

    public Text() {
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public List<RegionOfText> getTextRegions() {
        return textRegions;
    }

    public void setTextRegions(List<RegionOfText> textRegions) {
        this.textRegions = textRegions;
    }

    public Item getItem() {
        return item;
    }

    public void setItem(Item item) {
        this.item = item;
    }

    /**
     * Fetch the words that this object contains. A list is returned because not all languages use
     * consistent space-delimitation.  
     * @return a list of tokens that comprise all the words in this Text
     */
    public List<String> getWords() {
        return words;
    }

    public void setWords(ArrayList<String> words) {
        this.words = words;
    }
}
