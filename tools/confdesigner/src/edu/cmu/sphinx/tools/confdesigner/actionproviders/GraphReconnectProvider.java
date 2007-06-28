package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import edu.cmu.sphinx.tools.confdesigner.*;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import org.netbeans.api.visual.action.ConnectorState;
import org.netbeans.api.visual.action.ReconnectProvider;
import org.netbeans.api.visual.widget.ConnectionWidget;
import org.netbeans.api.visual.widget.LabelWidget;
import org.netbeans.api.visual.widget.Scene;
import org.netbeans.api.visual.widget.Widget;

import java.awt.*;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class GraphReconnectProvider implements ReconnectProvider {

    ConfEdge edge;
    ConfPin originalNode;
    ConfPin replacementNode;
    private ConfigScene scene;


    public GraphReconnectProvider(ConfigScene scene) {
        this.scene = scene;
    }


    public void reconnectingStarted(ConnectionWidget connectionWidget, boolean reconnectingSource) {
    }


    public void reconnectingFinished(ConnectionWidget connectionWidget, boolean reconnectingSource) {
        System.out.println("");
    }


    public boolean isSourceReconnectable(ConnectionWidget connectionWidget) {
        Object object = scene.findObject(connectionWidget);
        edge = scene.isEdge(object) ? (ConfEdge) object : null;
        originalNode = edge != null ? scene.getEdgeSource(edge) : null;
        return originalNode != null;
    }


    public boolean isTargetReconnectable(ConnectionWidget connectionWidget) {
        Object object = scene.findObject(connectionWidget);
        edge = scene.isEdge(object) ? (ConfEdge) object : null;
        originalNode = edge != null ? scene.getEdgeTarget(edge) : null;
        return originalNode != null;
    }


    public ConnectorState isReplacementWidget(ConnectionWidget connectionWidget, Widget replacementWidget, boolean reconnectingSource) {
        Widget sourceWidget, targetWidget;
        if (reconnectingSource) {
            sourceWidget = replacementWidget;
            targetWidget = connectionWidget.getTargetAnchor().getRelatedWidget();

        } else {
            sourceWidget = connectionWidget.getSourceAnchor().getRelatedWidget();
            targetWidget = replacementWidget;
        }

        if (sourceWidget == targetWidget)
            return ConnectorState.ACCEPT;
//
        if (replacementWidget instanceof ConfNodeWidget || replacementWidget instanceof ConfigScene || replacementWidget instanceof LabelWidget)
            return ConnectorState.REJECT;

        Object object = scene.findObject(replacementWidget);
        replacementNode = scene.isPin(object) ? (ConfPin) object : null;
        if (replacementNode != null && sourceWidget instanceof PortWidget && targetWidget instanceof PortWidget)
            return ConnectUtils.isTargetWidget(scene, sourceWidget, targetWidget);

        return object != null ? ConnectorState.REJECT_AND_STOP : ConnectorState.REJECT;
    }


    public boolean hasCustomReplacementWidgetResolver(Scene scene) {
        return false;
    }


    public Widget resolveReplacementWidget(Scene scene, Point sceneLocation) {
        return null;
    }


    public void reconnect(ConnectionWidget connectionWidget, Widget replacementWidget, boolean reconnectingSource) {
        if (replacementWidget == null)
            scene.removeEdge(edge);
        else if (reconnectingSource) {
            System.out.println("replacing source widget; THIS IS NOT REFLECTED IN THE CM-MODEL YET");
            scene.setEdgeSource(edge, replacementNode);

        } else {
            ConfPin listPin = edge.getTarget();
            if (listPin.isListPin())
                return;

            // remove the old property
            PortWidget targetPort = (PortWidget) connectionWidget.getTargetAnchor().getRelatedWidget();
            PortWidget sourcePort = (PortWidget) connectionWidget.getSourceAnchor().getRelatedWidget();

            ConfNodeWidget targetNodeWidget = (ConfNodeWidget) targetPort.getParentWidget();
            ConfNodeWidget sourceNodeWidget = (ConfNodeWidget) sourcePort.getParentWidget();

            ConfNode targetNode = (ConfNode) scene.findObject(targetNodeWidget);
            ConfNode sourceNode = (ConfNode) scene.findObject(sourceNodeWidget);

            scene.setEdgeTarget(edge, replacementNode);
            ConfNode newTargetNode = (ConfNode) scene.findObject(replacementWidget.getParentWidget());
            String newPropName = ((ConfPin) scene.findObject((PortWidget) replacementWidget)).getPropName();
            PropertySheet newTargetPS = newTargetNode.getPropSheet();


            PropertySheet ps = targetNode.getPropSheet();

            try {
//                ps.setComponent(targetPort.getLabelWidget().getLabel(), null, null);
//
////                        Configurable conf = cm.lookup(sourceNode.getInstanceName());
//                listPin = edge.getTarget();
//                if (listPin.isListPin()) {
//                    java.util.List<String> compList = new ArrayList<String>();
//                    for (ConfPin pin : targetNode.getListPins(listPin.getPropName())) {
//                        Collection<ConfEdge> confEdges = scene.findPinEdges(pin, false, true);
//                        if (!confEdges.isEmpty()) {
//                            compList.add(scene.getPinNode(confEdges.iterator().next().getSource()).getInstanceName());
//                        }
//                    }
//
//                    newTargetPS.setComponentList(newPropName, compList, null);
//                } else {
                newTargetPS.setComponent(newPropName, sourceNode.getInstanceName(), null);
//                }

            } catch (PropertyException e) {
                e.printStackTrace();
            }


        }
    }
}
