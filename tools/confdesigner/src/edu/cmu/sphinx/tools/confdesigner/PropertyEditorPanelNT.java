package edu.cmu.sphinx.tools.confdesigner;

import com.l2fprod.common.propertysheet.DefaultProperty;
import com.l2fprod.common.propertysheet.PropertySheetPanel;
import edu.cmu.sphinx.tools.confdesigner.propedit.SimplePropEditor;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.GlobalProperties;
import org.netbeans.api.visual.model.ObjectSceneEvent;
import org.netbeans.api.visual.model.ObjectState;

import javax.swing.*;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Map;
import java.util.Set;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class PropertyEditorPanelNT extends PropertyEditorPanel {


    ConfigurationManager cm;
    PropertySheet currentPS;
    private static final String NEW_GLOBAL_PROP = "Create new global property ... ";


    SimplePropEditor propEditor = new SimplePropEditor();
    GlobalPropsPanel globPropsPanel;

    boolean isShowingGlobalProps = true;


    public PropertyEditorPanelNT() {
        addKeyListener(new KeyAdapter() {

            public void keyPressed(KeyEvent e) {
                if (e.getKeyCode() == KeyEvent.VK_DELETE) {
                    int delRow = getTable().getSelectedRow();
                    getTable().removeRowSelectionInterval(delRow, delRow + 1);
                }
            }
        });
    }


    // todo add the appropriate javadoc of the underlying s4property as tooltip to all generated prop-fields
    public void rebuildPanel(PropertySheet ps) {
        currentPS = ps;

        removeAll();

        // show the default properties if nothing is selected
        if (ps == null) {

            add(globPropsPanel);
            globPropsPanel.invalidate();
        } else {
            isShowingGlobalProps = false;

            propEditor.clear();
            propEditor.addProperties(ps);

            add(propEditor);

            PropertyChangeListener pChangeListener = createPropSheetListener();
        }

        invalidate();
        validateTree();
    }


    class GlobalPropsPanel extends PropertySheetPanel {


        public GlobalPropsPanel() {
            createGlobalPropsPanel();
        }


        public void createGlobalPropsPanel() {

            DefaultProperty globProperty = new DefaultProperty();
            globProperty.setDisplayName("global Properties");

            PropertyChangeListener pChangeListener = new PropertyChangeListener() {

                public void propertyChange(PropertyChangeEvent evt) {
                    DefaultProperty p = (DefaultProperty) evt.getSource();
                    if (p.getDisplayName().equals(NEW_GLOBAL_PROP) && !p.getValue().equals("")) {
                        String name = null;
                        while (name == null || name.equals(""))
                            name = JOptionPane.showInputDialog("Please enter the name of the property");

                        p.setName(name);
                        p.setDisplayName(name);
                        repaint();

                        DefaultProperty newProp = new DefaultProperty();
                        newProp.setDisplayName(NEW_GLOBAL_PROP);
                        newProp.setName(NEW_GLOBAL_PROP);
                        newProp.setType(String.class);
                        newProp.setEditable(true);
                        newProp.addPropertyChangeListener(this);
                        addProperty(newProp);
                    }

                    if (p.getValue().equals("")) {
                        if (!p.getDisplayName().equals(NEW_GLOBAL_PROP)) {
                            int status = JOptionPane.showConfirmDialog(null, "Do you want to delete the empty property '" + p.getDisplayName() + "' ?");
                            if (status == JOptionPane.YES_OPTION) {
                                cm.setGlobalProperty(p.getDisplayName(), null);
                                removeProperty(p);
                            }
                        }
                    } else {
                        String globalPropName = p.getDisplayName();
                        String newValue = (String) p.getValue();

                        cm.setGlobalProperty(globalPropName, newValue);
                    }

                    isShowingGlobalProps = true;
                }
            };

            GlobalProperties properties = cm.getGlobalProperties();
            for (String globPropName : properties.keySet()) {
                DefaultProperty p = new DefaultProperty();
                p.setType(String.class);
                p.setDisplayName(globPropName);
                p.setValue(properties.get(globPropName));
                p.setEditable(true);

                addProperty(p);
                p.addPropertyChangeListener(pChangeListener);
            }

            // add a generic property which allows to add new global properties
            DefaultProperty p = new DefaultProperty();
            p.setDisplayName(NEW_GLOBAL_PROP);
            p.setName(NEW_GLOBAL_PROP);
            p.setType(String.class);
            p.setEditable(true);
            p.addPropertyChangeListener(pChangeListener);
            addProperty(p);
        }
    }


    public void setConfigurationManager(ConfigurationManager cm) {
        assert cm != null;

        this.cm = cm;
//        if (currentPS == null)

        globPropsPanel = new GlobalPropsPanel();
        rebuildPanel(null);
    }


    public void objectAdded(ObjectSceneEvent event, Object addedObject) {
        System.err.println("object added(" + addedObject + ")");
    }


    public void objectRemoved(ObjectSceneEvent event, Object removedObject) {
        System.err.println("object removed (" + removedObject + ")");
    }


    public void objectStateChanged(ObjectSceneEvent event, Object changedObject, ObjectState previousState, ObjectState newState) {
        System.err.println("object state added (" + newState + ")\"");

    }


    public void selectionChanged(ObjectSceneEvent event, Set<Object> previousSelection, Set<Object> newSelection) {
        System.err.println("selection changed (" + newSelection + ")");
    }


    public void highlightingChanged(ObjectSceneEvent event, Set<Object> previousHighlighting, Set<Object> newHighlighting) {

    }


    public void hoverChanged(ObjectSceneEvent event, Object previousHoveredObject, Object newHoveredObject) {
        System.err.println("hover changed (new hover object =" + newHoveredObject + ")");
    }


    public void focusChanged(ObjectSceneEvent event, Object previousFocusedObject, Object newFocusedObject) {

    }


}
