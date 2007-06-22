package edu.cmu.sphinx.tools.confdesigner.conftree;

import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;

/**
 * Not implemented yet. Later on we'll implement this tree in order to show up all available virtual modules
 *
 * @author Holger Brandl
 */
public class VirtModuleTree extends ConfigurableTree {

    public void rebuildTree() {
        setModel(new DefaultTreeModel(new DefaultMutableTreeNode()));
        categories.clear();
    }

}
