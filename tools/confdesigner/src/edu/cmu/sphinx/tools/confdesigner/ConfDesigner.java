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
import edu.cmu.sphinx.tools.confdesigner.util.SceneFinder;
import edu.cmu.sphinx.tools.executor.ExecutableExecutor;
import edu.cmu.sphinx.tools.executor.ExecutorListener;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import org.netbeans.api.visual.model.ObjectSceneEvent;
import org.netbeans.api.visual.model.ObjectSceneEventType;

import javax.swing.*;
import javax.swing.filechooser.FileFilter;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.io.IOException;
import java.util.*;
import java.util.List;


/** @author Holger Brandl */
public class ConfDesigner extends JFrame implements ExecutorListener {

    private SceneController sceneController;

    public static final String CONF_DESIGNER = "ConDesigner";

    public final String LAYOUT_SUFFIX = ".layout";
    public final String FORMAT_SUFFIX = ".sxl";

    private File cmLocation = null;
    private JDialog sateliteDialog;

    Map<PropertySheet, JMenuItem> curSceneExecutors = new HashMap<PropertySheet, JMenuItem>();


    public ConfDesigner() throws IOException, PropertyException {
        initComponents();

        ConfigScene scene = new ConfigScene();
        sceneController = new SceneController(scene);
        sceneController.addExecutorListener(this);

        configurableTree.confController = sceneController;

        scenePane.setViewportView(sceneController.getView());
        sceneController.getView().requestFocusInWindow();

        birdViewPanel.removeAll();
        birdViewPanel.add(scene.createSatelliteView());

        sceneController.setCm(cmLocation);
        propSheetPanel.setConfigurationManager(sceneController.getCm());

        scene.addObjectSceneListener(new ObjectSceneListenerAdapter() {

            public void selectionChanged(ObjectSceneEvent event, Set<Object> previousSelection, Set<Object> newSelection) {
                if (newSelection.size() != 1 && isVisible()) {
                    propSheetPanel.rebuildPanel(null);
                } else if (!newSelection.isEmpty()) {
                    Object o = newSelection.iterator().next();
                    if (o instanceof ConfNode) {
                        propSheetPanel.rebuildPanel(((ConfNode) o).getPropSheet());
                    }

                    sceneController.getView().requestFocusInWindow();
                }
            }
        }, ObjectSceneEventType.OBJECT_SELECTION_CHANGED);

//        cmLocation = new File("lala.sxl");
//        cmLocation = new File("../../sphinx4/src/demo/sphinx/hellongram/hellongram.config.xml");
        sceneController.setCm(cmLocation);
        propSheetPanel.setConfigurationManager(sceneController.getCm());

        addWindowListener(new WindowAdapter() {

            public void windowClosed(WindowEvent e) {
                System.exit(0);
            }
        });

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
    }


    public void addConfigurables(Collection<Class<? extends Configurable>> configClasses) {
        configurableTree.addConfigurables(configClasses);
    }


