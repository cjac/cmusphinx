package edu.cmu.sphinx.tools.confdesigner.util;

import edu.cmu.sphinx.tools.confdesigner.SceneController;
import edu.cmu.sphinx.tools.confdesigner.ConfNode;

import java.util.Collection;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
import java.awt.*;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class SceneFinder {

    private Frame parent;
    private SceneController sceneController;



    public SceneFinder(Frame parent, SceneController sceneController) {
        this.parent = parent;
        this.sceneController = sceneController;

    }


    public boolean process(String searchText) {
        List<ConfNode> matches = new ArrayList<ConfNode>();
        Collection<ConfNode> nodes = sceneController.getScene().getNodes();

        searchText = searchText.toLowerCase();
        
        for (ConfNode node : nodes) {
            if(node.getInstanceName().toLowerCase().contains(searchText))
                matches.add(node);
        }

        ConfNode selectedNode = null;

        // several nodes were found ask the user which one should be shown
        if(matches.size() > 1){
            FinderSelectionPanel finderPanel = new FinderSelectionPanel(parent, matches);
            finderPanel.setModal(true);
            finderPanel.setVisible(true);

            selectedNode = finderPanel.getSelectedNode();
        }else if (!matches.isEmpty())
            selectedNode = matches.get(0);


        if(selectedNode != null)
        sceneController.getScene().userSelectionSuggested(Collections.singleton(selectedNode), false);

        return selectedNode != null;
    }
}
