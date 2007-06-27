package edu.cmu.sphinx.tools.confdesigner.actions;

import edu.cmu.sphinx.tools.confdesigner.ConfigScene;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class NewBackgroundLabelAction extends AbstractAction {

    private ConfigScene scene;
    private Point lastLocation;


    public NewBackgroundLabelAction(ConfigScene scene) {
        this.scene = scene;

        putValue(NAME, "Add Background Label Here");
        scene.getView().addMouseMotionListener(new MouseMotionListener() {

            public void mouseDragged(MouseEvent e) {

            }


            public void mouseMoved(MouseEvent e) {
                lastLocation = e.getPoint();
            }
        });
    }


    public void actionPerformed(ActionEvent e) {
        // create a new background node

        System.out.println("" + e);

        assert lastLocation != null;

        scene.createLabel("Unnamed", lastLocation);

//        scene.createBackgroundNode()
    }
}
