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

package edu.cmu.sphinx.tools.riddler.persist;

import javax.persistence.*;
import java.util.ArrayList;
import java.util.List;

/**
 * A Dictionary's unique identifier.  A Dictionary consists of Pronunciation records.
 * @see Pronunciation
 * @author Garrett Weinberg
 */
@Entity
@NamedQuery(name = "findDictionariesByMetadatum", query = "SELECT d FROM Dictionary d, " +
        "IN (d.metadata) m " + "WHERE m.theKey = :key AND m.theValue = :value")
public class Dictionary implements StringIdentified {

    @Id
    @GeneratedValue(strategy = GenerationType.TABLE)
    private String id;

    @OneToMany(cascade = CascadeType.ALL, fetch = FetchType.EAGER, mappedBy = "dictionary")
    private List<Pronunciation> prons = new ArrayList<Pronunciation>();

    @OneToMany(cascade = CascadeType.ALL, fetch = FetchType.LAZY)
    private List<Metadatum> metadata = new ArrayList<Metadatum>();

    public Dictionary(List<Pronunciation> prons, List<Metadatum> metadata) {
        this.prons = prons;
        this.metadata = metadata;
    }

    public Dictionary() {
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public List<Pronunciation> getProns() {
        return prons;
    }

    public void setProns(List<Pronunciation> prons) {
        this.prons = prons;
    }

    public List<Metadatum> getMetadata() {
        return metadata;
    }

    public void setMetadata(List<Metadatum> metadata) {
        this.metadata = metadata;
    }

    public String toString() {
        return "Dictionary " + super.toString() + " with ID " + id + " and metadata " + metadata;
    }
}
