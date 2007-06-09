package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import edu.cmu.sphinx.tools.confdesigner.ConfEdge;
import edu.cmu.sphinx.tools.confdesigner.ConfPin;
import edu.cmu.sphinx.tools.confdesigner.ConfigScene;
import org.netbeans.api.visual.action.ConnectorState;
import org.netbeans.api.visual.action.ReconnectProvider;
import org.netbeans.api.visual.widget.ConnectionWidget;
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
        System.out.println("tt");
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
        Object object = scene.findObject(replacementWidget);
        replacementNode = scene.isPin(object) ? (ConfPin) object : null;
        if (replacementNode != null)
            return ConnectorState.ACCEPT;
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
        else if (reconnectingSource)
            scene.setEdgeSource(edge, replacementNode);
        else
            scene.setEdgeTarget(edge, replacementNode);
    }
}
