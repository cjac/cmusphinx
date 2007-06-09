package edu.cmu.sphinx.tools.confdesigner.actionproviders;

import org.netbeans.api.visual.action.TextFieldInplaceEditor;
import org.netbeans.api.visual.widget.LabelWidget;
import org.netbeans.api.visual.widget.Widget;

/**
 * DOCUMENT ME !
 *
 * @author Holger Brandl
 */
public class GraphInPlaceEditProvider implements TextFieldInplaceEditor {

    public boolean isEnabled(Widget widget) {
        return true;
    }


    public String getText(Widget widget) {
        return ((LabelWidget) widget).getLabel();
    }


    public void setText(Widget widget, String text) {
        ((LabelWidget) widget).setLabel(text);
    }


}