    private void addDummyNodes(ConfigScene scene) throws IOException, PropertyException {
//        ConfigurationManager cm = new ConfigurationManager(Utils.getURL(Azubi.DEFAULT_CONFIG_XML));

        sceneController.addNode(ThreadedAcousticScorer.class, null);
        sceneController.addNode(FrontEnd.class, null);
        sceneController.addNode(SimpleBreadthFirstSearchManager.class, null);
        sceneController.addNode(Decoder.class, null);
        sceneController.addNode(WavWriter.class, null);
        sceneController.addNode(DiscreteCosineTransform.class, null);

        scene.validate();
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


    private void saveItemActionPerformed() {
        if (cmLocation == null) {
            saveAsItemActionPerformed();
        } else {
            if (!cmLocation.getName().endsWith(FORMAT_SUFFIX))
                cmLocation = new File(cmLocation.getAbsolutePath() + FORMAT_SUFFIX);

            sceneController.save(cmLocation);
        }
    }


    private void saveAsItemActionPerformed() {
        JFileChooser jfc = new JFileChooser(new File("."));
        jfc.setMultiSelectionEnabled(false);
        jfc.setFileFilter(new FileFilter() {

            public boolean accept(File f) {
                return (f.getName().endsWith(FORMAT_SUFFIX) && f.isFile()) || f.isDirectory();
            }


            public String getDescription() {
                return "s4 configuration files (*" + FORMAT_SUFFIX + ")";
            }
        });

        int status = jfc.showSaveDialog(this);
        if (status == JFileChooser.APPROVE_OPTION) {
            cmLocation = jfc.getSelectedFile();
            saveItemActionPerformed();
        }
    }


    private void loadItemActionPerformed() {
        JFileChooser jfc = new JFileChooser(new File("."));
        jfc.setMultiSelectionEnabled(false);
        jfc.setFileFilter(new FileFilter() {

            public boolean accept(File f) {
                return (f.getName().endsWith(FORMAT_SUFFIX) && f.isFile()) || f.isDirectory();
            }


            public String getDescription() {
                return "s4 configuration files (*" + FORMAT_SUFFIX + ")";
            }
        });

        int status = jfc.showOpenDialog(this);
        if (status == JFileChooser.APPROVE_OPTION) {
            cmLocation = jfc.getSelectedFile();
            sceneController.setCm(cmLocation);


            propSheetPanel.setConfigurationManager(sceneController.getCm());
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
        if (sceneController.hasUnsavedChanges()) {
            int status = JOptionPane.showConfirmDialog(this, "There were unsaved changes. Do you want to save them ?", "Current configuration changed", JOptionPane.QUESTION_MESSAGE);
            switch (status) {
                case JOptionPane.YES_OPTION:
                    saveItemActionPerformed();
                    break;
                case JOptionPane.NO_OPTION:
                    // do nothing
                    break;
                case JOptionPane.CANCEL_OPTION:
                    return;
            }
        }

        sceneController.setCm(new ConfigurationManager());
        propSheetPanel.setConfigurationManager(sceneController.getCm());
    }


    private void showBirdViewItemActionPerformed(ActionEvent e) {
        if (sateliteDialog == null) {
            sateliteDialog = new JDialog(this);
            sateliteDialog.setBounds(50, 500, 200, 150);
            JPanel panel = new JPanel(new BorderLayout());
            panel.add(sceneController.getScene().createSatelliteView());

            sateliteDialog.getContentPane().add(panel);
        }

//        showBirdViewItem.setSelected(!showBirdViewItem.isSelected());

        sateliteDialog.setVisible(showBirdViewItem.isSelected());
    }


    private void layoutGraphItemActionPerformed() {
        sceneController.getScene().layoutScene();
    }


    private void aboutItemActionPerformed() {
        JOptionPane.showMessageDialog(this, "ConfDesigner 1.0 beta 1", "About", JOptionPane.INFORMATION_MESSAGE);
    }


    private void helpItemActionPerformed() {
    }


    private void sceneFinderActionPerformed() {
        boolean searchSucessful = new SceneFinder(this, sceneController).process(sceneFinder.getText());

        if (searchSucessful)
            sceneFinder.setText("");
    }


    private void initComponents() {
        // JFormDesigner - Component initialization - DO NOT MODIFY  //GEN-BEGIN:initComponents
        // Generated using JFormDesigner Open Source Project license - Sphinx-4 (cmusphinx.sourceforge.net/sphinx4/)
        menuBar1 = new JMenuBar();
        menu1 = new JMenu();
        newItem = new JMenuItem();
        saveItem = new JMenuItem();
        saveAsItem = new JMenuItem();
        loadItem = new JMenuItem();
        menuItem3 = new JMenuItem();
        menu2 = new JMenu();
        layoutGraphItem = new JMenuItem();
        menu3 = new JMenu();
        showBirdViewItem = new JCheckBoxMenuItem();
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
        scrollPane2 = new JScrollPane();
        configurableTree = new ConfigurableTree();
        splitPane3 = new JSplitPane();
        propSheetPanel = new ConfigurablePropPanel();
        birdViewPanel = new JPanel();
        scenePane = new JScrollPane();

        //======== this ========
        setTitle("ConfDesigner");
        Container contentPane = getContentPane();
        contentPane.setLayout(new BorderLayout());

        //======== menuBar1 ========
        {

            //======== menu1 ========
            {
                menu1.setText("File");

                //---- newItem ----
                newItem.setText("New");
                newItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
                newItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        newItemActionPerformed();
                    }
                });
                menu1.add(newItem);

                //---- saveItem ----
                saveItem.setText("Save");
                saveItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
                saveItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        saveItemActionPerformed();
                    }
                });
                menu1.add(saveItem);

                //---- saveAsItem ----
                saveAsItem.setText("Save As...");
                saveAsItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask() | KeyEvent.SHIFT_MASK));
                saveAsItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        saveAsItemActionPerformed();
                    }
                });
                menu1.add(saveAsItem);

                //---- loadItem ----
                loadItem.setText("Load...");
                loadItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_L, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
                loadItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        loadItemActionPerformed();
                    }
                });
                menu1.add(loadItem);
                menu1.addSeparator();

                //---- menuItem3 ----
                menuItem3.setText("Exit");
                menuItem3.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Q, Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()));
                menu1.add(menuItem3);
            }
            menuBar1.add(menu1);

            //======== menu2 ========
            {
                menu2.setText("Edit");

                //---- layoutGraphItem ----
                layoutGraphItem.setText("Relayout Graph");
                layoutGraphItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_L, KeyEvent.ALT_MASK));
                layoutGraphItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        layoutGraphItemActionPerformed();
                    }
                });
                menu2.add(layoutGraphItem);
            }
            menuBar1.add(menu2);

            //======== menu3 ========
            {
                menu3.setText("View");

                //---- showBirdViewItem ----
                showBirdViewItem.setText("Show Birdview");
                showBirdViewItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_B, KeyEvent.CTRL_MASK));
                showBirdViewItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        showBirdViewItemActionPerformed(e);
                    }
                });
                menu3.add(showBirdViewItem);
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
                helpItem.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        helpItemActionPerformed();
                    }
                });
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

                    //======== scrollPane2 ========
                    {
                        scrollPane2.setMinimumSize(null);
                        scrollPane2.setMaximumSize(null);

                        //---- configurableTree ----
                        configurableTree.setMaximumSize(new Dimension(2111, 2111));
                        configurableTree.setMinimumSize(new Dimension(100, 200));
                        configurableTree.setPreferredSize(null);
                        scrollPane2.setViewportView(configurableTree);
                    }
                    panel4.add(scrollPane2, BorderLayout.CENTER);
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
    private JMenuItem newItem;
    private JMenuItem saveItem;
    private JMenuItem saveAsItem;
    private JMenuItem loadItem;
    private JMenuItem menuItem3;
    private JMenu menu2;
    private JMenuItem layoutGraphItem;
    private JMenu menu3;
    private JCheckBoxMenuItem showBirdViewItem;
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
    private JScrollPane scrollPane2;
    private ConfigurableTree configurableTree;
    private JSplitPane splitPane3;
    private ConfigurablePropPanel propSheetPanel;
    private JPanel birdViewPanel;
    private JScrollPane scenePane;
    // JFormDesigner - End of variables declaration  //GEN-END:variables


    public static void main(String[] args) throws IOException, PropertyException {
        if (args.length == 1 && (args[0].equals("-help") || args[0].equals("-h") || args[0].startsWith("--h"))) {
            System.out.println(CONF_DESIGNER + " [-l <semicolon-separated list of jars or class-directories which " +
                    "contain configurables>] " +
                    "\n\n Note: The -l defines only which jars/locations to parse in order to find configurables. " +
                    "Nevertheless all these jars/directories need to be contained in the class-path of " + CONF_DESIGNER + ".");

            System.exit(0);
        }

        List<String> addtionalClasses = new ArrayList<String>();
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-l")) {
                assert args.length > i;
                addtionalClasses.addAll(Arrays.asList(args[i + 1].split(",")));
            }
        }

        ConfDesigner gui = new ConfDesigner();

        gui.addConfigurables(ClassPathParser.getConfigurableClasses(addtionalClasses));

        gui.setBounds(100, 100, 900, 700);
        gui.setVisible(true);
    }
}
