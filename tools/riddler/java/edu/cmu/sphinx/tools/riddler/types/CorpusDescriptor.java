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
 * Time: 9:16:28 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

import java.util.Calendar;

/**
 * information about a given corpus
 */
public class CorpusDescriptor {

    String name;
    String desc;
    /**
     * java.util.Date is not currently supported (by JAX-RPC 1.1) as an over-the-wire datatype
     */
    Calendar collectDate;

    public CorpusDescriptor(String name, String desc, Calendar collectDate) {
        this.name = name;
        this.desc = desc;
        this.collectDate = collectDate;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getDesc() {
        return desc;
    }

    public void setDesc(String desc) {
        this.desc = desc;
    }

    public Calendar getCollectDate() {
        return collectDate;
    }

    public void setCollectDate(Calendar collectDate) {
        this.collectDate = collectDate;
    }
}
