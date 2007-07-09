package edu.cmu.sphinx.tools.confdesigner.actions;

import edu.cmu.sphinx.tools.confdesigner.*;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import org.netbeans.api.visual.model.ObjectSceneEvent;
import org.netbeans.api.visual.model.ObjectSceneEventType;

import javax.swing.*;
import java.awt.*;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.StringSelection;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.util.*;
import java.util.List;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class CopySubGraphAction extends AbstractAction implements SesManListener {

    private SessionManager sesMan;
    private ObjectSceneListenerAdapter sceneListener;


    public CopySubGraphAction(SessionManager sesMan, Clipboard clipBoard) {
        this.sesMan = sesMan;

        putValue(NAME, "Copy");
        putValue(ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_C, KeyEvent.CTRL_MASK));
        putValue(MNEMONIC_KEY, KeyEvent.VK_C);
        setEnabled(false);

        sesMan.addSesManListener(this);

        sceneListener = new ObjectSceneListenerAdapter() {

            public void selectionChanged(ObjectSceneEvent event, Set<Object> previousSelection, Set<Object> newSelection) {
                super.selectionChanged(event, previousSelection, newSelection);
                setEnabled(!newSelection.isEmpty());
            }
        };
    }


    public void actionPerformed(ActionEvent e) {
        // copy the current selection into the clipboard. beside the selected graph nodes we also copy all connections
        // between selected nodes here

        // extract the currently selected subgraph
        Set<?> selectedObjects = sesMan.getActiveScene().getScene().getSelectedObjects();

        HashMap<String, Point> nodeLocations = new HashMap<String, Point>();
        ConfigurationManager subGraphCM = new ConfigurationManager();

        ConfigScene scene = sesMan.getActiveScene().getScene();
        // collect all nodes
        for (Object selectedObject : selectedObjects) {
            if (selectedObject instanceof ConfNode) {
                ConfNode selNode = (ConfNode) selectedObject;
                String instanceName = selNode.getInstanceName();

                PropertySheet ps = selNode.getPropSheet();
                Map<String, Object> basicRawProps = new HashMap<String, Object>();
                for (String propName : ps.getRegisteredProperties()) {
                    switch (ps.getType(propName)) {
                        case COMP:
                            break;
                        case COMPLIST:
                            break;
                        default:
                            basicRawProps.put(propName, ps.getRaw(propName));
                    }
                }

                subGraphCM.addConfigurable(ps.getConfigurableClass(), instanceName, basicRawProps);
                nodeLocations.put(instanceName, scene.findWidgetByName(instanceName).getLocation());
            }
        }

        // collect all connections
        Collection<String> subGraphComps = subGraphCM.getComponentNames();
        ConfigurationManager baseCM = sesMan.getActiveScene().getSceneController().getCm();

        for (Object selectedObject : selectedObjects) {
            if (selectedObject instanceof ConfEdge) {
                ConfEdge edge = (ConfEdge) selectedObject;

                ConfPin sourcePin = edge.getSource();
                ConfPin targetPin = edge.getTarget();

                ConfNode sourceNode = scene.getPinNode(sourcePin);
                ConfNode targetNode = scene.getPinNode(targetPin);

                if (subGraphComps.contains(sourceNode.getInstanceName()) && subGraphComps.contains(targetNode.getInstanceName())) {
                    if (edge.getTarget().isListPin()) {
                        // get the old list
                        String nodeName = targetNode.getInstanceName();
                        String listName = targetPin.getPropName();

                        List<String> oldList = (List<String>) baseCM.getPropertySheet(nodeName).getRaw(listName);

                        // get the intermediate new one
                        PropertySheet ps = subGraphCM.getPropertySheet(nodeName);
                        List<String> newList = (List<String>) ps.getRaw(listName);
                        if (newList == null)
                            newList = new ArrayList<String>();

                        // merge them
                        List<String> mergeList = new ArrayList<String>(oldList);
                        for (int i = 0; i < mergeList.size(); i++) {
                            String listEl = mergeList.get(i);
                            if (!(newList.contains(listEl) || sourceNode.getInstanceName().equals(listEl))) {
                                mergeList.remove(i);
                                i--;
                            }
                        }

                        try {
                            ps.setComponentList(listName, mergeList, null);
                        } catch (PropertyException e1) {
                            e1.printStackTrace();
                        }
                    } else {
                        PropertySheet ps = subGraphCM.getPropertySheet(targetNode.getInstanceName());
                        try {
                            ps.setComponent(targetPin.getPropName(), sourceNode.getInstanceName(), null);
                        } catch (PropertyException e1) {
                            e1.printStackTrace();
                        }
                    }
                }
            }
        }

        GraphSelection graphSelection = new GraphSelection(subGraphCM, nodeLocations);

        ConfDesigner.getClipBoard().setContents(new StringSelection("TT"), graphSelection);
        ConfDesigner.getClipBoard().setContents(graphSelection, graphSelection);
    }


    public void newActiveScene(SceneContext activeScene) {
        setEnabled(!activeScene.getScene().getSelectedObjects().isEmpty());
    }


    public void removedScene(SceneContext sc) {
        sc.getScene().removeObjectSceneListener(sceneListener, ObjectSceneEventType.OBJECT_SELECTION_CHANGED);
    }


    public void addedScene(SceneContext sc) {
        sc.getScene().addObjectSceneListener(sceneListener, ObjectSceneEventType.OBJECT_SELECTION_CHANGED);
    }
}


