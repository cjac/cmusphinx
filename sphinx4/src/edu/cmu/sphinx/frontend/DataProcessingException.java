/*
 * Copyright 1999-2002 Carnegie Mellon University.  
 * Portions Copyright 2002 Sun Microsystems, Inc.  
 * Portions Copyright 2002 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 * 
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL 
 * WARRANTIES.
 *
 */


package edu.cmu.sphinx.frontend;

/**
 * Thrown to indicate that a DataProcessor has problems processing
 * incoming Data objects.
 */
public class DataProcessingException extends Exception {

    /**
     * Constructs a DataProcessingException with no detailed message.
     */
    public DataProcessingException() {
        super();
    }

    /**
     * Constructs a DataProcessingException with the specified
     * detailed message.
     *
     * @param message the detailed message
     */
    public DataProcessingException(String message) {
        super(message);
    }
}
