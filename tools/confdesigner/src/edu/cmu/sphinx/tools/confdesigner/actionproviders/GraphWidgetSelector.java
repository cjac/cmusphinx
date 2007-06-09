package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import org.netbeans.api.visual.action.SelectProvider;
import org.netbeans.api.visual.model.ObjectScene;
import org.netbeans.api.visual.widget.Widget;

import java.awt.*;
import java.util.Collections;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class GraphWidgetSelector implements SelectProvider {

    private ObjectScene scene;


    public GraphWidgetSelector(ObjectScene scene) {
        this.scene = scene;
    }


    public boolean isAimingAllowed(Widget widget, Point point, boolean b) {
        return false;
    }


    public boolean isSelectionAllowed(Widget widget, Point point, boolean b) {
        return true;
    }


    public void select(Widget widget, Point point, boolean invertSelection) {
        Object object = scene.findObject(widget);

        if (object != null) {
            if (scene.getSelectedObjects().contains(object))
                return;

            scene.userSelectionSuggested(Collections.singleton(object), invertSelection);
        } else
            scene.userSelectionSuggested(Collections.emptySet(), invertSelection);
    }
}
