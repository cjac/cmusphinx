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

import java.util.Date;
import java.util.Map;

/**
 * Describes a given corpus
 */
public class CorpusDescriptor {

    Map<String, String> metadata;
    Date collectDate;

    public CorpusDescriptor(Map<String, String> metadata, Date collectDate) {
        this.metadata = metadata;
        this.collectDate = collectDate;
    }

    public CorpusDescriptor() {
    }

    public Map<String, String> getMetadata() {
        return metadata;
    }

    public void setMetadata(Map<String, String> metadata) {
        this.metadata = metadata;
    }

    public Date getCollectDate() {
        return collectDate;
    }

    public void setCollectDate(Date collectDate) {
        this.collectDate = collectDate;
    }
}
