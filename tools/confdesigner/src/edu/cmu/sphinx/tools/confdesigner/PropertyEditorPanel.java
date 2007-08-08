package edu.cmu.sphinx.tools.confdesigner;

import com.l2fprod.common.propertysheet.DefaultProperty;
import edu.cmu.sphinx.tools.confdesigner.propedit.*;
import edu.cmu.sphinx.util.props.*;

import javax.swing.*;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.TableModel;
import java.awt.*;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.lang.reflect.Proxy;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

/**
 * A self-rebuilding gui which allows to edit simple property-types (int, double, booelean, String).
 *
 * @author Holger Brandl
 */
public class PropertyEditorPanel extends JPanel implements TableModelListener {

    ConfigurationManager cm;
    PropertySheet currentPS;
    private static final String NEW_GLOBAL_PROP = "Create new global property ... ";

    boolean isShowingGlobalProps = true;

    private SimplePropEditor propEditor;


    public PropertyEditorPanel() {

        propEditor = new SimplePropEditor();
        propEditor.getTable().getModel().addTableModelListener(this);

        setLayout(new BorderLayout());
        add(propEditor);
//        addKeyListener(new KeyAdapter() {
//
//            public void keyPressed(KeyEvent e) {
//                if (e.getKeyCode() == KeyEvent.VK_DELETE) {
//                    int delRow = getTable().getSelectedRow();
//                    getTable().removeRowSelectionInterval(delRow, delRow + 1);
//                }
//            }
//        });
    }


    public void tableChanged(TableModelEvent e) {
        int row = e.getFirstRow();
        int column = e.getColumn();
        TableModel model = (TableModel) e.getSource();
        String columnName = model.getColumnName(column);
//        Object data = model.getValueAt(row, column);

        System.out.println("table model changed");

        // Do something with the data...
    }


    // todo add the appropriate javadoc of the underlying s4property as tooltip to all generated prop-fields
    public void rebuildPanel(PropertySheet ps) {

        propEditor.clear();
        currentPS = ps;

        // show the default properties if nothing is selected
        if (ps == null) {
            createGlobalPropsPanel();
            return;
        }

        isShowingGlobalProps = false;
        PropertyChangeListener pChangeListener = createPropSheetListener();

        TableProperty p = null, type;
        try {

            for (String propName : ps.getRegisteredProperties()) {
                Proxy wrapper = null;

                PropertySheet.PropertyType propType = currentPS.getType(propName);
                switch (propType) {

                    case COMP:
                        S4Component s4Component = ((S4Component) currentPS.getProperty(propName, S4Component.class).getAnnotation());
//                        p = new TableCompProperty(currentPS, propName, s4Component);

//                        p = new DefaultProperty();
//                        p.setDisplayName(propName);
//                        type = new DefaultProperty();
//                        type.setDisplayName("type");
//                        type.setValue(s4Component.type().getName());
//                        p.addSubProperty(type);
//
//                        if (!s4Component.defaultClass().equals(Configurable.class)) {
//                            DefaultProperty defClassProperty = new DefaultProperty();
//                            defClassProperty.setDisplayName("default type");
//                            defClassProperty.setValue(s4Component.defaultClass().getName());
//                            p.addSubProperty(defClassProperty);
//                        }
                        break;
                    case COMPLIST:
                        S4ComponentList s4CompList = ((S4ComponentList) currentPS.getProperty(propName, S4ComponentList.class).getAnnotation());
//
//                        p = new DefaultProperty();
//                        p.setDisplayName(propName);
//                        type = new DefaultProperty();
//                        type.setDisplayName("type");
//                        type.setValue(s4CompList.type().getName());
//                        p.addSubProperty(type);
                        break;
                    case BOOL:
                        S4Boolean s4bool = ((S4Boolean) currentPS.getProperty(propName, S4Boolean.class).getAnnotation());

                        p = new TableBoolProperty(currentPS, propName, s4bool);

//                        p = new DefaultProperty();
//                        p.setDisplayName(propName);
//                        // todo support that
////                    if (s4bool.isNotDefined())
////                        System.err.println("non-defaulting booleans are not supported");
//
//                        p.setType(Boolean.class);
//                        if (currentPS.getRaw(propName) != null)
//                            p.setValue(currentPS.getBoolean(propName));
//                        p.setEditable(true);
                        break;
                    case DOUBLE:
                        S4Double s4Double = ((S4Double) currentPS.getProperty(propName, S4Double.class).getAnnotation());

                        p = new TableDoubleProperty(currentPS, propName, s4Double);
                        break;
                    case INT:
                        S4Integer s4Integer = ((S4Integer) wrapper);

//                        p = new DefaultProperty();
//                        p.setDisplayName(propName);
//
//                        p.setType(Integer.class);
//                        if (currentPS.getRaw(propName) != null)
//                            p.setValue(currentPS.getInt(propName));
//                        p.setEditable(true);
//
//                        // todo check whether the property is in range
                        break;
                    case STRING:
                        S4String s4string = ((S4String) currentPS.getProperty(propName, S4String.class).getAnnotation());
                        p = new TableStringProperty(currentPS, propName, s4string);

//                        p = new DefaultProperty();
//                        p.setDisplayName(propName);
//
//                        p.setType(String.class);
//                        if (currentPS.getRaw(propName) != null)
//                            p.setValue(currentPS.getString(propName));
//                        p.setEditable(true);
                }

                //todo uncomment me
//                p.addPropertyChangeListener(pChangeListener);
                if (p != null)
                    propEditor.addProperty(p);
            }
        } catch (PropertyException e) {
            e.printStackTrace();
        }
    }


