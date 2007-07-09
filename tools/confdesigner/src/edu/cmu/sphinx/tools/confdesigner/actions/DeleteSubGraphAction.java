package edu.cmu.sphinx.tools.confdesigner.actions;

import edu.cmu.sphinx.tools.confdesigner.SessionManager;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class DeleteSubGraphAction extends AbstractAction {

    private SessionManager sesMan;


    public DeleteSubGraphAction(SessionManager sesMan) {
        this.sesMan = sesMan;

        putValue(NAME, "Delete");
        putValue(ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_DELETE, 0));
        putValue(MNEMONIC_KEY, KeyEvent.VK_D);
    }


    public void actionPerformed(ActionEvent e) {
        sesMan.getActiveScene().getScene().removeSelectedObjects();
    }
}
