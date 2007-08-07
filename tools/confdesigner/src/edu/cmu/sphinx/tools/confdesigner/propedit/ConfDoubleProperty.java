package edu.cmu.sphinx.tools.confdesigner.propedit;

import com.l2fprod.common.propertysheet.DefaultProperty;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.S4Double;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class ConfDoubleProperty extends DefaultProperty {

    private PropertySheet currentPS;
    private String propName;
    private S4Double s4Double;


    public ConfDoubleProperty(PropertySheet currentPS, String propName, S4Double s4Double) {
        this.currentPS = currentPS;
        this.propName = propName;
        this.s4Double = s4Double;

        setDisplayName(propName);

        setType(Double.class);
        if (currentPS.getRaw(propName) != null)
            setValue(currentPS.getDouble(propName));
        setEditable(true);
    }


    public ConfDoubleProperty(String propName, S4Double s4Double) {
    }
}
