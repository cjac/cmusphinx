/*
 * Created by JFormDesigner on Fri May 04 20:41:31 CEST 2007
 */

package edu.cmu.sphinx.tools.confdesigner;

import edu.cmu.sphinx.decoder.Decoder;
import edu.cmu.sphinx.decoder.scorer.ThreadedAcousticScorer;
import edu.cmu.sphinx.decoder.search.SimpleBreadthFirstSearchManager;
import edu.cmu.sphinx.frontend.FrontEnd;
import edu.cmu.sphinx.frontend.transform.DiscreteCosineTransform;
import edu.cmu.sphinx.frontend.util.WavWriter;
import edu.cmu.sphinx.tools.confdesigner.actions.*;
import edu.cmu.sphinx.tools.confdesigner.conftree.ConfigSelector;
import edu.cmu.sphinx.tools.confdesigner.util.SceneFinder;
import edu.cmu.sphinx.tools.executor.ExecutableExecutor;
import edu.cmu.sphinx.tools.executor.ExecutorListener;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.*;
import java.awt.datatransfer.Clipboard;
import java.awt.event.*;
import java.io.File;
import java.io.IOException;
import java.util.*;
import java.util.List;
import java.util.prefs.Preferences;


/** @author Holger Brandl */
public class ConfDesigner extends JFrame implements ExecutorListener {

    public static final String CONF_DESIGNER = "ConfDesigner";

    private JDialog sateliteDialog;
    JTabbedPane tabPane;
    private List<File> recentFiles = new ArrayList<File>();
    private ActionListener recentFileListener;

    private static Preferences prefs;

    private SessionManager sesMan;
    private Map<PropertySheet, JMenuItem> curSceneExecutors = new HashMap<PropertySheet, JMenuItem>();
    Map<SceneContext, JComponent> projectsTabs = new HashMap<SceneContext, JComponent>();

    private static Clipboard clipboard;


    public ConfDesigner() {
        initComponents();

        sesMan = new SessionManager(this);

//        cmLocation = new File("lala.sxl");
//        cmLocation = new File("../../sphinx4/src/demo/sphinx/hellongram/hellongram.config.xml");

        addWindowListener(new WindowAdapter() {

            public void windowClosing(WindowEvent e) {
                super.windowClosing(e);
                exitItemActionPerformed();
            }
        });

        addComponentListener(new ComponentAdapter() {

            public void componentResized(ComponentEvent e) {
                getPrefs().putInt("mainwin.width", getWidth());
                getPrefs().putInt("mainwin.height", getHeight());
                getPrefs().putInt("mainwin.xpos", (int) getLocation().getX());
                getPrefs().putInt("mainwin.ypos", (int) getLocation().getY());
            }
        });

        recentFileListener = new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                for (int i = 0; i < recentMenu.getItemCount(); i++) {
                    if (recentMenu.getItem(i).equals(e.getSource())) {
                        File recentFile = recentFiles.get(i);
                        if (recentFile.getName().endsWith(SessionManager.FORMAT_SUFFIX))
                            sesMan.loadScene(recentFile);
                        else if (recentFile.getName().endsWith(SessionManager.PROJECT_FORMAT_SUFFIX))
                            sesMan.loadProject(recentFile);
                        else
                            assert false : "invaled recent file " + recentFile.getAbsolutePath();
                    }
                }
            }
        };

        setFocusable(true);
        addKeyListener(new KeyAdapter() {
            public void keyPressed(KeyEvent e) {
                if (e.getKeyCode() == KeyEvent.VK_F && e.isControlDown())
                    sceneFinder.requestFocus();

//                if (e.getKeyCode() == KeyEvent.VK_S && e.isControlDown())
//                    saveImage2File(null);
            }
        });

