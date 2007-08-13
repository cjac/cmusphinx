package edu.cmu.sphinx.tools.confdesigner.propedit;

import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.S4Integer;

import javax.swing.*;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class TableIntegerProperty extends TableProperty {


    private String NOT_DEFINED = "Not defined";
    private DefaultTableCellRenderer renderer = new DefaultTableCellRenderer();

    private S4Integer s4Integer;
    private IntegerEditor doubleEditor;


    public TableIntegerProperty(JTable myTable, PropertySheet currentPS, String propName) {
        super(propName, myTable, currentPS);

        this.s4Integer = (S4Integer) currentPS.getProperty(propName, S4Integer.class).getAnnotation();
        doubleEditor = new IntegerEditor(s4Integer.range()[0], s4Integer.range()[1]);

        Object rawValue = propSheet.getInt(propName);
        if (rawValue != null)
            doubleEditor.ftf.setValue(rawValue);

        doubleEditor.ftf.invalidate();
//        else
//        doubleEditor.ftf.setValue(null);
    }


    public boolean isDefault() {
        if (propSheet.getRaw(getPropName()) == null)
            return false;
        else
            return s4Integer.defaultValue() == (Integer) getValue();
    }


    public void restoreDefault() {
        if (s4Integer.defaultValue() == S4Integer.NOT_DEFINED)
            setValue(null);
        else
            setValue(s4Integer.defaultValue());
    }


    public void setValue(Object value) {
//        assert value instanceof String;

        Integer doubleValue = null;
        if (value == null) {
            setUndefinedButMandatory(true);

        } else {
            setUndefinedButMandatory(false);
            doubleValue = (Integer) value;
        }

        myTable.repaint();

        // don't change anything if nothing has changed
        Object oldValue = propSheet.getInt(getPropName());
        if ((doubleValue == null && oldValue == null) || (doubleValue != null && doubleValue.equals(oldValue)))
            return;

        // range checking is automatically done by the attached cell editor
        propSheet.setInt(getPropName(), (Integer) getValue());
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
