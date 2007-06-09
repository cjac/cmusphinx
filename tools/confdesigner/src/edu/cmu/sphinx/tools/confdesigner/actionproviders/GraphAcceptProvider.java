package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import edu.cmu.sphinx.tools.confdesigner.ConfigScene;
import edu.cmu.sphinx.tools.confdesigner.SceneController;
import edu.cmu.sphinx.util.props.Configurable;
import org.netbeans.api.visual.action.AcceptProvider;
import org.netbeans.api.visual.action.ConnectorState;
import org.netbeans.api.visual.widget.Widget;

import java.awt.*;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.geom.AffineTransform;
import java.io.IOException;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class GraphAcceptProvider implements AcceptProvider {

    private ConfigScene graphScene;
    private SceneController controller;


    public GraphAcceptProvider(ConfigScene graphScene) {
        assert graphScene != null;
        this.graphScene = graphScene;
    }


    public void setController(SceneController controller) {
        this.controller = controller;
    }


    public ConnectorState isAcceptable(Widget widget, Point point, Transferable transferable) {
        Graphics2D g2 = (Graphics2D) graphScene.getView().getGraphics();
        Rectangle visRect = graphScene.getView().getVisibleRect();
        graphScene.getView().paintImmediately(visRect.x, visRect.y, visRect.width, visRect.height);

        double zoomFactor = graphScene.getZoomFactor();
        g2.drawImage(ConfigScene.NODE_IMAGE, AffineTransform.getTranslateInstance(point.getLocation().getX()*zoomFactor, point.getLocation().getY()*zoomFactor), null);
//        g2.drawImage(ConfigScene.NODE_IMAGE, (int) point.getX(), (int) point.getY(), null);

        return ConnectorState.ACCEPT;
    }


    public void accept(final Widget widget, final Point point, final Transferable transferable) {
        String confName = controller.addNode(getDropClass(transferable), null);
        graphScene.findWidgetByName(confName).setPreferredLocation(widget.convertLocalToScene(point));
    }


    private Class<? extends Configurable> getDropClass(Transferable t) {
        Class<? extends Configurable> confClass = null;

        try {
            confClass = (Class<? extends Configurable>) t.getTransferData(new DataFlavor(Class.class, "Configurable"));
        } catch (UnsupportedFlavorException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return confClass;
    }
}
