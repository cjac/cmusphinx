/**
 * Created by IntelliJ IDEA.
 * User: weinbergs
 * Date: Dec 27, 2006
 * Time: 11:23:17 PM
 * To change this template use File | Settings | File Templates.
 */

package edu.cmu.sphinx;

import javax.jws.WebService;
import javax.jws.WebMethod;
import javax.jws.soap.SOAPBinding;

@WebService(
        name = "EndpointInterface",
        targetNamespace = "http://edu.cmu.sphinx/riddler",
        serviceName = "Riddler")
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