package edu.cmu.sphinx.tools.confdesigner;

import org.netbeans.api.visual.model.ObjectSceneEvent;

import java.util.Set;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
class ConfigSceneListener extends ObjectSceneListenerAdapter {

    private ConfDesigner designer;
    private SceneController sceneController;


    public ConfigSceneListener(ConfDesigner designer) {
        this.designer = designer;
    }


    public void selectionChanged(ObjectSceneEvent event, Set<Object> previousSelection, Set<Object> newSelection) {
        PropertyEditorPanel propPanel = designer.getPropSheetPanel();

        if (newSelection.size() != 1) {
            propPanel.rebuildPanel(null);
        } else if (!newSelection.isEmpty()) {
            Object o = newSelection.iterator().next();
            if (o instanceof ConfNode) {
                propPanel.rebuildPanel(((ConfNode) o).getPropSheet());
            }

            sceneController.getScene().getView().requestFocusInWindow();
        }
    }


    public void setController(SceneController controller) {
        sceneController = controller;
    }
}
