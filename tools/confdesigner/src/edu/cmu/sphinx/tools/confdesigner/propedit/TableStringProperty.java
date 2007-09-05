package edu.cmu.sphinx.tools.confdesigner.propedit;

import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.S4String;

import javax.swing.*;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class TableStringProperty extends TableProperty {

    private S4String s4String;

    private String NOT_DEFINED = "Not defined";
    private JComboBox comboBox;

    private boolean isRanged;
    private TableCellRenderer renderer;


    public TableStringProperty(JTable myTable, PropertySheet currentPS, String propName) {
        super(propName, myTable, currentPS);

        this.s4String = (S4String) currentPS.getProperty(propName, S4String.class).getAnnotation();

        if (currentPS.getRaw(propName) != null) {
            setValue(currentPS.getString(propName));
            renderer = new DefaultTableCellRenderer();
        } else {
            String defValue = s4String.defaultValue();
            String[] range = s4String.range();
            if (range.length != 0) {
                isRanged = true;

                String[] comboRange = new String[range.length + 1];
                comboRange[0] = NOT_DEFINED;
                System.arraycopy(range, 0, comboRange, 1, range.length);

                comboBox = new JComboBox(comboRange);
                setValue(comboBox);

                renderer = new ComboTableCellRenderer();
                comboBox.setRenderer((ListCellRenderer) renderer);
            } else {
                if (defValue.equals(S4String.NOT_DEFINED))
                    setValue(NOT_DEFINED);

                renderer = new DefaultTableCellRenderer();
            }

            // set color to gray to indicate the defaultness
        }
    }


    public boolean isDefault() {
        String curValue = (String) getValue();

        if ((curValue.equals(NOT_DEFINED) && s4String.defaultValue().equals(S4String.NOT_DEFINED)))
            return true;

        return curValue.equals(s4String.defaultValue());
    }


    public void restoreDefault() {
        setValue(s4String.defaultValue());
    }


    public void setValue(Object value) {
        myTable.repaint();

        // don't change anything if nothing has changed
        Object oldValue = propSheet.getString(getPropName());
        if ((value == null && oldValue == null) || (value != null && value.equals(oldValue)))
            return;

        // range checking is automatically done by the attached cell editor
        if (comboBox != null) {
            String comboValue = (String) comboBox.getModel().getSelectedItem();

            if (comboValue.equals(NOT_DEFINED)) {
                propSheet.setString(getPropName(), null);
                setUndefinedButMandatory(s4String.mandatory());
            } else {
                propSheet.setString(getPropName(), comboValue);
                setUndefinedButMandatory(false && s4String.mandatory());
            }
        } else {
            propSheet.setString(getPropName(), (String) value);
            setUndefinedButMandatory((value == null || ((String) value).trim().length() == 0) && s4String.mandatory());
        }
    }


    public Object getValue() {
        if (comboBox != null)
            return comboBox.getSelectedItem();
        else
            return propSheet.getString(getPropName());
    }


    public TableCellRenderer getValueRenderer() {
        return renderer;
    }


    public TableCellEditor getValueEditor() {
        if (isRanged) {
            return new DefaultCellEditor(comboBox);
        } else {
            return new DefaultCellEditor(new JTextField());
        }
    }
}
