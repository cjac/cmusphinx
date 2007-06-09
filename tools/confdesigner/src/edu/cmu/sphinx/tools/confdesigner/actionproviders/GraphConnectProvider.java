package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import edu.cmu.sphinx.tools.confdesigner.ConfEdge;
import edu.cmu.sphinx.tools.confdesigner.ConfPin;
import edu.cmu.sphinx.tools.confdesigner.ConfigScene;
import edu.cmu.sphinx.tools.confdesigner.ConfNode;
import edu.cmu.sphinx.util.props.*;
import org.netbeans.api.visual.action.ConnectProvider;
import org.netbeans.api.visual.action.ConnectorState;
import org.netbeans.api.visual.widget.Scene;
import org.netbeans.api.visual.widget.Widget;

import java.awt.*;
import java.util.*;

/**
 * DOCUMENT ME !
 *
 * @author Holger Brandl
 */
public class GraphConnectProvider implements ConnectProvider {

    private ConfigScene scene;
    private ConfigurationManager cm;


    public GraphConnectProvider(ConfigScene scene) {
        this.scene = scene;
    }


    public boolean isSourceWidget(Widget sourceWidget) {
        Object object = scene.findObject(sourceWidget);

        if (!(object instanceof ConfPin))
            return false;

        ConfPin pin = (ConfPin) object;

        assert scene.isPin(object);
        return pin.getPropName().equals(ConfPin.THIS_NAME);
    }


    public ConnectorState isTargetWidget(Widget sourceWidget, Widget targetWidget) {
        Object object = scene.findObject(targetWidget);

        if (!(object instanceof ConfPin))
            return ConnectorState.REJECT_AND_STOP;

        ConfPin sourcePin = (ConfPin) scene.findObject(sourceWidget);
        ConfPin targetPin = (ConfPin) scene.findObject(targetWidget);

        if (!sourcePin.getPropName().equals(ConfPin.THIS_NAME))
            return ConnectorState.REJECT;

        Class<? extends Configurable> sourceType = sourcePin.getType();
        Class<? extends Configurable> targetType = targetPin.getType();

        if (ConfigurationManagerUtils.isDerivedClass(sourceType, targetType)) {
            Collection<ConfEdge> allEdges = scene.getEdges();
            for (ConfEdge edge : allEdges) {
                if (scene.getEdgeTarget(edge).equals(targetPin))
                    return ConnectorState.REJECT;
            }

            return ConnectorState.ACCEPT;
        }

//        System.out.println("sourcet " + sourceType + " targett " + targetType);

        return ConnectorState.REJECT;
    }


    public boolean hasCustomTargetWidgetResolver(Scene scene) {
        return false;
    }


    public Widget resolveTargetWidget(Scene scene, Point sceneLocation) {
        return null;
    }


    public void createConnection(Widget sourceWidget, Widget targetWidget) {
        ConfPin sourcePin = (ConfPin) scene.findObject(sourceWidget);
        ConfPin targetPin = (ConfPin) scene.findObject(targetWidget);
        ConfEdge edge = new ConfEdge(sourcePin, targetPin);

        scene.createEdge(edge);

        // now commit the changes to the configuration manager
        ConfNode targetNode = scene.getPinNode(edge.getTarget());
        ConfNode sourceNode = scene.getPinNode(edge.getSource());
        PropertySheet ps = cm.getPropertySheet(targetNode.getInstanceName());
        try {
//                        Configurable conf = cm.lookup(sourceNode.getInstanceName());
            ConfPin listPin = edge.getTarget();
            if (listPin.isListPin()) {
                java.util.List<String> compList = new ArrayList<String>();
                for (ConfPin pin : targetNode.getListPins(listPin.getPropName())) {
                    Collection<ConfEdge> confEdges = scene.findPinEdges(pin, false, true);
                    if (!confEdges.isEmpty()){
                        compList.add(scene.getPinNode(confEdges.iterator().next().getSource()).getInstanceName());
                    }
                }

                ps.setComponentList(edge.getTarget().getPropName(), compList, null);
            } else {
                ps.setComponent(edge.getTarget().getPropName(), sourceNode.getInstanceName(), null);
            }
        } catch (PropertyException e) {
            e.printStackTrace();
        }
    }


    public void setCM(ConfigurationManager cm) {
        assert cm != null;
        this.cm = cm;
    }
}
