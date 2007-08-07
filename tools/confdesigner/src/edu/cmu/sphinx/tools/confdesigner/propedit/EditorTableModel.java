package edu.cmu.sphinx.tools.confdesigner.propedit;

import javax.swing.*;
import javax.swing.table.AbstractTableModel;
import java.util.ArrayList;
import java.util.List;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class EditorTableModel extends AbstractTableModel {

    List<TableProperty> props = new ArrayList<TableProperty>();


    public int getRowCount() {
        return props.size();
    }


    public int getColumnCount() {
        return 2;
    }


    public Object getValueAt(int rowIndex, int columnIndex) {
        TableProperty property = props.get(rowIndex);

        //todo remove this!!
        if (property == null)
            return "test";

        if (columnIndex == 0) {
            return property.getDisplayName();
        } else {
            return property.getValue();
        }

    }


    public void addProperty(TableProperty property) {
        assert props != null;
        props.add(property);

        fireTableDataChanged();
    }


    public void clear() {
        props.clear();
        fireTableDataChanged();
    }


    public boolean isCellEditable(int rowIndex, int columnIndex) {
        return columnIndex > 0 && !(getValueAt(rowIndex, columnIndex) instanceof JComboBox);
    }
}
