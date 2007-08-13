package edu.cmu.sphinx.tools.confdesigner.propedit;

import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertySheet;

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
        myTable.getTableHeader().getColumnModel().getColumn(0).setHeaderValue("Name");
        myTable.getTableHeader().getColumnModel().getColumn(1).setHeaderValue("Value");
        myTable = new CellSpecificRenderTable(this, tableModel);

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
        split.setDividerSize(3);
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


    public TableProperty getProperty(int row) {
        return tableModel.getProperty(row);
    }


    public JTable getTable() {
        return myTable;
    }


    public static void main(String[] args) {
        JFrame frame = new JFrame("property editor");


        ConfigurationManager cm = new ConfigurationManager();
        String instName = "myLogMath";
        cm.addConfigurable(LogMath.class, instName);
        PropertySheet ps = cm.getPropertySheet(instName);

        SimplePropEditor editor = new SimplePropEditor();
//        editor.addProperty(new TableDoubleProperty(ps, instName, (S4Double) ps.getProperty(LogMath.PROP_LOG_BASE, S4Double.class).getAnnotation()));
        editor.addProperty(new TableBoolProperty(editor.getTable(), ps, LogMath.PROP_USE_ADD_TABLE));
        frame.getContentPane().add(editor);

        frame.setBounds(300, 400, 200, 400);
        frame.setVisible(true);
    }


}

//class PropertyCellRenderer extends DefaultListCellRenderer {
//
//        /** Creates a new instance of LocaleRenderer */
//        public PropertyCellRenderer() {
//        }
//
//
//        public Component getListCellRendererComponent(JList list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
//            super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
//
//            setText(value.toString());
//
//            Color strColor = getColor(list, index);
//            setForeground(strColor);
////                setBackground(color);
//
//            return this;
//        }
//
//
//        private Color getColor(JList list, int index) {
////            ListModel listModel = list.getModel();
////
////            if ((index + 1) >= listModel.getSize())
////                return Color.BLACK;
////
////            if (listModel.getElementAt(index + 1) instanceof VadSegment && listModel.getElementAt(index) instanceof VadSegment) {
////                return Color.GREEN;
////            }
////
////            if (listModel.getElementAt(index + 1) instanceof SegmentHypothesis && listModel.getElementAt(index) instanceof SegmentHypothesis) {
////                return Color.RED;
////            }
////
//            return Color.BLACK;
//        }
//    }
//
//
