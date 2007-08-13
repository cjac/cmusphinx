package edu.cmu.sphinx.tools.confdesigner.propedit;

import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.S4Boolean;

import javax.swing.*;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;

/** @author Holger Brandl */
public class TableBoolProperty extends TableProperty {


    private S4Boolean s4Boolean;

    private String NOT_DEFINED = "Not defined";
    private ComboTableCellRenderer renderer = new ComboTableCellRenderer();
    private JComboBox comboBox = new JComboBox(new Object[]{NOT_DEFINED, Boolean.TRUE, Boolean.FALSE});


    public TableBoolProperty(JTable myTable, PropertySheet currentPS, String propName) {
        super(propName, myTable, currentPS);

        this.s4Boolean = (S4Boolean) currentPS.getProperty(propName, S4Boolean.class).getAnnotation();

        Boolean defValue = propSheet.getBoolean(propName);
        if (defValue == null)
            comboBox.getModel().setSelectedItem(NOT_DEFINED);
        else
            comboBox.getModel().setSelectedItem(defValue);

        comboBox.setRenderer(renderer);
    }


    public boolean isDefault() {
        Object comboValue = comboBox.getModel().getSelectedItem();

        if (s4Boolean.isNotDefined() && !comboValue.equals(NOT_DEFINED))
            return false;
        else
            return comboValue.equals(s4Boolean.defaultValue());
    }


    public void restoreDefault() {
        if (s4Boolean.isNotDefined())
            setValue(null);
        else
            setValue(s4Boolean.defaultValue());
    }


    public void setValue(Object value) {
//        assert value instanceof String;

        Boolean boolValue = null;
        if (value.equals(NOT_DEFINED)) {
            setUndefinedButMandatory(true);

        } else {
            setUndefinedButMandatory(false);
            boolValue = (Boolean) value;
        }

        myTable.repaint();

        // don't change anything if nothing has changed
        Object oldValue = propSheet.getBoolean(getPropName());
        if ((boolValue == null && oldValue == null) || (boolValue != null && boolValue.equals(oldValue)))
            return;

        // range checking is automatically done by the attached cell editor
        propSheet.setBoolean(getPropName(), boolValue);
    }


    public Object getValue() {
        return comboBox.getSelectedItem();
    }


    public TableCellRenderer getValueRenderer() {
        return renderer;
    }


    public TableCellEditor getValueEditor() {
        return new DefaultCellEditor(comboBox);
    }
}
