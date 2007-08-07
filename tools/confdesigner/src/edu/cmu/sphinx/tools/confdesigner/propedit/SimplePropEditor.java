package edu.cmu.sphinx.tools.confdesigner.propedit;

import javax.swing.*;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class SimplePropEditor extends JPanel {

    JTable myTable;

    private JSplitPane split;
    private JScrollPane descriptionScrollPane;
    private JEditorPane descriptionPanel;


    public SimplePropEditor() {
        myTable = new JTable();

        descriptionScrollPane = new JScrollPane();
        descriptionPanel = new JEditorPane();
        descriptionScrollPane.setViewportView(descriptionPanel);

        split = new JSplitPane();
        split.setOrientation(JSplitPane.VERTICAL_SPLIT);
        split.add(myTable);
        split.add(descriptionPanel);
    }
}
