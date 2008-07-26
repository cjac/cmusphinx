package edu.cmu.sphinx.tools.confdesigner;

import edu.cmu.sphinx.tools.executor.ExecutorListener;
import edu.cmu.sphinx.util.props.*;

import java.util.*;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class GraphLoader {

    private SceneController confController;
    private ConfigScene scene;


    public GraphLoader(SceneController sceneController) {
        confController = sceneController;
        this.scene = confController.getScene();
    }


    public boolean mergeIntoScene(ConfigurationManager cm) {
        confController.getCm().addSubConfiguration(cm);
        loadScene(confController.getCm(), null, cm.getComponentNames());

        return true;
    }


    public boolean loadScene(ConfigurationManager cm, List<ExecutorListener> executorListeners, Collection<String> addCompNames) {
        Map<String, ConfNode> nodes = new HashMap<String, ConfNode>();


        for (String compName : addCompNames) {
            assert !nodes.keySet().contains(compName) : "scene already contains node named '" + compName + "'";

            nodes.put(compName, confController.addNode(cm.getPropertySheet(compName), compName));
        }

        scene.validate();

        // connect all components
        for (String compName : addCompNames) {
            connect2Graph(cm, compName, nodes);
        }

        scene.validate();

        return true;
    }


    private void connect2Graph(ConfigurationManager cm, String compName, Map<String, ConfNode> nodes) {
        PropertySheet ps = cm.getPropertySheet(compName);
        for (String propName : ps.getRegisteredProperties()) {
            if (ps.getType(propName) == PropertySheet.PropertyType.COMP) {
                String propValue = (String) ps.getRaw(propName);
                if (propValue == null)
                    continue;

                propValue = ConfigurationManagerUtils.getPropertyManager(ps).getStrippedComponentName(propValue);

                ConfNode sourceNode = nodes.get(propValue);
                ConfNode targetNode = nodes.get(compName);
                assert sourceNode != null && targetNode != null;

                ConfPin sourcePort = sourceNode.getThisPin();
                ConfPin targetPort = targetNode.getPin(propName);

                scene.createEdge(new ConfEdge(sourcePort, targetPort));

            } else if (ps.getType(propName) == PropertySheet.PropertyType.COMPLIST) {
                S4PropWrapper s4PropWrapper = null;
                try {
                    s4PropWrapper = ps.getProperty(propName, S4ComponentList.class);
                } catch (PropertyException e) {
                    e.printStackTrace();
                }

                Class<? extends Configurable> listType = ((S4ComponentList) s4PropWrapper.getAnnotation()).type();

                List<String> listComps = (List<String>) ps.getRaw(propName);
                if (listComps == null)
                    continue;

                for (int i = 0; i < listComps.size(); i++) {
                    String componentName = listComps.get(i);

                    componentName = ConfigurationManagerUtils.getPropertyManager(ps).getStrippedComponentName(componentName);

                    ConfNode targetNode = nodes.get(compName);
                    ConfNode sourceNode = nodes.get(componentName);

                    if(targetNode == null || sourceNode == null)
                        continue;
                    
                    assert sourceNode != null;
                    assert targetNode != null;

                    ConfPin sourcePort = sourceNode.getPin(ConfNode.PARENT_PIN);

                    if (getUnusedListPins(targetNode, propName, scene).size() <= 1) {
                        targetNode.addInputPin(scene, propName, i + 1, listType);
                    }


                    List<ConfPin> freeListPins = getUnusedListPins(targetNode, propName, scene);
                    scene.validate();
                    scene.createEdge(new ConfEdge(sourcePort, freeListPins.get(0)));
                }
            }

            scene.validate();
        }
    }


    public static List<ConfPin> getUnusedListPins(ConfNode targetNode, String propListName, ConfigScene scene) {
        List<ConfPin> freeListPins = new ArrayList<ConfPin>();

        for (ConfPin listPin : targetNode.getListPins(propListName)) {
            if (scene.findPinEdges(listPin, false, true).size() == 0)
                freeListPins.add(listPin);
        }

        return freeListPins;
    }
}
