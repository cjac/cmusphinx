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
 * Date: Dec 27, 2006
 * Time: 11:23:17 PM
 */

package edu.cmu.sphinx.tools.riddler;

import javax.jws.WebMethod;
import javax.jws.WebService;
import javax.jws.soap.SOAPBinding;

@WebService(
        name = "EchoInterface",
        targetNamespace = "http://edu.cmu.sphinx/riddler",
        serviceName = "Echo")
@SOAPBinding(style = SOAPBinding.Style.RPC)
public class JSEBean01 implements EchoInterface
{
    @WebMethod
    public String echo(String input)
    {
        System.out.println("I'm Hit! " + input);
        return input;
    }
}