package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import edu.cmu.sphinx.tools.confdesigner.ConfNodeWidget;
import org.netbeans.api.visual.action.PopupMenuProvider;
import org.netbeans.api.visual.widget.Widget;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class GraphPopUpProvider implements PopupMenuProvider {

    public JPopupMenu getPopupMenu(final Widget widget, Point point) {
        JPopupMenu menu = new JPopupMenu("test");

        ActionListener al = new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                JMenuItem sourceItem = (JMenuItem) e.getSource();
                if (sourceItem.getText().contains("rename")) {
                    ConfNodeWidget iconNodeWidget = (ConfNodeWidget) widget;

                    String s = JOptionPane.showInputDialog("Pleas enter new component name:", iconNodeWidget.getLabelWidget().getLabel());
                    if (s != null && !s.equals(""))
                        iconNodeWidget.setLabel(s);
                }

            }
        };

        JMenuItem item = new JMenuItem("expand defaults");
        item.addActionListener(al);
        menu.add(item);

        item = new JMenuItem("remove node");
        item.addActionListener(al);
        menu.add(item);

        item = new JMenuItem("rename node");
        item.addActionListener(al);
        menu.add(item);

        return menu;
    }
}
