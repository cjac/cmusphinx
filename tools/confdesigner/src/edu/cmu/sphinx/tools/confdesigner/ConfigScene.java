/*
 * The contents of this file are subject to the terms of the Common Development
 * and Distribution License (the License). You may not use this file except in
 * compliance with the License.
 *
 * You can obtain a copy of the License at http://www.netbeans.org/cddl.html
 * or http://www.netbeans.org/cddl.txt.
 *
 * When distributing Covered Code, include this CDDL Header Notice in each file
 * and include the License file at http://www.netbeans.org/cddl.txt.
 * If applicable, add the following below the CDDL Header, with the fields
 * enclosed by brackets [] replaced by your own identifying information:
 * "Portions Copyrighted [year] [name of copyright owner]"
 *
 * The Original Software is NetBeans. The Initial Developer of the Original
 * Software is Sun Microsystems, Inc. Portions Copyright 1997-2006 Sun
 * Microsystems, Inc. All Rights Reserved.
 */
package edu.cmu.sphinx.tools.confdesigner;

import edu.cmu.sphinx.tools.confdesigner.actionproviders.*;
import org.netbeans.api.visual.action.ActionFactory;
import org.netbeans.api.visual.action.PopupMenuProvider;
import org.netbeans.api.visual.action.WidgetAction;
import org.netbeans.api.visual.anchor.AnchorFactory;
import org.netbeans.api.visual.anchor.AnchorShape;
import org.netbeans.api.visual.anchor.PointShape;
import org.netbeans.api.visual.graph.GraphPinScene;
import org.netbeans.api.visual.graph.layout.GridGraphLayout;
import org.netbeans.api.visual.layout.LayoutFactory;
import org.netbeans.api.visual.layout.SceneLayout;
import org.netbeans.api.visual.model.ObjectSceneEvent;
import org.netbeans.api.visual.model.ObjectSceneEventType;
import org.netbeans.api.visual.model.ObjectState;
import org.netbeans.api.visual.router.Router;
import org.netbeans.api.visual.router.RouterFactory;
import org.netbeans.api.visual.widget.ConnectionWidget;
import org.netbeans.api.visual.widget.LayerWidget;
import org.netbeans.api.visual.widget.Widget;
import org.openide.util.Utilities;

import javax.swing.*;
import java.awt.*;

/** @author David Kaspar */
public class ConfigScene extends GraphPinScene<ConfNode, ConfEdge, ConfPin> {

    private LayerWidget mainLayer = new LayerWidget(this);
    private LayerWidget connectionLayer = new LayerWidget(this);
    private LayerWidget interractionLayer = new LayerWidget(this);
    private LayerWidget backgroundLayer = new LayerWidget(this);

    GraphConnectProvider connectProvider = new GraphConnectProvider(this);
    GraphAcceptProvider acceptProvider = new GraphAcceptProvider(this);
    GraphInPlaceEditProvider inplaceProvider = new GraphInPlaceEditProvider();

    private WidgetAction moveAction = ActionFactory.createMoveAction();
    private WidgetAction popupMenuAction = ActionFactory.createPopupMenuAction(new MyPopupMenuProvider());
    private WidgetAction connectAction = ActionFactory.createExtendedConnectAction(interractionLayer, connectProvider);
    private WidgetAction reconnectAction = ActionFactory.createReconnectAction(new GraphReconnectProvider(this));
    private WidgetAction moveControlPointAction = ActionFactory.createFreeMoveControlPointAction();
    private WidgetAction selectAction = ActionFactory.createSelectAction(new GraphWidgetSelector(this));
    private WidgetAction inplaceEditAction = ActionFactory.createInplaceEditorAction(inplaceProvider);
    private WidgetAction acceptAction = ActionFactory.createAcceptAction(acceptProvider);

    public static final Image NODE_IMAGE = Utilities.loadImage("test/resources/custom_displayable_32.png");
    //    private Router router = RouterFactory.createOrthogonalSearchRouter(mainLayer, connectionLayer, interractionLayer);
    private Router router = RouterFactory.createOrthogonalSearchRouter(mainLayer);

    private SceneLayout sceneLayout;


    public ConfigScene() {
        addChild(backgroundLayer);
        addChild(mainLayer);
        addChild(connectionLayer);
        addChild(interractionLayer);


        getActions().addAction(ActionFactory.createRectangularSelectAction(this, backgroundLayer));
        getActions().addAction(ActionFactory.createPanAction());
        getActions().addAction(ActionFactory.createZoomAction());
        getActions().addAction(ActionFactory.createPopupMenuAction(new GraphSceneMenu(this)));
        ConfigSceneUtils.initGrids(this);

        getActions().addAction(acceptAction);

        sceneLayout = LayoutFactory.createSceneGraphLayout(this, new GridGraphLayout<ConfNode, ConfEdge>());

        addObjectSceneListener(new ObjectSceneListenerAdapter() {

            public void objectStateChanged(ObjectSceneEvent event, Object changedObject, ObjectState previousState, ObjectState newState) {
                super.objectStateChanged(event, changedObject, previousState, newState);
            }
        }, ObjectSceneEventType.OBJECT_STATE_CHANGED);
    }


