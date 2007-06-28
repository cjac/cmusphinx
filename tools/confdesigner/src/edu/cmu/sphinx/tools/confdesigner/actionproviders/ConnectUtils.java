package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import edu.cmu.sphinx.tools.confdesigner.ConfEdge;
import edu.cmu.sphinx.tools.confdesigner.ConfPin;
import edu.cmu.sphinx.tools.confdesigner.ConfigScene;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManagerUtils;
import org.netbeans.api.visual.action.ConnectorState;
import org.netbeans.api.visual.widget.Widget;

import java.util.Collection;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class ConnectUtils {

    public static ConnectorState isTargetWidget(ConfigScene scene, Widget sourceWidget, Widget targetWidget) {
        Object object = scene.findObject(targetWidget);

        if (!(object instanceof ConfPin))
            return ConnectorState.REJECT_AND_STOP;

        ConfPin sourcePin = (ConfPin) scene.findObject(sourceWidget);
        ConfPin targetPin = (ConfPin) scene.findObject(targetWidget);

        if (!sourcePin.getPropName().equals(ConfPin.THIS_NAME) && targetPin.getPropName().equals(ConfPin.THIS_NAME))
            return ConnectorState.REJECT;

        Class<? extends Configurable> sourceType = sourcePin.getType();
        Class<? extends Configurable> targetType = targetPin.getType();

        if (ConfigurationManagerUtils.isDerivedClass(sourceType, targetType)) {
            Collection<ConfEdge> allEdges = scene.getEdges();
            for (ConfEdge edge : allEdges) {
                if (scene.getEdgeTarget(edge).equals(targetPin) && scene.getEdgeTarget(edge) != targetPin) {
                    System.out.println("edge " + edge);
                    return ConnectorState.REJECT;
                }
            }

            return ConnectorState.ACCEPT;
        }

//        System.out.println("sourcet " + sourceType + " targett " + targetType);

        return ConnectorState.REJECT;
    }
}
