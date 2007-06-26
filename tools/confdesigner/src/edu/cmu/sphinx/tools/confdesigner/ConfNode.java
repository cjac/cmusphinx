package edu.cmu.sphinx.tools.confdesigner;

import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.PropertySheet;

import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;

/** @author Holger Brandl */
public class ConfNode {

    private String instanceName;
    private PropertySheet propSheet;

    public static final String PARENT_PIN = "ConfigurableSource";

    private ConfPin thisPin;

    Map<String, ConfPin> propPins = new HashMap<String, ConfPin>();
    Map<String, List<ConfPin>> listPins = new HashMap<String, List<ConfPin>>();


    public ConfNode(String instanceName, PropertySheet ps) {
        this.instanceName = instanceName;
        this.propSheet = ps;
    }


    ConfPin addOutputPin(ConfigScene scene, Class<? extends Configurable> type) {
        thisPin = new ConfPin(ConfPin.THIS_NAME, type);

        //todo add the this pin using another node-layout
        scene.addPin(this, thisPin);

        return thisPin;
    }


    public ConfPin addInputPin(ConfigScene scene, String propName, Class<? extends Configurable> type) {
        return addInputPin(scene, propName, -1, type);
    }


    public ConfPin addInputPin(ConfigScene scene, String propName, int listPos, Class<? extends Configurable> type) {
        ConfPin propPin = new ConfPin(propName, type);
        if (listPos > 0)
            propPin.setListPosition(listPos);

        propPins.put(propPin.toString(), propPin);

        scene.addPin(this, propPin);

        if (propPin.isListPin()) {
            String propListName = propPin.getPropName();
            if (!listPins.containsKey(propListName))
                listPins.put(propListName, new ArrayList<ConfPin>());

            listPins.get(propListName).add(propPin);
        }

        return propPin;
    }


    public String getInstanceName() {
        return instanceName;
    }


    public void setInstanceName(String newName) {
        instanceName = newName;
    }


    public List<ConfPin> getListPins(String propListName) {
        return listPins.get(propListName);
    }


    public PropertySheet getPropSheet() {
        return propSheet;
    }


    public ConfPin getPin(String pinName) {
        if (pinName.equals(PARENT_PIN))
            return thisPin;

        assert propPins.containsKey(pinName);
        return propPins.get(pinName);
    }


    public Map<String, ConfPin> getPropertyPins() {
        return propPins;
    }


    public ConfPin getThisPin() {
        return thisPin;
    }


    public void revoveListPin(ConfPin pin) {
        assert pin.isListPin();
        getListPins(pin.getPropName()).remove(pin);
    }
}
