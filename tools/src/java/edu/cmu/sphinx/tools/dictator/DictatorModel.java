package edu.cmu.sphinx.tools.dictator;

import javax.swing.*;

/**
 * Copyright 1999-2006 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * Portions Copyright 2002 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * User: Peter Wolf
 * Date: Apr 28, 2006
 * Time: 10:18:36 PM
 */
public class DictatorModel {

    DictatorView view;
    ClipboardControls clipboard;

    DictatorModel(String configFile) {
        try {
            clipboard = new ClipboardControls();

            view = new DictatorView();
            JFrame f = new JFrame("Haikya Dictator");

            f.setContentPane(view.mainPanel);
            f.pack();
            f.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

            f.setVisible(true);
            view.open(configFile,"");

        } catch (Exception e) {
            throw new Error(e);
        }
    }


    public static void main(String [] args) {

        ClipboardControls cc = new ClipboardControls();

        //display what is currently on the clipboard
        System.out.println("Clipboard contains:" + cc.getClipboardContents());

        //change the contents and then re-display
        cc.setClipboardContents("blah, blah, blah");

        System.out.println("Clipboard contains:" + cc.getClipboardContents());

    }


}
