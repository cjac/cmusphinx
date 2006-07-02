package edu.cmu.sphinx.tools.dictator;

import java.awt.*;
import java.awt.datatransfer.*;
import java.io.IOException;

/**
 * Copyright 1999-2006 Carnegie Mellon University.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * User: Peter Wolf
 * Date: Apr 30, 2006
 * Time: 9:02:48 AM
 */
public class ClipboardControls implements ClipboardOwner {

    /**
     * Empty implementation of the ClipboardOwner interface.
     */
    public void lostOwnership(Clipboard aClipboard, Transferable aContents) {
        //do nothing
    }

    /**
     * Place a String on the clipboard, and make this class the
     * owner of the Clipboard's contents.
     */
    public void setClipboardContents(String aString) {
        StringSelection stringSelection = new StringSelection(aString);
        Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
        clipboard.setContents(stringSelection, this);
    }

    /**
     * Get the String residing on the clipboard.
     *
     * @return any text found on the Clipboard; if none found, return an
     *         empty String.
     */
    public String getClipboardContents() {
        String result = "";
        Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
        //odd: the Object param of getContents is not currently used
        Transferable contents = clipboard.getContents(null);
        boolean hasTransferableText =
                (contents != null) &&
                        contents.isDataFlavorSupported(DataFlavor.stringFlavor);
        if (hasTransferableText) {
            try {
                result = (String) contents.getTransferData(DataFlavor.stringFlavor);
            }
            catch (UnsupportedFlavorException ex) {
                //highly unlikely since we are using a standard DataFlavor
                System.out.println(ex);
                ex.printStackTrace();
            }
            catch (IOException ex) {
                System.out.println(ex);
                ex.printStackTrace();
            }
        }
        return result;
    }

}
