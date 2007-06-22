package edu.cmu.sphinx.tools.confdesigner.conftree;

import edu.cmu.sphinx.tools.confdesigner.SceneController;
import edu.cmu.sphinx.util.props.Configurable;

import javax.swing.*;
import java.awt.*;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class ConfigSelector extends JPanel {

    private List<ConfigurableTree> treeList = new ArrayList<ConfigurableTree>();


    public ConfigSelector() {
        setLayout(new BorderLayout());
        JTabbedPane tabPane = new JTabbedPane();

        ConfigurableTree packageTree = new PackageTree();
        ConfigurableTree categoryTree = new CategoryTree();
        ConfigurableTree virtModuleTree = new PackageTree();

        treeList.add(packageTree);
        treeList.add(categoryTree);
        treeList.add(virtModuleTree);

        tabPane.addTab("package", packageTree);
        tabPane.addTab("category", categoryTree);
        tabPane.addTab("virtual", virtModuleTree);

        add(tabPane);
    }


    public void setController(SceneController controller) {
        for (ConfigurableTree tree : treeList) {
            tree.setController(controller);
        }
    }


    public void addConfigurables(Collection<Class<? extends Configurable>> configClasses) {
        for (ConfigurableTree configurableTree : treeList) {
            configurableTree.addConfigurables(configClasses);
        }
    }


    public void setFilter(String filterText) {
        for (ConfigurableTree configurableTree : treeList) {
            configurableTree.setFilter(filterText);
        }
    }
}
