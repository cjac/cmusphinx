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

    private String myName;
    private Object myValue;


    public abstract void add(SimplePropEditor simplePropEditor);


    public void setDisplayName(String propName) {
        myName = propName;
    }


    public abstract void setValue(Object value);


    public Object getValue() {
        return myValue;
    }


    public String getDisplayName() {
        return myName;
    }


    public Color getFieldColor() {
        return Color.GREEN;
    }


    public TableCellRenderer getNameRenderer() {
        return new DefaultTableCellRenderer();
    }


    public TableCellRenderer getValueRenderer() {
        return new DefaultTableCellRenderer();
    }


    public abstract TableCellEditor getValueEditor();
}
