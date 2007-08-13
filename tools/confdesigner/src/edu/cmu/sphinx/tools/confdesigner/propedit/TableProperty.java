package edu.cmu.sphinx.tools.confdesigner.propedit;

import javax.swing.*;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;
import java.awt.*;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public abstract class TableProperty {

    protected JTable myTable;

    private String propName;
    private boolean isUndefinedButMandatory;
    private DefaultTableCellRenderer propNameRenderer;


    public TableProperty(String propName, JTable myTable) {
        this.myTable = myTable;
        this.propName = propName;

        propNameRenderer = new DefaultTableCellRenderer() {

            // implements javax.swing.table.TableCellRenderer
            public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
                Component renderer = super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
                if (isUndefinedButMandatory)
                    renderer.setForeground(Color.RED);
                else
                    renderer.setForeground(Color.BLACK);

                return renderer;

            }
        };
    }


    public void setDisplayName(String propName) {
    }


    public abstract void setValue(Object value);


    public abstract Object getValue();


    public String getPropName() {
        return propName;
    }


    public String toString() {
        return propName;
    }


    public Color getPropNameColor() {
        if (isUndefinedButMandatory)
            return Color.RED;
        else
            return Color.GREEN;
    }


    public DefaultTableCellRenderer getPropNameRenderer() {
        return propNameRenderer;
    }


    public void setUndefinedButMandatory(boolean undefinedButMandatory) {
        isUndefinedButMandatory = undefinedButMandatory;
    }


    public abstract TableCellEditor getValueEditor();


    public abstract TableCellRenderer getValueRenderer();
}
