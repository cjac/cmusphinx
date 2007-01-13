/**
 * Created by IntelliJ IDEA.
 * User: weinbergs
 * Date: Dec 28, 2006
 * Time: 8:14:30 PM
 */

package edu.cmu.sphinx;

import java.rmi.Remote;

/**
 * document me!
 */
public interface EchoInterface extends Remote {
    public String echo(String input);
}