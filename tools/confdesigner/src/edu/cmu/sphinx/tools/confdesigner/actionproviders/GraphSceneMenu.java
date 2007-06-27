package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import edu.cmu.sphinx.tools.confdesigner.ConfigScene;
import edu.cmu.sphinx.tools.confdesigner.actions.NewBackgroundLabelAction;
import org.netbeans.api.visual.action.PopupMenuProvider;
import org.netbeans.api.visual.widget.Widget;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/** @author alex */
public class GraphSceneMenu implements PopupMenuProvider, ActionListener {

    private static final String ADD_NEW_NODE_ACTION = "addNewNodeAction"; // NOI18N

    private ConfigScene scene;

    private JPopupMenu menu;
    private Point point;

    private int nodeCount = 3;


    public GraphSceneMenu(ConfigScene scene) {
        this.scene = scene;
        menu = new JPopupMenu("Scene Menu");
        JMenuItem item;

        item = new JMenuItem(new NewBackgroundLabelAction(scene));
        item.setActionCommand(ADD_NEW_NODE_ACTION);
        item.addActionListener(this);
        menu.add(item);
    }


    public JPopupMenu getPopupMenu(Widget widget, Point point) {
        this.point = point;
        return menu;
    }


    public void actionPerformed(ActionEvent e) {
        if (ADD_NEW_NODE_ACTION.equals(e.getActionCommand())) {
//            String hm = "Node"+(nodeCount++);
//            Widget newNode = scene.addNode(hm);
//            scene.getSceneAnimator().animatePreferredLocation(newNode,point);
//            scene.validate();
        }
    }

}
