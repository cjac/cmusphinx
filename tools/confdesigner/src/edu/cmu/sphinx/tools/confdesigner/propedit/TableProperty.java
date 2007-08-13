package edu.cmu.sphinx.tools.confdesigner.propedit;

import edu.cmu.sphinx.util.props.PropertySheet;

import javax.swing.*;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;
import java.awt.*;

/**
 * A generic property to be shown in a property JTable.
 *
 * @author Holger Brandl
 */
public abstract class TableProperty {

    protected JTable myTable;

    protected PropertySheet propSheet;

    private String propName;
    private boolean isUndefinedButMandatory;
    private DefaultTableCellRenderer propNameRenderer;


    public TableProperty(String propName, JTable myTable, PropertySheet currentPS) {
        this.myTable = myTable;
        this.propName = propName;
        this.propSheet = currentPS;

        propNameRenderer = new DefaultTableCellRenderer() {

            public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
                Component renderer = super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
                if (isUndefinedButMandatory)
                    renderer.setForeground(Color.RED);
                else
                    renderer.setForeground(Color.BLACK);

                if (isDefault()) {
                    renderer.setForeground(Color.LIGHT_GRAY);
                }

                return renderer;
            }
        };
    }


    public abstract boolean isDefault();


    public abstract void restoreDefault();


    public abstract void setValue(Object value);


    public abstract Object getValue();


    public abstract TableCellEditor getValueEditor();


    public abstract TableCellRenderer getValueRenderer();


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

}
