package edu.cmu.sphinx.tools.confdesigner.propedit;

import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.S4Boolean;

import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class TableBoolProperty extends TableProperty {

    private PropertySheet currentPS;
    private String propName;
    private S4Boolean s4Boolean;


    public TableBoolProperty(PropertySheet currentPS, String propName, S4Boolean s4Boolean) {
        super();
        this.currentPS = currentPS;
        this.propName = propName;
        this.s4Boolean = s4Boolean;
    }


    public void setValue(Object value) {
        assert value instanceof Double;

        Double newValue = (Double) value;
//        setValue();
        // range checking is automatically done by the attached cell editor
        currentPS.setDouble(propName, newValue);
    }


    public TableCellRenderer getNameRenderer() {
        return new DefaultTableCellRenderer();
    }


    public TableCellRenderer getValueRenderer() {
        return new DefaultTableCellRenderer();
    }


    public TableCellEditor getValueEditor() {
        return new IntegerEditor(0, 1);
    }
}
