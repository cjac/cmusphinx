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


package edu.cmu.sphinx.util;

/**
 * An ID generator that gives out positive integer IDs and
 * checks for overflow.
 */ 
public class IDGenerator {

    /**
     * Indicates a non-ID value, which is -1.
     */
    public final static int NON_ID = -1;

    private int id = 0;
    
    /**
     * Returns the next valid ID, checks for integer overflow.
     *
     * @return the next valid ID, goes back to zero if overflow
     */
    public int getNextID() {
        if (id == Integer.MAX_VALUE) {
            return (id = 0);
        } else {
            return id++;
        }
    }

    /**
     * Resets the ID to zero.
     */
    public void reset() {
        id = 0;
    }
}


