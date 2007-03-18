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

/**
 * A Dictionary's unique identifier.  A Dictionary consists of Pronunciation records.
 * @see Pronunciation
 * @author Garrett Weinberg
 */
@Entity
public class Dictionary {

    @Id
    @GeneratedValue(strategy = GenerationType.TABLE)
    private long id;

    @OneToMany(cascade = CascadeType.ALL, fetch = FetchType.EAGER)
    private List<Pronunciation> prons;

    private Map<String, String> metadata;

    public Dictionary(long id, List<Pronunciation> prons, Map<String, String> metadata) {
        this.id = id;
        this.prons = prons;
        this.metadata = metadata;
    }

    public Dictionary() {
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
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
}
