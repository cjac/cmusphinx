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


    private String NOT_DEFINED = "Not defined";
    private DefaultTableCellRenderer renderer = new DefaultTableCellRenderer();

    private S4Double s4Double;
    private DoubleEditor doubleEditor;


    public TableDoubleProperty(JTable myTable, PropertySheet currentPS, String propName) {
        super(propName, myTable, currentPS);

        this.s4Double = (S4Double) currentPS.getProperty(propName, S4Double.class).getAnnotation();
        doubleEditor = new DoubleEditor(s4Double.range()[0], s4Double.range()[1]);

        Object rawValue = propSheet.getDouble(propName);
        if (rawValue != null)
            doubleEditor.ftf.setValue(rawValue);

        doubleEditor.ftf.invalidate();
    }

//    public TableBoolProperty(JTable myTable, PropertySheet currentPS, String propName) {
//        super(propName, myTable, currentPS);
//
//        this.s4Boolean = (S4Boolean) currentPS.getProperty(propName, S4Boolean.class).getAnnotation();
//
//        Boolean defValue = propSheet.getBoolean(propName);
//        if (defValue == null)
//            comboBox.getModel().setSelectedItem(NOT_DEFINED);
//        else
//            comboBox.getModel().setSelectedItem(defValue);
//
//        comboBox.setRenderer(renderer);
//    }


    public boolean isDefault() {
        if (propSheet.getRaw(getPropName()) == null)
            return false;
        else
            return s4Double.defaultValue() == (Double) getValue();
    }


    public void restoreDefault() {
        if (s4Double.defaultValue() == S4Double.NOT_DEFINED)
            setValue(null);
        else
            setValue(s4Double.defaultValue());
    }


    public void setValue(Object value) {
//        assert value instanceof String;

        Double doubleValue = null;
        if (value == null) {
            setUndefinedButMandatory(true);

        } else {
            setUndefinedButMandatory(false);
            doubleValue = (Double) value;
        }

        myTable.repaint();

        // don't change anything if nothing has changed
        Object oldValue = propSheet.getDouble(getPropName());
        if ((doubleValue == null && oldValue == null) || (doubleValue != null && doubleValue.equals(oldValue)))
            return;

        // range checking is automatically done by the attached cell editor
        propSheet.setDouble(getPropName(), (Double) getValue());
    }


    public Object getValue() {
        return doubleEditor.getCellEditorValue();
    }


    public TableCellRenderer getValueRenderer() {
        return renderer;
    }


    public TableCellEditor getValueEditor() {
        return doubleEditor;
    }
}
