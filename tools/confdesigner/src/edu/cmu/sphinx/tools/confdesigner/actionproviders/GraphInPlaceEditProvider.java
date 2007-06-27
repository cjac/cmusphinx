package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import edu.cmu.sphinx.tools.confdesigner.ConfNode;
import edu.cmu.sphinx.tools.confdesigner.ConfigScene;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import org.netbeans.api.visual.action.TextFieldInplaceEditor;
import org.netbeans.api.visual.widget.LabelWidget;
import org.netbeans.api.visual.widget.Widget;

/**
 * DOCUMENT ME !
 *
 * @author Holger Brandl
 */
public class GraphInPlaceEditProvider implements TextFieldInplaceEditor {

    private ConfigurationManager cm;
    private ConfigScene scene;


    public boolean isEnabled(Widget widget) {
        return true;
    }


    public String getText(Widget widget) {
        return ((LabelWidget) widget).getLabel();
    }


    public void setText(Widget widget, String newName) {
        String oldName = ((LabelWidget) widget).getLabel();
        if (!oldName.equals(newName)) {
            ((LabelWidget) widget).setLabel(newName);

            // make sure that the renamed widget is not a background widget
            if (scene.findObject(widget.getParentWidget()) != null) {
                cm.renameConfigurable(oldName, newName);
                ConfNode node = (ConfNode) scene.findObject(widget.getParentWidget());
                node.setInstanceName(newName);
            }
        }
    }


    public void setCM(ConfigurationManager cm) {
        assert cm != null;
        this.cm = cm;
    }


    public void setScene(ConfigScene configScene) {
        scene = configScene;
    }
}
