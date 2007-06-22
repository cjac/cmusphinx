package edu.cmu.sphinx.tools.confdesigner.conftree;

import edu.cmu.sphinx.util.props.Configurable;

import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;

/**
 * A component tree where the tree structure is defined by the package-structure of the class path.
 *
 * @author Holger Brandl
 */
public class PackageTree extends ConfigurableTree {

    public void rebuildTree() {
        setModel(new DefaultTreeModel(new DefaultMutableTreeNode()));
        categories.clear();

        //add all s4-configurables to the tree

        DefaultMutableTreeNode rootNode = (DefaultMutableTreeNode) treeModel.getRoot();
        rootNode.removeAllChildren();

        for (Class<? extends Configurable> configurable : filteredConfigClasses) {
            String category = configurable.getPackage().getName();
            if (!categories.containsKey(category)) {
                categories.put(category, new DefaultMutableTreeNode(category));
                rootNode.add(categories.get(category));
            }

            DefaultMutableTreeNode categoryNode = categories.get(category);
            categoryNode.add(new ConfigurableNode(configurable));
        }

        for (int i = 0; i < getRowCount(); i++)
            expandRow(i);
    }
}
