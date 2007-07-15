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
 * Date: Apr 5, 2007
 * Time: 11:24:32 PM
 */

package edu.cmu.sphinx.tools.riddler.persist;

import edu.cmu.sphinx.tools.riddler.shared.MetadataWrapper;

import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;
import javax.persistence.GenerationType;
import java.util.List;
import java.util.Map;
import java.util.ArrayList;
import java.util.HashMap;

/**
 * class that exists solely to work around JPA's unwillingness to compare Map<String, String> objects to one
 * another through the query API
 */
@Entity
public class Metadatum {

    /**
     * just using integer ID's for these because they aren't exposed to clients
     */
    @Id
    @GeneratedValue(strategy = GenerationType.TABLE)
    int id;

    // "key" is a reserved word in SQL; hence this name
    private String theKey;
    private String theValue;

    public Metadatum() {
    }

    public Metadatum(String theKey, String theValue) {
        this.theKey = theKey;
        this.theValue = theValue;
    }

    public String getKey() {
        return theKey;
    }

    public void setKey(String theKey) {
        this.theKey = theKey;
    }

    public String getValue() {
        return theValue;
    }

    public void setValue(String theValue) {
        this.theValue = theValue;
    }

    public static List<Metadatum> listFromWrapper(MetadataWrapper wrapper) {
        List<Metadatum> out = new ArrayList<Metadatum>();
        for (Map.Entry<String, String> e : wrapper.getContents().entrySet())
            out.add(new Metadatum(e.getKey(), e.getValue()));
        return out;
    }

    public static MetadataWrapper wrapperFromList(List<Metadatum> list) {
        Map<String, String> contents = new HashMap<String, String>();
        for (Metadatum m : list)
            contents.put(m.getKey(), m.getValue());
        return new MetadataWrapper(contents);
    }
}
