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
 * Date: Dec 28, 2006
 * Time: 8:14:30 PM
 */

package edu.cmu.sphinx.tools.riddler;

import java.rmi.Remote;

/**
 * document me!
 */
public interface EchoInterface extends Remote {
    public String echo(String input);
}