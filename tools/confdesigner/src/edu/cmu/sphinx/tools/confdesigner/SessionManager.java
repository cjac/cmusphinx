package edu.cmu.sphinx.tools.confdesigner;

import org.netbeans.api.visual.model.ObjectSceneEventType;

import javax.swing.*;
import javax.swing.filechooser.FileFilter;
import java.io.*;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class SessionManager {

    List<SceneContext> sceneContexts = new ArrayList<SceneContext>();
    private ConfDesigner confDesigner;
    private SceneContext activeScene;

    private List<SesManListener> sesManListeners = new ArrayList<SesManListener>();
    private ConfigSceneListener sceneListener;

    public final static String LAYOUT_SUFFIX = ".layout";
    public final static String FORMAT_SUFFIX = ".sxl";
    public final static String PROJECT_FORMAT_SUFFIX = FORMAT_SUFFIX + ".prj";

    private File prjLocation;


    public SessionManager(final ConfDesigner confDesigner) {

        this.confDesigner = confDesigner;
        sceneListener = new ConfigSceneListener(confDesigner);
    }


    public int numConfigs() {
        return sceneContexts.size();
    }


    public void setActiveScene(SceneContext activeScene) {
        if (this.activeScene != null) {
            ConfigScene configScene = this.activeScene.getSceneController().getScene();
            configScene.removeObjectSceneListener(sceneListener);
        }

        this.activeScene = activeScene;
        SceneController controller = this.activeScene.getSceneController();
        sceneListener.setController(controller);

        controller.getScene().addObjectSceneListener(sceneListener, ObjectSceneEventType.OBJECT_SELECTION_CHANGED);

        for (SesManListener listener : sesManListeners)
            listener.newActiveScene(activeScene);
    }


    public SceneContext getActiveScene() {
        return activeScene;
    }


    public void registerSceneContext(SceneContext sc) {
        sceneContexts.add(sc);
//        setActiveScene(sc);

        for (SesManListener listener : sesManListeners) {
            listener.addedScene(sc);
        }
    }


    public void unRegisterSceneContext(SceneContext sc) {
        int tabIndex = sceneContexts.indexOf(sc);

        int nextTab = tabIndex != 0 ? tabIndex - 1 : 0;
        setActiveScene(sceneContexts.get(nextTab));

        sceneContexts.remove(sc);

        for (SesManListener listener : sesManListeners) {
            listener.removedScene(sc);
        }
    }


    /** Adds a new listener. */
    public void addSesManListener(SesManListener l) {
        if (l == null)
            return;

        sesManListeners.add(l);
    }


    /** Removes a listener. */
    public void removeSesManListener(SesManListener l) {
        if (l == null)
            return;

        sesManListeners.remove(l);
    }


    public List<SceneContext> getSceneContexts() {
        return sceneContexts;
    }


    public boolean saveScene(SceneContext sceneContext) {
        File cmLocation = sceneContext.getLocation();
        boolean didSaving;

        if (cmLocation == null) {
            didSaving = saveSceneAs(sceneContext);
        } else {
            assert cmLocation.getName().endsWith(FORMAT_SUFFIX);

            sceneContext.getSceneController().save(cmLocation);
            sceneContext.setChanged(false);

            didSaving = true;
        }

        return didSaving;
    }


    /** Returns true is the scene was saved. */
    public boolean saveSceneAs(SceneContext scene) {
        JFileChooser jfc = new JFileChooser(new File("."));
        jfc.setMultiSelectionEnabled(false);
        jfc.setFileFilter(new FileFilter() {

            public boolean accept(File f) {
                return (f.getName().endsWith(SessionManager.FORMAT_SUFFIX) && f.isFile()) || f.isDirectory();
            }


            public String getDescription() {
                return "s4 configuration files (*" + SessionManager.FORMAT_SUFFIX + ")";
            }
        });

        int status = jfc.showSaveDialog(confDesigner);
        if (status == JFileChooser.APPROVE_OPTION) {
            File cmLocation = jfc.getSelectedFile();
            if (!cmLocation.getName().endsWith(FORMAT_SUFFIX))
                cmLocation = new File(cmLocation.getAbsolutePath() + FORMAT_SUFFIX);

            scene.setLocation(cmLocation);
            confDesigner.setSceneName(scene, cmLocation.getName());
            saveScene(scene);
            return true;
        }

        return false;
    }


    public void loadScene() {
        JFileChooser jfc = new JFileChooser(new File("."));

        jfc.setMultiSelectionEnabled(false);
        jfc.setFileFilter(new FileFilter() {

            public boolean accept(File f) {
                return (f.getName().endsWith(SessionManager.FORMAT_SUFFIX) && f.isFile()) || f.isDirectory();
            }


            public String getDescription() {
                return "s4 configuration files (*" + SessionManager.FORMAT_SUFFIX + ")";
            }
        });

        int status = jfc.showOpenDialog(confDesigner);
        if (status == JFileChooser.APPROVE_OPTION) {
            File cmLocation = jfc.getSelectedFile();
            loadScene(cmLocation);
            confDesigner.updateRecentFiles(cmLocation);
        }
    }


    public void loadScene(File cmLocation) {
        // reuse the current scene if it's empty
        SceneContext activeScene = getActiveScene();

        if (activeScene.getSceneController().getCm().getComponentNames().isEmpty()) {
            activeScene.getSceneController().setCm(cmLocation);

            activeScene.getSceneController().getCm().addConfigurationChangeListener(activeScene);
            activeScene.setLocation(cmLocation);
            confDesigner.setSceneName(activeScene, cmLocation.getName());

            setActiveScene(activeScene);

        } else {
            SceneContext newSceneContext = new SceneContext(cmLocation);
            registerSceneContext(newSceneContext);
            setActiveScene(newSceneContext);
        }
    }


    public void processUnsavedChanges(List<SceneContext> sceneContexts) {
        // collect all modified but still unchanged scene contexts
        Collection<SceneContext> changedScenes = new ArrayList<SceneContext>();
        for (SceneContext sceneContext : sceneContexts) {
            if (sceneContext.wasChanged())
                changedScenes.add(sceneContext);
        }

        for (SceneContext changedScene : changedScenes) {
            File location = changedScene.getLocation();

            int status = JOptionPane.showConfirmDialog(confDesigner, "There are unsaved changes in " + location.getName() + ". Do you want to save them ?", "Current configuration changed", JOptionPane.YES_NO_OPTION);
            if (status == JOptionPane.YES_OPTION) {
                boolean didSaving;

                if (changedScene.getLocation().getName().contains(SceneContext.UNTITLED)) {
                    didSaving = saveSceneAs(changedScene);
                } else {
                    didSaving = saveScene(changedScene);
                }

                if (didSaving)
                    changedScene.setChanged(false);
            }
        }
    }


    public void loadProject() {
        JFileChooser jfc = new JFileChooser(new File("."));

        jfc.setMultiSelectionEnabled(false);
        jfc.setFileFilter(new FileFilter() {

            public boolean accept(File f) {
                return (f.getName().endsWith(SessionManager.PROJECT_FORMAT_SUFFIX) && f.isFile()) || f.isDirectory();
            }


            public String getDescription() {
                return "s4 configuration project (*" + SessionManager.PROJECT_FORMAT_SUFFIX + ")";
            }
        });

        int status = jfc.showOpenDialog(confDesigner);
        if (status == JFileChooser.APPROVE_OPTION) {
            File prjLocation = jfc.getSelectedFile();

            loadProject(prjLocation);
            confDesigner.updateRecentFiles(prjLocation);
        }
    }


    public void loadProject(File prjLocation) {
        // close all open tabs
        closeAll();

        try {
            // read the names of all project elements
            BufferedReader br = new BufferedReader(new FileReader(prjLocation));
            String prjElement;
            while ((prjElement = br.readLine()) != null) {
                if (prjElement.length() == 0)
                    continue;

                assert new File(prjElement).isFile() : "Project contains non-exisiting elements (" + prjElement + ")";
                loadScene(new File(prjElement));
            }

            br.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    public void closeAll() {
        processUnsavedChanges(getSceneContexts());

        List<SceneContext> allContexts = new ArrayList<SceneContext>(getSceneContexts());

        for (SceneContext sc : allContexts) {
            unRegisterSceneContext(sc);
        }

        prjLocation = null;
    }


    public void saveProject() {
        processUnsavedChanges(getSceneContexts());

        if (prjLocation == null) {
            saveProjectAs();
        } else {
            try {
                BufferedWriter bw = new BufferedWriter(new FileWriter(prjLocation));
                for (SceneContext sceneContext : sceneContexts) {
                    File sceneLocation = sceneContext.getLocation();
                    if (sceneLocation.isFile()) {
                        bw.write(sceneLocation.getAbsolutePath());
                        bw.newLine();
                    }
                }

                bw.flush();
                bw.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        confDesigner.updateRecentFiles(prjLocation);
    }


    public void saveProjectAs() {

        JFileChooser jfc = new JFileChooser(new File("."));
        jfc.setMultiSelectionEnabled(false);
        jfc.setFileFilter(new FileFilter() {

            public boolean accept(File f) {
                return (f.getName().endsWith(SessionManager.PROJECT_FORMAT_SUFFIX) && f.isFile()) || f.isDirectory();
            }


            public String getDescription() {
                return "s4 configuration project (*" + SessionManager.PROJECT_FORMAT_SUFFIX + ")";
            }
        });

        int status = jfc.showSaveDialog(confDesigner);
        if (status == JFileChooser.APPROVE_OPTION) {
            prjLocation = jfc.getSelectedFile();
            if (!prjLocation.getName().endsWith(PROJECT_FORMAT_SUFFIX))
                prjLocation = new File(prjLocation.getAbsolutePath() + PROJECT_FORMAT_SUFFIX);

            saveProject();
        }
    }
}