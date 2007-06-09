package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import edu.cmu.sphinx.tools.confdesigner.ConfNodeWidget;
import org.netbeans.api.visual.action.EditProvider;
import org.netbeans.api.visual.widget.Widget;

import javax.swing.*;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class GraphEditProvider implements EditProvider {

    public void edit(Widget widget) {
        if(!(widget instanceof ConfNodeWidget))
        return;

        ConfNodeWidget nodeWidget = (ConfNodeWidget) widget;
        String s = JOptionPane.showInputDialog("Pleas enter new component name:", nodeWidget.getLabelWidget().getLabel());

        if (s != null && !s.equals(""))
            nodeWidget.setLabel(s);
    }
}
