package edu.cmu.sphinx.tools.confdesigner;

import org.netbeans.api.visual.widget.general.IconNodeWidget;
import org.netbeans.api.visual.widget.Scene;
import org.netbeans.api.visual.model.ObjectState;
import org.netbeans.api.visual.laf.LookFeel;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class ConfNodeWidget extends IconNodeWidget {

    public ConfNodeWidget(Scene scene) {
        super(scene);
    }


    public ConfNodeWidget(Scene scene, TextOrientation orientation) {
        super(scene, orientation);
    }


    public void notifyStateChanged(ObjectState previousState, ObjectState state) {
        LookFeel lookFeel = getScene().getLookFeel();
        setBorder(lookFeel.getBorder(state));
        setForeground(lookFeel.getForeground(state));
    }


    public void addThisPin(PortWidget portWidget) {
        
        addChild(portWidget);
        
    }
}
