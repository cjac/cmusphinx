package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import org.netbeans.api.visual.action.TextFieldInplaceEditor;
import org.netbeans.api.visual.widget.LabelWidget;
import org.netbeans.api.visual.widget.Widget;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.ConfigurationManagerUtils;

/**
 * DOCUMENT ME !
 *
 * @author Holger Brandl
 */
public class GraphInPlaceEditProvider implements TextFieldInplaceEditor {

    private ConfigurationManager cm;


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

            cm.renameConfigurable(oldName, newName);
        }
    }


    public void setCM(ConfigurationManager cm) {
        assert cm != null;
        this.cm = cm;
    }
}
