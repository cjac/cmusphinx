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
 * Time: 12:25:06 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

import java.util.Map;

/**
 * Contains metadata about a Dictionary
 */
public class DictionaryDescriptor {

    Map<String, String> metadata;

    public DictionaryDescriptor(Map<String, String> metadata) {
        this.metadata = metadata;
    }

    public DictionaryDescriptor() {
    }

    public Map<String, String> getMetadata() {
        return metadata;
    }

    public void setMetadata(Map<String, String> metadata) {
        this.metadata = metadata;
    }
}
