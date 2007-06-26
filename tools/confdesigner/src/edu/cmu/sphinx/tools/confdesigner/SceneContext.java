package edu.cmu.sphinx.tools.confdesigner;

import edu.cmu.sphinx.tools.executor.ExecutorListener;
import edu.cmu.sphinx.util.props.ConfigurationChangeListener;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertySheet;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class SceneContext implements ExecutorListener, ConfigurationChangeListener {

    private SceneController sceneController;
    private File cmLocation = null;

    private static int curUntitledNumber = 1;

    private List<PropertySheet> curSceneExecutors = new ArrayList<PropertySheet>();

    private boolean isChanged = false;
    public final static String UNTITLED = "Untitled-";


    public SceneContext(File cmLocation) {
        this.sceneController = new SceneController(new ConfigScene());
        sceneController.addExecutorListener(this);

        sceneController.setCm(cmLocation);
        sceneController.getCm().addConfigurationChangeListener(this);

        this.cmLocation = cmLocation;

    }


    public SceneController getSceneController() {
        return sceneController;
    }


    public ConfigScene getScene() {
        return sceneController.getScene();
    }


    public File getLocation() {
        if (cmLocation == null)
            cmLocation = new File(UNTITLED + (curUntitledNumber++) + ".sxl");

        return cmLocation;
    }


    public void setLocation(File selectedFile) {
        cmLocation = selectedFile;
    }


    public void addedExecutor(final PropertySheet executablePS) {
        curSceneExecutors.add(executablePS);
    }


    public void removedExecutor(PropertySheet ps) {
        curSceneExecutors.remove(ps);
    }


    public List<PropertySheet> getCurSceneExecutors() {
        return curSceneExecutors;
    }


    public boolean wasChanged() {
        return isChanged;
    }


    public void setChanged(boolean changed) {
        this.isChanged = changed;
    }


    public void configurationChanged(String configurableName, String propertyName, ConfigurationManager cm) {
        isChanged = true;
    }


    public void componentAdded(ConfigurationManager configurationManager, PropertySheet propertySheet) {
        isChanged = true;
    }


    public void componentRemoved(ConfigurationManager configurationManager, PropertySheet propertySheet) {
        isChanged = true;
    }


    public void componentRenamed(ConfigurationManager configurationManager, PropertySheet propertySheet, String oldName) {
        isChanged = true;
    }
}
