package edu.cmu.sphinx.tools.confdesigner.propedit;

import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.S4Component;

import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class TableCompProperty extends TableProperty {

    private PropertySheet currentPS;
    private String propName;
    private S4Component s4Double;


    public TableCompProperty(PropertySheet currentPS, String propName, S4Component s4String) {
        this.currentPS = currentPS;
        this.propName = propName;
        this.s4Double = s4String;

        setDisplayName(propName);

        if (currentPS.getRaw(propName) != null) {
            setValue(currentPS.getDouble(propName));
        } else {
            Class<? extends Configurable> defValue = s4String.type();

            // set color to gray to indicate the defaultness
        }
    }


    public void setValue(Object value) {
    }


    public TableCellRenderer getNameRenderer() {
        return null;
    }


    public TableCellRenderer getValueRenderer() {
        return null;
    }


    public TableCellEditor getValueEditor() {
        return null;
    }
}
