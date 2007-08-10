package edu.cmu.sphinx.tools.confdesigner.propedit;

import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.S4Boolean;

import javax.swing.*;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class TableBoolProperty extends TableProperty {

    private PropertySheet propSheet;
    private S4Boolean s4Boolean;
    private String NOT_DEFINED = "Not defined";
    private ComboTableCellRenderer renderer = new ComboTableCellRenderer();
    private JComboBox comboBox = new JComboBox(new Object[]{NOT_DEFINED, Boolean.TRUE, Boolean.FALSE});


    public TableBoolProperty(PropertySheet currentPS, String propName) {
        super(propName);
        this.propSheet = currentPS;
        this.s4Boolean = (S4Boolean) currentPS.getProperty(propName, S4Boolean.class).getAnnotation();


        Boolean defValue = propSheet.getBoolean(propName);
        if (defValue == null)
            comboBox.getModel().setSelectedItem(NOT_DEFINED);
        else
            comboBox.getModel().setSelectedItem(defValue);

        comboBox.setRenderer(renderer);
    }


    public void setValue(Object value) {
//        assert value instanceof String;

        Boolean boolValue = null;
//        if(!value.equals(NOT_DEFINED))
//            boolValue = Boolean.getBoolean((String) value);

        // don't change anything if nothing has changed
        Object oldValue = propSheet.getRaw(getPropName());
        if ((boolValue == null && oldValue == null) || boolValue.equals(oldValue))
            return;

        // range checking is automatically done by the attached cell editor
        propSheet.setBoolean(getPropName(), boolValue);
    }


    public Object getValue() {
        return comboBox.getSelectedItem();
    }


    public TableCellRenderer getNameRenderer() {
        return new DefaultTableCellRenderer();
    }


    public TableCellRenderer getValueRenderer() {
        return renderer;
    }


    public TableCellEditor getValueEditor() {
        return new DefaultCellEditor(comboBox);
    }
}
