package edu.cmu.sphinx.tools.confdesigner.actions;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class DeleteSubGraphAction extends AbstractAction {

    public DeleteSubGraphAction() {
        putValue(NAME, "Delete");
        putValue(ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_DELETE, 0));
        putValue(MNEMONIC_KEY, KeyEvent.VK_D);
    }


    public void actionPerformed(ActionEvent e) {

    }
}