    protected Widget attachNodeWidget(ConfNode node) {
        ConfNodeWidget widget = new ConfNodeWidget(this);

        widget.setLayout(LayoutFactory.createVerticalFlowLayout(LayoutFactory.SerialAlignment.CENTER, 4));
        widget.setImage(NODE_IMAGE);
        widget.setLabel(node.getInstanceName());

        widget.setToolTipText("Class: " + node.getPropSheet().getConfigurableClass().getName());

        widget.getLabelWidget().getActions().addAction(inplaceEditAction);

        widget.getActions().addAction(selectAction);
        widget.getActions().addAction(moveAction);
        widget.getActions().addAction(createObjectHoverAction());
        widget.getActions().addAction(popupMenuAction);
//        widget.getActions().addAction(connectAction);


        mainLayer.addChild(widget);

        return widget;
    }


    protected Widget attachPinWidget(ConfNode node, ConfPin pin) {
        PortWidget portWidget = new PortWidget(this, pin.toString());

        ConfNodeWidget nodeWidget = ((ConfNodeWidget) findWidget(node));

        if (pin.getPropName().equals(ConfPin.THIS_NAME)) {
            nodeWidget.addThisPin(portWidget);
            portWidget.getActions().addAction(createObjectHoverAction());

        } else {
            nodeWidget.addChild(portWidget); // todo distinguish between properties and the this port here
        }

//        portWidget.getActions().addAction(createObjectHoverAction());
//        portWidget.getActions().addAction(selectAction);
        portWidget.getActions().addAction(connectAction);

        portWidget.getImageWidget().getActions().addAction(selectAction);
        portWidget.getImageWidget().getActions().addAction(connectAction);
        return portWidget;
    }


    protected Widget attachEdgeWidget(ConfEdge edge) {
        ConnectionWidget c = new ConnectionWidget(this);
        c.setRouter(router);

        c.setToolTipText("Double-click for Add/Remove Control Point");
        c.setTargetAnchorShape(AnchorShape.TRIANGLE_FILLED);
        c.setControlPointShape(PointShape.SQUARE_FILLED_BIG);
        c.setEndPointShape(PointShape.SQUARE_FILLED_BIG);
        connectionLayer.addChild(c);

        c.getActions().addAction(reconnectAction);
        c.getActions().addAction(createSelectAction());
        c.getActions().addAction(ActionFactory.createAddRemoveControlPointAction());
        c.getActions().addAction(moveControlPointAction);
//        c.getActions().addAction(ActionFactory.createPopupMenuAction(edgeMenu));

        return c;
    }


    protected void attachEdgeSourceAnchor(ConfEdge edge, ConfPin oldSourcePin, ConfPin sourcePin) {
        ConnectionWidget widget = (ConnectionWidget) findWidget(edge);
        Widget sourceNodeWidget = findWidget(sourcePin);
        widget.setSourceAnchor(sourceNodeWidget != null ? AnchorFactory.createDirectionalAnchor(sourceNodeWidget, AnchorFactory.DirectionalAnchorKind.HORIZONTAL) : null);
    }


    protected void attachEdgeTargetAnchor(ConfEdge edge, ConfPin oldTargetPin, ConfPin targetPin) {
        ConnectionWidget widget = (ConnectionWidget) findWidget(edge);
        Widget targetNodeWidget = findWidget(targetPin);
//        widget.setTargetAnchor(targetNodeWidget != null ? AnchorFactory.createRectangularAnchor(targetNodeWidget) : null);
        widget.setTargetAnchor(targetNodeWidget != null ? AnchorFactory.createDirectionalAnchor(targetNodeWidget, AnchorFactory.DirectionalAnchorKind.HORIZONTAL) : null);
    }


    /** Adds and connects an edge to the current graph. */
    public void createEdge(ConfEdge edge) {
        addEdge(edge);
        setEdgeSource(edge, edge.getSource());
        setEdgeTarget(edge, edge.getTarget());
    }


    /** Invokes layout of the scene. */
    public void layoutScene() {
        sceneLayout.invokeLayout();
    }


    public Widget findWidgetByName(String compName) {
        ConfNode node = findNodeByName(compName);
        return node != null ? findWidget(node) : null;
    }


    public ConfNode findNodeByName(String instanceName) {
        assert instanceName != null;
        for (ConfNode node : getNodes()) {
            if (node.getInstanceName().equals(instanceName))
                return node;
        }

        return null;
    }


    private static class MyPopupMenuProvider implements PopupMenuProvider {

        public JPopupMenu getPopupMenu(Widget widget, Point localLocation) {
            JPopupMenu popupMenu = new JPopupMenu();
//            popupMenu.add(new JMenuItem("edit component"));
            return popupMenu;
        }
    }
}
