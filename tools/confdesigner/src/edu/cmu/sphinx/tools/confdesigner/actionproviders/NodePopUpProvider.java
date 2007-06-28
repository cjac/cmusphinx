package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import edu.cmu.sphinx.tools.confdesigner.actions.RemoveNodeFromSceneAction;
import org.netbeans.api.visual.action.PopupMenuProvider;
import org.netbeans.api.visual.widget.Widget;

import javax.swing.*;
import java.awt.*;

/**
 * A context menu for the single configurable graph nodes. It contains common node action like removing,  etc. .
 *
 * @author Holger Brandl
 */
public class NodePopUpProvider implements PopupMenuProvider {

    public JPopupMenu getPopupMenu(final Widget widget, Point point) {
        JPopupMenu menu = new JPopupMenu("node menu");

        JMenuItem item = new JMenuItem("expand defaults");
        menu.add(item);
        menu.add(new JSeparator());
        menu.add(new JMenuItem(new RemoveNodeFromSceneAction(widget)));

        return menu;
    }
}