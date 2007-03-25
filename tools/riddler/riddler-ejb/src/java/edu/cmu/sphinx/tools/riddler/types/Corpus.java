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
 * Time: 8:23:37 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

import javax.persistence.*;
import java.util.*;

/**
 * A Corpus' unique identifier.  A Corpus consists of Items and has an associated Dictionary
 * mapping Words to pronunciations.
 * @see Item
 * @see Pronunciation
 * @author Garrett Weinberg
 */
@Entity
public class Corpus {

    @Id
    @GeneratedValue(strategy = GenerationType.TABLE)
    private String id;

    @OneToMany(cascade = CascadeType.PERSIST, fetch = FetchType.EAGER, mappedBy = "corpus")
    private List<Item> items = new ArrayList<Item>();

    @OneToOne(cascade = CascadeType.REFRESH, fetch = FetchType.LAZY)
    private Dictionary dictionary;

    @Temporal(value = TemporalType.DATE)
    private Date collectDate;

    @Lob @Basic
    private Map<String, String> metadata = new HashMap<String, String>();

    public Corpus(List<Item> items, Dictionary dictionary, Date collectDate, Map<String, String> metadata) {
        this.items = items;
        this.dictionary = dictionary;
        this.collectDate = collectDate;
        this.metadata = metadata;
    }

    public Corpus() {
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public List<Item> getItems() {
        return items;
    }

    public void setItemIDs(List<Item> items) {
        this.items = items;
    }

    public Dictionary getDictionary() {
        return dictionary;
    }

    public void setDictionary(Dictionary dictionary) {
        this.dictionary = dictionary;
    }

    public Date getCollectDate() {
        return collectDate;
    }

    public void setCollectDate(Date collectDate) {
        this.collectDate = collectDate;
    }

    public Map<String, String> getMetadata() {
        return metadata;
    }

    public void setMetadata(Map<String, String> metadata) {
        this.metadata = metadata;
    }
}
