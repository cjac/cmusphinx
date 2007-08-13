package edu.cmu.sphinx.tools.confdesigner.propedit;

import javax.swing.*;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
class CellSpecificRenderTable extends JTable {

    private SimplePropEditor simplePropEditor;


    public CellSpecificRenderTable(SimplePropEditor simplePropEditor, EditorTableModel tableModel) {
        super(tableModel);
        this.simplePropEditor = simplePropEditor;
    }


    public TableCellRenderer getCellRenderer(int row, int column) {
        TableProperty tableProperty = simplePropEditor.getProperty(row);

        if (row >= 0 && column >= 0) {
            if (column == 0) {
                return tableProperty.getPropNameRenderer();
            } else {
                return tableProperty.getValueRenderer();
            }
        } else {
            super.getCellRenderer(row, column);
        }

        return null;
    }


    public TableCellEditor getCellEditor(int row, int column) {
        assert column == 1;

        return simplePropEditor.getProperty(row).getValueEditor();
    }
}
