package edu.cmu.sphinx.tools.confdesigner.propedit;

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


    public void setValue(Object value) {
        myValue = value;
    }


    public Object getValue() {
        return myValue;
    }


    public String getDisplayName() {
        return myName;
    }


    public Color getFieldColor() {
        return Color.GREEN;
    }
}
