package edu.cmu.sphinx.tools.confdesigner.propedit;

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

    private String propName;


    public TableProperty(String propName) {
        this.propName = propName;
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


    public Color getFieldColor() {
        return Color.GREEN;
    }


    public TableCellRenderer getNameRenderer() {
        return new DefaultTableCellRenderer();
    }


    public abstract TableCellEditor getValueEditor();


    public abstract TableCellRenderer getValueRenderer();
}
