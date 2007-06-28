package edu.cmu.sphinx.tools.confdesigner.actions;

import edu.cmu.sphinx.tools.confdesigner.ConfigScene;
import org.netbeans.api.visual.widget.Widget;

import javax.swing.*;
import java.awt.event.ActionEvent;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class RemoveNodeFromSceneAction extends AbstractAction {

    private Widget myWidget;


    public RemoveNodeFromSceneAction(Widget widget) {
        myWidget = widget;

        putValue(NAME, "Remove Node");
    }


    public void actionPerformed(ActionEvent e) {
        ConfigScene scene = (ConfigScene) myWidget.getScene();

//        scene.setSelectedObjects(new HashSet<Object>(Arrays.asList(myWidget)));
        scene.removeSelectedObjects();
//        scene.removeNode((ConfNode) scene.findObject(myWidget));
    }
}
