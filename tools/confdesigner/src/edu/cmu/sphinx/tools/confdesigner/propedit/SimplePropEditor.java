package edu.cmu.sphinx.tools.confdesigner.propedit;

import de.hondari.carl.azubi.dsp.VadSegment;
import de.hondari.carl.azubi.segspot.SegmentHypothesis;

import javax.swing.*;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;
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
//        myTable = new JTable(tableModel);
        myTable = new CellSpecificRenderTable(tableModel);

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


    public JTable getTable() {
        return myTable;
    }


    public class PropertyCellRenderer extends DefaultListCellRenderer {

        /** Creates a new instance of LocaleRenderer */
        public PropertyCellRenderer() {
        }


        public Component getListCellRendererComponent(JList list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
            super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);

            setText(value.toString());

            Color strColor = getColor(list, index);
            setForeground(strColor);
//                setBackground(color);

            return this;
        }


        private Color getColor(JList list, int index) {
            ListModel listModel = list.getModel();

            if ((index + 1) >= listModel.getSize())
                return Color.BLACK;

            if (listModel.getElementAt(index + 1) instanceof VadSegment && listModel.getElementAt(index) instanceof VadSegment) {
                return Color.GREEN;
            }

            if (listModel.getElementAt(index + 1) instanceof SegmentHypothesis && listModel.getElementAt(index) instanceof SegmentHypothesis) {
                return Color.RED;
            }

            return Color.BLACK;
        }
    }

    class CellSpecificRenderTable extends JTable {

        public CellSpecificRenderTable(EditorTableModel tableModel) {
            super(tableModel);
        }


        public TableCellRenderer getCellRenderer(int row, int column) {
            TableProperty tableProperty = tableModel.getProperty(row);

            if (column == 0) {
                return tableProperty.getNameRenderer();
            } else {
                return tableProperty.getValueRenderer();
            }

        }


        public TableCellEditor getCellEditor(int row, int column) {
            assert column == 1;

            return tableModel.getProperty(row).getValueEditor();
        }
    }
}


