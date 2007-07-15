/*
 * Main.java
 *
 * Created on February 11, 2007, 4:30 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package edu.cmu.sphinx.tools.riddler.client;

import edu.cmu.sphinx.tools.riddler.shared.RiddlerRemote;
import edu.cmu.sphinx.tools.riddler.shared.MetadataWrapper;

import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.swing.*;
import java.util.Map;
import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.rmi.RemoteException;

/**
 *
 * example EJB client to Riddler
 */
public class Main {

    RiddlerRemote riddler;

    Logger logger = Logger.getLogger(getClass().getName());

    /** Creates a new instance of Main */
    public Main() {
        try  {
            InitialContext ctx = new InitialContext();
            riddler = (RiddlerRemote) ctx.lookup(RiddlerRemote.class.getName());

            MetadataWrapper mw = new MetadataWrapper();
            Map<String, String> contents = new HashMap<String, String>();
            contents.put("SomeKey", "SomeValue");
            mw.setContents(contents);


            String newID = riddler.createDictionary(mw);
            String retrievedID = riddler.getDictionary(mw);
            assert newID.equals(retrievedID);
            logger.log(Level.INFO, "Riddler EJB Client exiting successfully.");
        }
        catch (RemoteException e) {
            logger.log(Level.SEVERE, "Error", e);
        }
        catch (NamingException e) {
            logger.log(Level.SEVERE, "Error", e);
        }
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        new Main();
    }

}
