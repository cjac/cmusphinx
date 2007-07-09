package edu.cmu.sphinx.tools.confdesigner.actions;

import edu.cmu.sphinx.tools.confdesigner.SessionManager;

import javax.swing.*;
import java.awt.datatransfer.Clipboard;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class CutSubGraphAction extends CopySubGraphAction {

    private SessionManager sesMan;
    private DeleteSubGraphAction delAction;


    public CutSubGraphAction(SessionManager sesMan, Clipboard clipBoard, DeleteSubGraphAction delAction) {
        super(sesMan, clipBoard);
        this.sesMan = sesMan;
        this.delAction = delAction;

        putValue(NAME, "Cut");
        putValue(ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_X, KeyEvent.CTRL_MASK));
        putValue(MNEMONIC_KEY, KeyEvent.VK_X);
    }


    public void actionPerformed(ActionEvent e) {
        super.actionPerformed(e);

        // delete all selected actions
        delAction.actionPerformed(e);
    }
}
