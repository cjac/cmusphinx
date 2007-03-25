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

import javax.persistence.*;
import java.util.List;
import java.util.Map;
import java.util.ArrayList;
import java.util.HashMap;
import java.awt.*;

/**
 * A Dictionary's unique identifier.  A Dictionary consists of Pronunciation records.
 * @see Pronunciation
 * @author Garrett Weinberg
 */
@Entity
@NamedQuery(name = "findDictionaryByMetadata", query = "SELECT DISTINCT d FROM Dictionary d WHERE d.metadata = :metadata")
public class Dictionary {

    @Id
    @GeneratedValue(strategy = GenerationType.TABLE)
    private String id;

    @OneToMany(cascade = CascadeType.ALL, fetch = FetchType.EAGER, mappedBy = "dictionary")
    private List<Pronunciation> prons = new ArrayList<Pronunciation>();

    @Lob @Basic
    private Map<String, String> metadata = new HashMap<String, String>();

    public Dictionary(List<Pronunciation> prons, Map<String, String> metadata) {
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

    public Map<String, String> getMetadata() {
        return metadata;
    }

    public void setMetadata(Map<String, String> metadata) {
        this.metadata = metadata;
    }

    public String toString() {
        return "Dictionary " + super.toString() + " with ID " + id + " and metadata " + metadata;
    }
}
