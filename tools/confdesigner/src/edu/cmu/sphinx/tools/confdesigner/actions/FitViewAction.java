package edu.cmu.sphinx.tools.confdesigner.actions;

import edu.cmu.sphinx.tools.confdesigner.ConfigScene;
import edu.cmu.sphinx.tools.confdesigner.SessionManager;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;

/**
 * Fits a given scene graph into the current panel-dimensions
 *
 * @author Holger Brandl
 */
public class FitViewAction extends AbstractAction {

    private SessionManager sesMan;


    public FitViewAction(SessionManager sesMan) {
        this.sesMan = sesMan;

        putValue(NAME, "FitView");
        putValue(ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_F, KeyEvent.ALT_MASK));
    }


    public void actionPerformed(ActionEvent e) {
        //determine the current viewport and the scene dimension and compute an apprpriate scaling factor
        ConfigScene scene = sesMan.getActiveScene().getScene();

        int width = (int) scene.getBounds().getWidth();
        int height = (int) scene.getBounds().getHeight();

        JComponent parent = (JComponent) scene.getView().getParent();
        if (parent instanceof JScrollPane || parent instanceof JPanel)
            parent = (JComponent) parent.getParent();

        int imWidth = parent.getWidth();
        int imHeight = parent.getHeight();

        double heigthRatio = imHeight / (double) height;
        double widthRatio = imWidth / (double) width;

        double scaleFactor = heigthRatio > widthRatio ? widthRatio : heigthRatio;
        scene.setZoomFactor(scaleFactor);
    }
}
