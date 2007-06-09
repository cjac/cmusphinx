package edu.cmu.sphinx.tools.confdesigner;

import edu.cmu.sphinx.util.props.Configurable;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class ConfPin {

    private String propName;
    private Class<? extends Configurable> type;

    public static final String THIS_NAME = "this";
    private int listPosition = -1;


    public ConfPin(String propName, Class<? extends Configurable> type) {
        this.propName = propName;
        this.type = type;
    }


    public Class<? extends Configurable> getType() {
        return type;
    }


    public String getPropName() {
        return propName;
    }


    public String toString() {
        if (listPosition > 0)
            return propName + listPosition;
        else
            return propName;
    }


    public boolean isListPin() {
        return listPosition >= 0;
    }


    public void setListPosition(int listPosition) {
        this.listPosition = listPosition;
    }


    public int getListPosition() {
        return listPosition;
    }
}