    private PropertyChangeListener createPropSheetListener() {
        return new PropertyChangeListener() {

            public void propertyChange(PropertyChangeEvent evt) {
                try {
                    Object sourceProperty = evt.getSource();

                    DefaultProperty p = (DefaultProperty) sourceProperty;
                    String propName = p.getDisplayName();

                    Class propType = p.getType();
                    if (propType.equals(Boolean.class)) {
                        currentPS.setBoolean(propName, (Boolean) p.getValue());

                    } else if (propType.equals(Integer.class)) {
                        Integer newValue = (Integer) p.getValue();

                        int[] range = ((S4Integer) currentPS.getProperty(propName, S4Integer.class).getAnnotation()).range();
                        if (newValue < range[0] || newValue > range[1]) {
                            JOptionPane.showConfirmDialog(null, "Property '" + propName + "' is not in range: (" + range[0] + ", " + range[1] + ")");
                            clear(p);
                        } else
                            currentPS.setInt(propName, newValue);

                    } else if (propType.equals(Double.class)) {
                        Double newValue = (Double) p.getValue();

                        double[] range = ((S4Double) currentPS.getProperty(propName, S4Double.class).getAnnotation()).range();
                        if (newValue < range[0] || newValue > range[1]) {
                            JOptionPane.showConfirmDialog(null, "Property '" + propName + "' is not in range (" + range[0] + ", " + range[1] + ")");
                            clear(p);
                        } else
                            currentPS.setDouble(propName, newValue);

                    } else if (propType.equals(String.class)) {
                        String newValue = (String) p.getValue();

                        List<String> range = Arrays.asList(((S4String) currentPS.getProperty(propName, S4String.class).getAnnotation()).range());
                        if (!range.isEmpty() && range.contains(newValue)) {
                            JOptionPane.showConfirmDialog(null, "Property '" + propName + "' is not in range: " + range);
                            clear(p);
                        } else
                            currentPS.setString(propName, newValue);
                    }

                } catch (PropertyException pe) {
                    System.err.println(pe.getStackTrace());
                }
            }
        };
    }


    private void clear(DefaultProperty p) {
        try {
            p.setValue(null);
        } catch (Exception e) {
        }
    }


    private void createGlobalPropsPanel() {

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
//                    addProperty(newProp);
                }

                if (p.getValue().equals("")) {
                    if (!p.getDisplayName().equals(NEW_GLOBAL_PROP)) {
                        int status = JOptionPane.showConfirmDialog(null, "Do you want to delete the empty property '" + p.getDisplayName() + "' ?");
                        if (status == JOptionPane.YES_OPTION) {
                            cm.setGlobalProperty(p.getDisplayName(), null);
//                            removeProperty(p);
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

        Map<String, String> properties = cm.getGlobalProperties();
        for (String globPropName : properties.keySet()) {
            DefaultProperty p = new DefaultProperty();
            p.setType(String.class);
            p.setDisplayName(globPropName);
            p.setValue(properties.get(globPropName));
            p.setEditable(true);

//            addProperty(p);
            p.addPropertyChangeListener(pChangeListener);
        }

        // add a generic property which allows to add new global properties
        DefaultProperty p = new DefaultProperty();
        p.setDisplayName(NEW_GLOBAL_PROP);
        p.setName(NEW_GLOBAL_PROP);
        p.setType(String.class);
        p.setEditable(true);
        p.addPropertyChangeListener(pChangeListener);
//        addProperty(p);
    }


    public void setConfigurationManager(ConfigurationManager cm) {
        assert cm != null;
        this.cm = cm;

        rebuildPanel(null);
    }
}