//        addDummyNodes(scene);
        tabPane = new JTabbedPane();
        tabPane.addMouseListener(new MouseAdapter() {

            public void mouseClicked(MouseEvent e) {
                if (e.getButton() == MouseEvent.BUTTON2)
                    closeTabItemActionPerformed();
            }
        });
        tabPane.addChangeListener(new ChangeListener() {

            public void stateChanged(ChangeEvent e) {
                Component selectedTab = tabPane.getSelectedComponent();
                SceneContext newActiveScene = null;
                for (SceneContext sc : projectsTabs.keySet()) {
                    if (projectsTabs.get(sc).equals(selectedTab))
                        newActiveScene = sc;
                }

                if (newActiveScene != null) {
                    sesMan.setActiveScene(newActiveScene);
                    ConfDesigner.this.setTitle(CONF_DESIGNER + " - " + newActiveScene.getLocation().getName());
                }
            }
        });

        sesMan.addSesManListener(new SesManListener() {

            public void newActiveScene(SceneContext sc) {
//                if (sesMan.numConfigs() > 1)
//                    tabPane.getModel().setSelectedIndex(tabPane.indexOfComponent(projectsTabs.get(sc)));

                SceneController sceneController = sc.getSceneController();
                ConfigScene scene = sceneController.getScene();

                sceneController.getScene().getView().requestFocusInWindow();

                birdViewPanel.removeAll();
                birdViewPanel.add(scene.createSatelliteView());
                birdViewPanel.validate();

                scene.setSnap2Grid(snap2GridItem.isSelected());

                configurableTree.setController(sceneController);

                for (PropertySheet ps : curSceneExecutors.keySet())
                    removedExecutor(ps);
                for (PropertySheet ps : sc.getCurSceneExecutors())
                    addedExecutor(ps);

                propSheetPanel.setConfigurationManager(sceneController.getCm());

                // if there is a single selected object reselect it in order to show it wihtin the props-panel
                Set<?> selectedObjects = sc.getScene().getSelectedObjects();
                if (selectedObjects.size() == 1) {
                    Object selObject = selectedObjects.iterator().next();
                    if (selObject instanceof ConfNode)
                        propSheetPanel.rebuildPanel(((ConfNode) selObject).getPropSheet());
                }
            }


            public void addedScene(SceneContext sc) {
                JScrollPane scrollPane = new JScrollPane();
                scrollPane.setViewportView(sc.getSceneController().getScene().getView());

                projectsTabs.put(sc, scrollPane);

                if (sesMan.numConfigs() == 1) {
                    if (!(scenePane.getComponents().length > 0 && scenePane.getComponent(0) instanceof JScrollPane)) {
                        scenePane.removeAll();
                        scenePane.add(scrollPane);
                        ConfDesigner.this.setTitle(CONF_DESIGNER + " - " + sc.getLocation().getName());
                    }

                } else {
                    if (scenePane.getComponents().length == 0) {
                        scenePane.add(tabPane);
                    } else if (scenePane.getComponent(0) instanceof JScrollPane) {
                        JScrollPane scrollPaneOld = (JScrollPane) scenePane.getComponent(0);

                        scenePane.removeAll();
                        scenePane.add(tabPane);

                        tabPane.addTab(sesMan.getSceneContexts().get(0).getLocation().getName(), scrollPaneOld);
                    }

                    tabPane.addTab(sc.getLocation().getName(), scrollPane);
                    tabPane.validate();
                    tabPane.getModel().setSelectedIndex(tabPane.indexOfComponent(scrollPane));
                }

                sc.getSceneController().addExecutorListener(ConfDesigner.this);

                scenePane.validate();
                sesMan.setActiveScene(sc);
            }


            public void removedScene(SceneContext sc) {
                sc.getSceneController().removeExecutorListener(ConfDesigner.this);

                if (sesMan.numConfigs() > 0) {
                    tabPane.remove(projectsTabs.get(sc));

                } else {
                    scenePane.removeAll();
                    tabPane.removeAll();

                    SceneContext initalSceneContext = new SceneContext(null);
                    sesMan.registerSceneContext(initalSceneContext);
                    sesMan.setActiveScene(initalSceneContext);
                }

                tabPane.validate();
                scenePane.validate();
            }
        });


        expSceneImgItem.setAction(new ExportImageAction(sesMan, false, this));
        fitViewItem.setAction(new FitViewAction(sesMan));
        helpItem.setAction(new UrlAction("Help", "http://en.wikipedia.org/wiki/ConfDesigner", this));

        pasteItem.setAction(new PasteSubGraphAction(sesMan, getClipBoard()));
        copyItem.setAction(new CopySubGraphAction(sesMan, getClipBoard()));

        DeleteSubGraphAction deleteAction = new DeleteSubGraphAction(sesMan);
        deleteItem.setAction(deleteAction);
        cutItem.setAction(new CutSubGraphAction(sesMan, getClipBoard(), deleteAction));


        snap2GridItem.setSelected(ConfDesigner.getPrefs().getBoolean("snap2Grid", true));
        snap2GridItem.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                sesMan.getActiveScene().getScene().setSnap2Grid(snap2GridItem.isSelected());
                ConfDesigner.getPrefs().putBoolean("snap2Grid", snap2GridItem.isSelected());
            }
        });

        // intit the view
        SceneContext initalSceneContext = new SceneContext(null);
        sesMan.registerSceneContext(initalSceneContext);
        sesMan.setActiveScene(initalSceneContext);

        updateRecentFiles(null);
    }


    public void addConfigurables(Collection<Class<? extends Configurable>> configClasses) {
        configurableTree.addConfigurables(configClasses);
    }


    private void addDummyNodes(ConfigScene scene) throws IOException, PropertyException {
//        ConfigurationManager cm = new ConfigurationManager(Utils.getURL(Azubi.DEFAULT_CONFIG_XML));

        sesMan.getActiveScene().getSceneController().addNode(ThreadedAcousticScorer.class, null);
        sesMan.getActiveScene().getSceneController().addNode(FrontEnd.class, null);
        sesMan.getActiveScene().getSceneController().addNode(SimpleBreadthFirstSearchManager.class, null);
        sesMan.getActiveScene().getSceneController().addNode(Decoder.class, null);
        sesMan.getActiveScene().getSceneController().addNode(WavWriter.class, null);
        sesMan.getActiveScene().getSceneController().addNode(DiscreteCosineTransform.class, null);

        scene.validate();
    }


    public SessionManager getSesMan() {
        return sesMan;
    }


    public void addedExecutor(final PropertySheet executablePS) {
        JMenuItem menuItem = new JMenuItem();
        menuItem.addActionListener(new ExecutableExecutor(executablePS));
        menuItem.setText("Start '" + executablePS.getInstanceName() + "'");

        runMenu.setEnabled(true);
        runMenu.add(menuItem);

        curSceneExecutors.put(executablePS, menuItem);
    }


    public void removedExecutor(PropertySheet ps) {
        JMenuItem item = curSceneExecutors.remove(ps);
        if (item == null)
            return;

        runMenu.remove(item);
        runMenu.validate();

        if (curSceneExecutors.isEmpty()) {
            runMenu.setEnabled(false);
            runMenu.setToolTipText("Can not find any exectutables within the current scene");
        } else {
            runMenu.setEnabled(true);
            runMenu.setToolTipText(null);
        }
    }


    private void classFilterFieldActionPerformed() {
        // getSomeInput
        String filterText = classFilterField.getText();

        configurableTree.setFilter(filterText);
    }


    private void filterResetButtonActionPerformed() {
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                configurableTree.setFilter(null);
                classFilterField.setText("");
                classFilterField.requestFocus();
            }
        });
    }


    private void newItemActionPerformed() {

        SceneContext newSC = new SceneContext(null);
        sesMan.registerSceneContext(newSC);
        sesMan.setActiveScene(newSC);
//        sceneController.setCm(new ConfigurationManager());
//        propSheetPanel.setConfigurationManager(sceneController.getCm());
    }


    private void showBirdViewItemActionPerformed() {
        if (sateliteDialog == null) {
            sateliteDialog = new JDialog(this);
            sateliteDialog.setBounds(50, 500, 200, 150);
            JPanel panel = new JPanel(new BorderLayout());
            panel.add(sesMan.getActiveScene().getScene().createSatelliteView());

            sateliteDialog.getContentPane().add(panel);
        }

//        showBirdViewItem.setSelected(!showBirdViewItem.isSelected());

        sateliteDialog.setVisible(showBirdViewItem.isSelected());
    }


    private void layoutGraphItemActionPerformed() {
        sesMan.getActiveScene().getScene().layoutScene();
        System.err.println("scene layout updated");
    }


    private void aboutItemActionPerformed() {
        JOptionPane.showMessageDialog(this, "ConfDesigner 1.0 beta 2+", "About", JOptionPane.INFORMATION_MESSAGE);
    }


    private void sceneFinderActionPerformed() {
        boolean searchSucessful = new SceneFinder(this, sesMan.getActiveScene().getSceneController()).process(sceneFinder.getText());

        if (searchSucessful)
            sceneFinder.setText("");
    }


    public PropertyEditorPanel getPropSheetPanel() {
        return propSheetPanel;
    }


    void saveItemActionPerformed() {
        SceneContext tab = sesMan.getActiveScene();
        sesMan.saveScene(tab);
    }


    void saveAsItemActionPerformed() {
        SceneContext activeScene = sesMan.getActiveScene();

        sesMan.saveSceneAs(activeScene);
    }


    private void loadItemActionPerformed() {
        sesMan.loadScene();
    }


    private void loadPrjItemActionPerformed() {
        sesMan.loadProject();
        // TODO add your code here
    }


    private void closeTabItemActionPerformed() {
        sesMan.processUnsavedChanges(Arrays.asList(sesMan.getActiveScene()));
        sesMan.unRegisterSceneContext(sesMan.getActiveScene());
    }


    private void closePrjItemActionPerformed() {
        sesMan.closeAll();
    }


    private void exitItemActionPerformed() {
        closePrjItemActionPerformed();
        System.exit(0);
    }


    private void savePrjItemActionPerformed() {
        sesMan.saveProject();
    }


    private void savePrjAsItemActionPerformed() {
        sesMan.saveProjectAs();
    }


    public void setSceneName(SceneContext scene, String newSceneName) {
        if (sesMan.getActiveScene().equals(scene))
            setTitle(ConfDesigner.CONF_DESIGNER + " - " + newSceneName);

        if (tabPane.getComponentCount() > 0)
            tabPane.setTitleAt(tabPane.indexOfComponent(projectsTabs.get(scene)), newSceneName);
    }


    public synchronized static Preferences getPrefs() {
        if (prefs == null) {
            prefs = Preferences.userNodeForPackage(ConfDesigner.class);
        }

        return prefs;
    }


    /** Updates the recent file dialog. */
    public void updateRecentFiles(File fileLocation) {
        // get the list
        Preferences p = getPrefs();
        recentFiles.clear();

        for (int i = 0; i < 5; i++) {
            String path = p.get("recent" + i, null);
            if (path != null && new File(path).isFile()) {
                recentFiles.add(new File(path));
            }
        }

        //update it
        if (fileLocation != null)
            if (recentFiles.isEmpty() || !recentFiles.get(0).equals(fileLocation))
                recentFiles.add(0, fileLocation);


        for (int i = 0; i < recentFiles.size(); i++) {
            File file = recentFiles.get(i);
            p.put("recent" + i, file.getAbsolutePath());
        }

        //update the menu
        recentMenu.removeAll();
        for (File prefFile : recentFiles) {
            JMenuItem menuItem = new JMenuItem(prefFile.getName());
            menuItem.addActionListener(recentFileListener);

            recentMenu.add(menuItem);
        }
    }


    private void initComponents() {
        // JFormDesigner - Component initialization - DO NOT MODIFY  //GEN-BEGIN:initComponents
        // Generated using JFormDesigner Open Source Project license - Sphinx-4 (cmusphinx.sourceforge.net/sphinx4/)
        menuBar1 = new JMenuBar();
        menu1 = new JMenu();
        newTabItem = new JMenuItem();
        loadItem = new JMenuItem();
        saveTabItem = new JMenuItem();
        saveTabAsItem = new JMenuItem();
        loadPrjItem = new JMenuItem();
        savePrjItem = new JMenuItem();
        savePrjAsItem = new JMenuItem();
        recentMenu = new JMenu();
        closeTabItem = new JMenuItem();
        closePrjItem = new JMenuItem();
        exitItem = new JMenuItem();
        menu2 = new JMenu();
        cutItem = new JMenuItem();
        copyItem = new JMenuItem();
        pasteItem = new JMenuItem();
        deleteItem = new JMenuItem();
        menu3 = new JMenu();
        snap2GridItem = new JCheckBoxMenuItem();
        showBirdViewItem = new JCheckBoxMenuItem();
        fitViewItem = new JMenuItem();
        layoutGraphItem = new JMenuItem();
        expSceneImgItem = new JMenuItem();
        runMenu = new JMenu();
        menu4 = new JMenu();
        helpItem = new JMenuItem();
        aboutItem = new JMenuItem();
        hSpacer1 = new JPanel(null);
        label2 = new JLabel();
        sceneFinder = new JTextField();
        splitPane1 = new JSplitPane();
        splitPane2 = new JSplitPane();
        panel4 = new JPanel();
        filterPanel = new JPanel();
        label1 = new JLabel();
        classFilterField = new JTextField();
        filterResetButton = new JButton();
        configurableTree = new ConfigSelector();
        splitPane3 = new JSplitPane();
        propSheetPanel = new PropertyEditorPanelNT();
        birdViewPanel = new JPanel();
        scenePane = new JPanel();

        //======== this ========
        setTitle("ConfDesigner");
        Container contentPane = getContentPane();
        contentPane.setLayout(new BorderLayout());

        //======== menuBar1 ========
        {

            //======== menu1 ========
            {
                menu1.setText("File");

                //---- newTabItem ----
                newTabItem.setText("New Tab");
                newTabItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
                newTabItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        newItemActionPerformed();
                    }
                });
                menu1.add(newTabItem);

                //---- loadItem ----
                loadItem.setText("Load...");
                loadItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_L, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
                loadItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        loadItemActionPerformed();
                    }
                });
                menu1.add(loadItem);

                //---- saveTabItem ----
                saveTabItem.setText("Save Tab");
                saveTabItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
                saveTabItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        saveItemActionPerformed();
                    }
                });
                menu1.add(saveTabItem);

                //---- saveTabAsItem ----
                saveTabAsItem.setText("Save Tab As...");
                saveTabAsItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, KeyEvent.CTRL_MASK | KeyEvent.ALT_MASK));
                saveTabAsItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        saveAsItemActionPerformed();
                    }
                });
                menu1.add(saveTabAsItem);
                menu1.addSeparator();

                //---- loadPrjItem ----
                loadPrjItem.setText("Load Project ...");
                loadPrjItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_L, KeyEvent.CTRL_MASK | KeyEvent.SHIFT_MASK));
                loadPrjItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        loadPrjItemActionPerformed();
                    }
                });
                menu1.add(loadPrjItem);

                //---- savePrjItem ----
                savePrjItem.setText("Save Project");
                savePrjItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, KeyEvent.CTRL_MASK | KeyEvent.SHIFT_MASK));
                savePrjItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        savePrjItemActionPerformed();
                    }
                });
                menu1.add(savePrjItem);

                //---- savePrjAsItem ----
                savePrjAsItem.setText("Save Project As ...");
                savePrjAsItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, KeyEvent.CTRL_MASK | KeyEvent.ALT_MASK | KeyEvent.SHIFT_MASK));
                savePrjAsItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        savePrjAsItemActionPerformed();
                    }
                });
                menu1.add(savePrjAsItem);
                menu1.addSeparator();

                //======== recentMenu ========
                {
                    recentMenu.setText("Open Recent");
                }
                menu1.add(recentMenu);
                menu1.addSeparator();

                //---- closeTabItem ----
                closeTabItem.setText("Close Tab");
                closeTabItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_W, KeyEvent.CTRL_MASK));
                closeTabItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        closeTabItemActionPerformed();
                    }
                });
                menu1.add(closeTabItem);

                //---- closePrjItem ----
                closePrjItem.setText("Close Project");
                closePrjItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_W, KeyEvent.CTRL_MASK | KeyEvent.SHIFT_MASK));
                closePrjItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        closePrjItemActionPerformed();
                    }
                });
                menu1.add(closePrjItem);
                menu1.addSeparator();

                //---- exitItem ----
                exitItem.setText("Exit");
                exitItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Q, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
                exitItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        exitItemActionPerformed();
                    }
                });
                menu1.add(exitItem);
            }
            menuBar1.add(menu1);

            //======== menu2 ========
            {
                menu2.setText("Edit");

                //---- cutItem ----
                cutItem.setText("Cut");
                menu2.add(cutItem);

                //---- copyItem ----
                copyItem.setText("Copy");
                menu2.add(copyItem);

                //---- pasteItem ----
                pasteItem.setText("Paste");
                menu2.add(pasteItem);

                //---- deleteItem ----
                deleteItem.setText("Delete");
                menu2.add(deleteItem);
            }
            menuBar1.add(menu2);

            //======== menu3 ========
            {
                menu3.setText("View");

                //---- snap2GridItem ----
                snap2GridItem.setText("Snap to Grid");
                menu3.add(snap2GridItem);

                //---- showBirdViewItem ----
                showBirdViewItem.setText("Show Birdview");
                showBirdViewItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_B, KeyEvent.CTRL_MASK));
                showBirdViewItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        showBirdViewItemActionPerformed();
                    }
                });
                menu3.add(showBirdViewItem);
                menu3.addSeparator();

                //---- fitViewItem ----
                fitViewItem.setText("Fit View");
                fitViewItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F, KeyEvent.ALT_MASK));
                menu3.add(fitViewItem);

                //---- layoutGraphItem ----
                layoutGraphItem.setText("Relayout Graph");
                layoutGraphItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_L, KeyEvent.ALT_MASK));
                layoutGraphItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        layoutGraphItemActionPerformed();
                    }
                });
                menu3.add(layoutGraphItem);
                menu3.addSeparator();

                //---- expSceneImgItem ----
                expSceneImgItem.setText("Export Scene Image");
                menu3.add(expSceneImgItem);
            }
            menuBar1.add(menu3);

            //======== runMenu ========
            {
                runMenu.setText("Run");
                runMenu.setToolTipText("Can not find any exectutables within the current scene");
                runMenu.setEnabled(false);
            }
            menuBar1.add(runMenu);

            //======== menu4 ========
            {
                menu4.setText("Help");

                //---- helpItem ----
                helpItem.setText("Help");
                helpItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F1, 0));
                menu4.add(helpItem);

                //---- aboutItem ----
                aboutItem.setText("About");
                aboutItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        aboutItemActionPerformed();
                    }
                });
                menu4.add(aboutItem);
            }
            menuBar1.add(menu4);
            menuBar1.add(hSpacer1);

            //---- label2 ----
            label2.setText(" find : ");
            menuBar1.add(label2);

            //---- sceneFinder ----
            sceneFinder.setMaximumSize(new Dimension(100, 100));
            sceneFinder.setPreferredSize(new Dimension(80, 20));
            sceneFinder.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    sceneFinderActionPerformed();
                }
            });
            menuBar1.add(sceneFinder);
        }
        setJMenuBar(menuBar1);

        //======== splitPane1 ========
        {
            splitPane1.setDividerSize(2);
            splitPane1.setResizeWeight(0.1);
            splitPane1.setDividerLocation(300);

            //======== splitPane2 ========
            {
                splitPane2.setOrientation(JSplitPane.VERTICAL_SPLIT);
                splitPane2.setResizeWeight(0.8);
                splitPane2.setDividerLocation(300);
                splitPane2.setDividerSize(2);
                splitPane2.setLastDividerLocation(350);

                //======== panel4 ========
                {
                    panel4.setPreferredSize(new Dimension(200, 400));
                    panel4.setMinimumSize(null);
                    panel4.setMaximumSize(null);
                    panel4.setLayout(new BorderLayout());

                    //======== filterPanel ========
                    {
                        filterPanel.setLayout(new BorderLayout());

                        //---- label1 ----
                        label1.setText(" class filter : ");
                        filterPanel.add(label1, BorderLayout.WEST);

                        //---- classFilterField ----
                        classFilterField.addActionListener(new ActionListener() {
                            public void actionPerformed(ActionEvent e) {
                                classFilterFieldActionPerformed();
                            }
                        });
                        filterPanel.add(classFilterField, BorderLayout.CENTER);

                        //---- filterResetButton ----
                        filterResetButton.setText("x");
                        filterResetButton.addActionListener(new ActionListener() {
                            public void actionPerformed(ActionEvent e) {
                                filterResetButtonActionPerformed();
                            }
                        });
                        filterPanel.add(filterResetButton, BorderLayout.EAST);
                    }
                    panel4.add(filterPanel, BorderLayout.SOUTH);

                    //---- configurableTree ----
                    configurableTree.setMaximumSize(new Dimension(2111, 2111));
                    configurableTree.setMinimumSize(new Dimension(100, 200));
                    configurableTree.setPreferredSize(null);
                    panel4.add(configurableTree, BorderLayout.CENTER);
                }
                splitPane2.setTopComponent(panel4);

                //======== splitPane3 ========
                {
                    splitPane3.setOrientation(JSplitPane.VERTICAL_SPLIT);
                    splitPane3.setResizeWeight(1.0);
                    splitPane3.setMinimumSize(null);
                    splitPane3.setPreferredSize(new Dimension(456, 300));

                    //---- propSheetPanel ----
                    propSheetPanel.setMinimumSize(null);
                    propSheetPanel.setMaximumSize(null);
                    propSheetPanel.setPreferredSize(new Dimension(100, 100));
                    splitPane3.setTopComponent(propSheetPanel);

                    //======== birdViewPanel ========
                    {
                        birdViewPanel.setMinimumSize(null);
                        birdViewPanel.setMaximumSize(null);
                        birdViewPanel.setPreferredSize(new Dimension(150, 150));
                        birdViewPanel.setLayout(new BorderLayout());
                    }
                    splitPane3.setBottomComponent(birdViewPanel);
                }
                splitPane2.setBottomComponent(splitPane3);
            }
            splitPane1.setLeftComponent(splitPane2);

            //======== scenePane ========
            {
                scenePane.setPreferredSize(new Dimension(200, 200));
                scenePane.setFocusTraversalPolicyProvider(true);
                scenePane.setLayout(new BorderLayout());
            }
            splitPane1.setRightComponent(scenePane);
        }
        contentPane.add(splitPane1, BorderLayout.CENTER);
        pack();
        setLocationRelativeTo(getOwner());
        // JFormDesigner - End of component initialization  //GEN-END:initComponents
    }


    // JFormDesigner - Variables declaration - DO NOT MODIFY  //GEN-BEGIN:variables
    // Generated using JFormDesigner Open Source Project license - Sphinx-4 (cmusphinx.sourceforge.net/sphinx4/)
    private JMenuBar menuBar1;
    private JMenu menu1;
    private JMenuItem newTabItem;
    private JMenuItem loadItem;
    private JMenuItem saveTabItem;
    private JMenuItem saveTabAsItem;
    private JMenuItem loadPrjItem;
    private JMenuItem savePrjItem;
    private JMenuItem savePrjAsItem;
    private JMenu recentMenu;
    private JMenuItem closeTabItem;
    private JMenuItem closePrjItem;
    private JMenuItem exitItem;
    private JMenu menu2;
    private JMenuItem cutItem;
    private JMenuItem copyItem;
    private JMenuItem pasteItem;
    private JMenuItem deleteItem;
    private JMenu menu3;
    private JCheckBoxMenuItem snap2GridItem;
    private JCheckBoxMenuItem showBirdViewItem;
    private JMenuItem fitViewItem;
    private JMenuItem layoutGraphItem;
    private JMenuItem expSceneImgItem;
    private JMenu runMenu;
    private JMenu menu4;
    private JMenuItem helpItem;
    private JMenuItem aboutItem;
    private JPanel hSpacer1;
    private JLabel label2;
    private JTextField sceneFinder;
    private JSplitPane splitPane1;
    private JSplitPane splitPane2;
    private JPanel panel4;
    private JPanel filterPanel;
    private JLabel label1;
    private JTextField classFilterField;
    private JButton filterResetButton;
    private ConfigSelector configurableTree;
    private JSplitPane splitPane3;
    private PropertyEditorPanelNT propSheetPanel;
    private JPanel birdViewPanel;
    private JPanel scenePane;
    // JFormDesigner - End of variables declaration  //GEN-END:variables


    public static void main(String[] args) throws IOException, PropertyException {
        if (args.length == 1 && (args[0].equals("-help") || args[0].equals("-h") || args[0].startsWith("--h"))) {
            System.out.println(CONF_DESIGNER + " [-l <semicolon-separated list of jars or class-directories which " +
                    "contain configurables>] [-f <config-xml or config-project-files>]" +
                    "\n\n Note: The -l defines only which jars/locations to parse in order to find configurables. " +
                    "Nevertheless all these jars/directories need to be contained in the class-path of " + CONF_DESIGNER + ".");

            System.exit(0);
        }

        List<String> addtionalClasses = new ArrayList<String>();
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-l")) {
                assert args.length > i;
                addtionalClasses.addAll(Arrays.asList(args[i + 1].split(";")));
            }
        }

        // find the intial file to be loaded
        File preLoadFile = null;
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-f")) {
                assert args.length > i : "-f requires a file-argument";
                preLoadFile = new File(args[i + 1]);
                break;
            }
        }

        ConfDesigner gui = new ConfDesigner();

        if (preLoadFile != null && preLoadFile.isFile()) {
            if (preLoadFile.getName().endsWith(SessionManager.FORMAT_SUFFIX))
                gui.getSesMan().loadScene(preLoadFile);
            else {
                assert preLoadFile.getName().endsWith(SessionManager.PROJECT_FORMAT_SUFFIX);
                gui.getSesMan().loadProject(preLoadFile);
            }
        }

        gui.addConfigurables(ClassPathParser.getConfigurableClasses(addtionalClasses));

        gui.setBounds(getPrefs().getInt("mainwin.xpos", 100), getPrefs().getInt("mainwin.ypos", 100), getPrefs().getInt("mainwin.width", 900), getPrefs().getInt("mainwin.height", 700));

        gui.setVisible(true);
    }


    public synchronized static Clipboard getClipBoard() {
        if (clipboard == null) {
            clipboard = new Clipboard("myClip");
        }

        return clipboard;
    }
}
