package edu.cmu.sphinx.tools.confdesigner.conftree;

import edu.cmu.sphinx.util.props.*;
import edu.cmu.sphinx.util.props.Configurable;

import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import java.util.*;

/**
 * A component tree where the tree structure is defined by user-defined category annoations. Because the
 * <code>ConfCategory</code> allows multiple tagging, components might occur in several functional categories.
 *
 * @author Holger Brandl
 * @see ConfCategory
 */
public class CategoryTree extends ConfigurableTree {

    public void rebuildTree() {
        setModel(new DefaultTreeModel(new DefaultMutableTreeNode()));
        categories.clear();

        //add all s4-configurables to the tree

        DefaultMutableTreeNode rootNode = (DefaultMutableTreeNode) treeModel.getRoot();
        rootNode.removeAllChildren();

        List<Class<? extends Configurable>> untaggedClasses = new ArrayList<Class<? extends Configurable>>();

        for (Class<? extends Configurable> confClass : filteredConfigClasses) {

            Set<String> classCategories = collectCategories(confClass);
            if (classCategories.isEmpty()) {
                untaggedClasses.add(confClass);
                continue;
            }

            for (String category : classCategories) {
                if (!categories.containsKey(category)) {
                    categories.put(category, new DefaultMutableTreeNode(category));
                    rootNode.add(categories.get(category));
                }

                DefaultMutableTreeNode categoryNode = categories.get(category);
                categoryNode.add(new ConfigurableNode(confClass));
            }
        }

        // add all untagged classes

        DefaultMutableTreeNode untagNode = new DefaultMutableTreeNode("untagged");
        rootNode.add(untagNode);
        for (Class<? extends Configurable> untaggedClass : untaggedClasses)
            untagNode.add(new ConfigurableNode(untaggedClass));

//        for (int i = 0; i < getRowCount(); i++)
        expandRow(0);
    }


    /**
     * Extract the ui-tags of a given classes, its parent-classes and all its implementing interfaces.
     * <p/>
     * Basically it iterates over all parent classes of the given <code>confClass</code> and all their implementing
     * interfaces in order to collect the category tags
     */
    public static Set<String> collectCategories(Class confClass) {
        Set<String> catNames = new HashSet<String>();

        ConfCategory cateogry = (ConfCategory) confClass.getAnnotation(ConfCategory.class);
        if (cateogry != null)
            catNames.addAll(Arrays.asList(cateogry.value()));

        Class<?> superClass = confClass.getSuperclass();
        if (superClass != null) {
            catNames.addAll(collectCategories(superClass));
        }

        for (Class curInterface : confClass.getInterfaces()) {
            cateogry = (ConfCategory) curInterface.getAnnotation(ConfCategory.class);
            if (cateogry != null)
                catNames.addAll(Arrays.asList(cateogry.value()));

            catNames.addAll(collectCategories(curInterface));
        }

        return catNames;
    }
}
