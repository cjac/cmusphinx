package edu.cmu.sphinx.tools.confdesigner.propedit;

import javax.swing.*;
import java.awt.*;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class SimplePropEditor extends JPanel {

    private JTable myTable;
    private EditorTableModel tableModel;

    private JSplitPane split;
    private JScrollPane descriptionScrollPane;
    private JEditorPane descriptionPanel;


    public SimplePropEditor() {
        tableModel = new EditorTableModel();
        myTable = new JTable(tableModel);
        myTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

        descriptionScrollPane = new JScrollPane();
        descriptionPanel = new JEditorPane();
        descriptionScrollPane.setViewportView(descriptionPanel);

        JScrollPane tableScrolPane = new JScrollPane();
        tableScrolPane.setViewportView(myTable);

        split = new JSplitPane();
        split.setOrientation(JSplitPane.VERTICAL_SPLIT);
        split.add(tableScrolPane, JSplitPane.TOP);
        split.add(descriptionPanel, JSplitPane.BOTTOM);
        split.setDividerLocation(0.9);
        split.setResizeWeight(0.9);

        setLayout(new BorderLayout());
        add(split);
    }


    public void clear() {
        tableModel.clear();


    }


    public void addProperty(TableProperty p) {
        tableModel.addProperty(p);
//        myTable.validate();
    }
}
