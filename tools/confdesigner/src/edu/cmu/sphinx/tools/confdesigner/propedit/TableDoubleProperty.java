package edu.cmu.sphinx.tools.confdesigner.propedit;

import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.S4Double;

import javax.swing.*;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class TableDoubleProperty extends TableProperty {

    private PropertySheet currentPS;
    private String propName;
    private S4Double s4Double;
    private double[] range;


    public TableDoubleProperty(JTable myTable, PropertySheet currentPS, String propName) {
        super(propName, myTable, currentPS);
        this.currentPS = currentPS;
        this.propName = propName;
        this.s4Double = s4Double;

        this.range = s4Double.range();

        if (currentPS.getRaw(propName) != null) {
            setValue(currentPS.getDouble(propName));
        } else {
            double defValue = s4Double.defaultValue();
            if (defValue != S4Double.NOT_DEFINED)
                setValue(defValue);

            // set color to gray to indicate the defaultness
        }
    }


    public boolean isDefault() {
        return false;
    }


    public void restoreDefault() {
    }


    public void setValue(Object value) {
        assert value instanceof Double;

        Double newValue = (Double) value;
        // range checking is automatically done by the attached cell editor
        currentPS.setDouble(propName, newValue);
    }


    public Object getValue() {
        return null;
    }


    public TableCellRenderer getValueRenderer() {
        return new DefaultTableCellRenderer();
    }


    public TableCellEditor getValueEditor() {
        return new DoubleEditor(range[0], range[1]);
    }
}
