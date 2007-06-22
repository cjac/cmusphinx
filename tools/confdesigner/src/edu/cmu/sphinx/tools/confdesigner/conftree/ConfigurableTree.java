package edu.cmu.sphinx.tools.confdesigner.conftree;

import edu.cmu.sphinx.tools.confdesigner.SceneController;
import edu.cmu.sphinx.util.props.Configurable;
import org.netbeans.api.visual.widget.Widget;

import javax.swing.*;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreePath;
import java.awt.*;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public abstract class ConfigurableTree extends JTree {

    private SceneController sceneController;

    protected Map<String, DefaultMutableTreeNode> categories = new HashMap<String, DefaultMutableTreeNode>();
    protected Collection<Class<? extends Configurable>> filteredConfigClasses = new HashSet<Class<? extends Configurable>>();
    protected Collection<Class<? extends Configurable>> allConfigClasses = new HashSet<Class<? extends Configurable>>();


    public ConfigurableTree() {
        setTransferHandler(new ConfigTransferHandler());
//        TransferHandler transferHandler = getTransferHandler();
        setDragEnabled(true);
        setEnabled(true);


        addMouseListener(new MouseAdapter() {

            public void mouseReleased(MouseEvent e) {

                if (e.getClickCount() != 2)
                    return;

                Point pt = e.getPoint();
                TreePath pathForLocation = getPathForLocation((int) pt.getX(), (int) pt.getY());

                if (pathForLocation != null) {
                    Object selectedNode = pathForLocation.getLastPathComponent();
                    if (!(selectedNode instanceof ConfigurableNode))
                        return;

                    ConfigurableNode configurableNode = (ConfigurableNode) selectedNode;

                    Class<? extends Configurable> confClass = configurableNode.getConfigurableClass();
                    setSelectionPath(pathForLocation);

                    if (sceneController != null) {
                        String confName = sceneController.addNode(confClass, null);

                        Rectangle visRect = sceneController.getScene().getView().getVisibleRect();
                        Widget w = sceneController.getScene().findWidgetByName(confName);
                        w.setPreferredLocation(new Point((int) visRect.getCenterX(), (int) visRect.getCenterY()));
                        sceneController.getScene().validate();

                    }
                }
            }


            public void mousePressed(MouseEvent e) {
                super.mousePressed(e);

                if (e.getClickCount() == 2) {
//                    mouseReleased(e);
                } else {
                    JComponent c = (JComponent) e.getSource();
                    TransferHandler handler = c.getTransferHandler();
                    handler.exportAsDrag(c, e, TransferHandler.COPY);
                }
            }
        });
    }


    /** Rebuild the panel. This methods needs to be overridden in order to allow a user-defined tree-creation. */
    abstract public void rebuildTree();


    public synchronized void addConfigurables(Collection<Class<? extends Configurable>> configs) {
        allConfigClasses.addAll(configs);
        filteredConfigClasses.addAll(allConfigClasses);

        rebuildTree();
    }


    public void setFilter(String filterText) {
        filteredConfigClasses.clear();

        if (filterText == null)
            filteredConfigClasses.addAll(allConfigClasses);
        else {
            for (Class<? extends Configurable> aClass : allConfigClasses) {
                if (doPassFilter(filterText.toLowerCase(), aClass))
                    filteredConfigClasses.add(aClass);
            }
        }

        rebuildTree();
    }


    private boolean doPassFilter(String filterText, Class aClass) {
        if (aClass == null)
            return false;

        if (aClass.getName().toLowerCase().contains(filterText))
            return true;

        if (doPassFilter(filterText, aClass.getSuperclass()))
            return true;

        for (Class aInterface : aClass.getInterfaces()) {
            if (doPassFilter(filterText, aInterface))
                return true;
            else if (doPassFilter(filterText, aInterface.getSuperclass()))
                return true;
        }

        return false;
    }


    public void setController(SceneController sceneController) {
        this.sceneController = sceneController;

        rebuildTree();
    }
}

class TransferableConfigurable implements Transferable {

    private Class<? extends Configurable> confClass;
    public static DataFlavor dataFlavor = new DataFlavor(Class.class, "Configurable");


    public TransferableConfigurable(Class<? extends Configurable> confClass) {
        this.confClass = confClass;
    }


    public DataFlavor[] getTransferDataFlavors() {
        return new DataFlavor[]{dataFlavor};
    }


    public boolean isDataFlavorSupported(DataFlavor flavor) {
        return flavor.equals(dataFlavor);
    }


    public Object getTransferData(DataFlavor flavor) throws UnsupportedFlavorException, IOException {
        if (isDataFlavorSupported(flavor))
            return confClass;
        else
            return null;
    }
}

class ConfigurableNode extends DefaultMutableTreeNode {

    private Class<? extends Configurable> configurableClass;
    private String name;


    public ConfigurableNode(Class<? extends Configurable> configurableClass) {
        this.configurableClass = configurableClass;

        String[] strings = configurableClass.getName().split("[.]");
        name = strings[strings.length - 1];
    }


    public String toString() {
        return name;
    }


    public Class<? extends Configurable> getConfigurableClass() {
        return configurableClass;
    }


    public String getConfigurableName() {
        return configurableClass.getName();
    }
}


class ConfigTransferHandler extends TransferHandler {

    DataFlavor serialArrayListFlavor;


    public ConfigTransferHandler() {
        serialArrayListFlavor = new DataFlavor(Class.class, "Configurable");
    }


    public boolean importData(JComponent c, Transferable t) {
        return canImport(c, t.getTransferDataFlavors());
    }


    public int getSourceActions(JComponent c) {
        return COPY_OR_MOVE;
    }


    protected Transferable createTransferable(JComponent c) {
        if (c instanceof ConfigurableTree) {
            JTree source = (JTree) c;
            if (source != null && source.getSelectionPath() != null && source.getSelectionPath().getLastPathComponent() instanceof ConfigurableNode) {
                ConfigurableNode selectedValue = (ConfigurableNode) source.getSelectionPath().getLastPathComponent();

                return new TransferableConfigurable(selectedValue.getConfigurableClass());
            }
        }

        return null;
    }
}