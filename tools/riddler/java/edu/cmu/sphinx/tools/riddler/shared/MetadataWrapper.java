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
 * Date: Mar 11, 2007
 * Time: 6:50:17 PM
 */

package edu.cmu.sphinx.tools.riddler.shared;

import java.util.HashMap;
import java.util.Map;
import java.io.Serializable;

/**
 * HashMap wrapper class to work around the JAX-WS issue mentioned here: http://forums.java.net/jive/message.jspa?messageID=185244 
 * @author Garrett Weinberg
 */
public class MetadataWrapper implements Serializable {

    private Map<String, String> contents;

    public MetadataWrapper(Map<String, String> contents) {
        this.contents = contents;
    }

    public MetadataWrapper() {
        contents = new HashMap();
    }

    public Map<String, String> getContents() {
        return contents;
    }

    public void setContents(Map<String, String> contents) {
        this.contents = contents;
    }

    public String toString() {
        return contents.toString();
    }
}
