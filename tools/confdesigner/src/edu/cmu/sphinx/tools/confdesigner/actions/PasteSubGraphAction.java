package edu.cmu.sphinx.tools.confdesigner.actions;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class PasteSubGraphAction extends AbstractAction {

    public PasteSubGraphAction() {
        putValue(NAME, "Paste");
        putValue(ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_V, KeyEvent.CTRL_MASK));
        putValue(MNEMONIC_KEY, KeyEvent.VK_V);
    }


    public void actionPerformed(ActionEvent e) {

    }
}
