package edu.cmu.sphinx.tools.confdesigner.propedit;

import edu.cmu.sphinx.util.props.ConfigurationChangeListener;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.test.DummyComp;

import javax.swing.*;
import java.awt.*;
import java.lang.reflect.Proxy;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class SimplePropEditor extends JPanel {

    private JTable myTable;
    private EditorTableModel tableModel;

    private JSplitPane split;
    private JScrollPane descriptionScrollPane;
    private JEditorPane descriptionPanel;


    public SimplePropEditor() {
        tableModel = new EditorTableModel();
        myTable = new JTable(tableModel);
        myTable.getTableHeader().getColumnModel().getColumn(0).setHeaderValue("Name");
        myTable.getTableHeader().getColumnModel().getColumn(1).setHeaderValue("Value");
        myTable = new CellSpecificRenderTable(this, tableModel);

        myTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

        descriptionScrollPane = new JScrollPane();
        descriptionPanel = new JEditorPane();
        descriptionScrollPane.setViewportView(descriptionPanel);

        JScrollPane tableScrolPane = new JScrollPane();
        tableScrolPane.setViewportView(myTable);

        split = new JSplitPane();
        split.setOrientation(JSplitPane.VERTICAL_SPLIT);
        split.add(tableScrolPane, JSplitPane.TOP);
        split.add(descriptionPanel, JSplitPane.BOTTOM);
        split.setDividerLocation(0.9);
        split.setDividerSize(3);
        split.setResizeWeight(0.9);

        setLayout(new BorderLayout());
        add(split);
    }


    public void clear() {
        tableModel.clear();
    }


    public void addProperty(TableProperty p) {
        tableModel.addProperty(p);
//        myTable.validate();
    }


    public TableProperty getProperty(int row) {
        return tableModel.getProperty(row);
    }


    public JTable getTable() {
        return myTable;
    }


    public static void main(String[] args) {
        JFrame frame = new JFrame("property editor");


        ConfigurationManager cm = new ConfigurationManager();
        cm.addConfigurationChangeListener(new ConfigurationChangeListener() {

            public void configurationChanged(String configurableName, String propertyName, ConfigurationManager cm) {
                System.out.println("configuration changed: " + configurableName + " propName=" + propertyName + " value=" + cm.getPropertySheet(configurableName).getRaw(propertyName));
            }


            public void componentAdded(ConfigurationManager cm, PropertySheet ps) {
                System.out.println("component added");
            }


            public void componentRemoved(ConfigurationManager cm, PropertySheet ps) {
                System.out.println("component removed");
            }


            public void componentRenamed(ConfigurationManager cm, PropertySheet ps, String oldName) {
                System.out.println("component compoenent removed");
            }
        });
        String instName = "myLogMath";
        cm.addConfigurable(DummyComp.class, instName);
        PropertySheet ps = cm.getPropertySheet(instName);

        SimplePropEditor editor = new SimplePropEditor();
//        editor.addProperty(new TableDoubleProperty(ps, instName, (S4Double) ps.getProperty(LogMath.PROP_LOG_BASE, S4Double.class).getAnnotation()));
        editor.addProperty(new TableBoolProperty(editor.getTable(), ps, DummyComp.PROP_USE_FOOBAR));
        editor.addProperty(new TableStringProperty(editor.getTable(), ps, DummyComp.PROP_BEST_PIZZA));
        editor.addProperty(new TableStringProperty(editor.getTable(), ps, DummyComp.PROP_BEST_ASR));
        editor.addProperty(new TableDoubleProperty(editor.getTable(), ps, DummyComp.PROP_ALPHA));
        editor.addProperty(new TableIntegerProperty(editor.getTable(), ps, DummyComp.PROP_BEAM_WIDTH));
        frame.getContentPane().add(editor);

        frame.setBounds(300, 400, 200, 400);
        frame.setVisible(true);
    }


    public void addProperties(PropertySheet ps) {
        for (String propName : ps.getRegisteredProperties()) {
            Proxy wrapper = null;

            PropertySheet.PropertyType propType = ps.getType(propName);
            switch (propType) {


                case BOOL:
                    addProperty(new TableBoolProperty(getTable(), ps, propName));
                    break;
                case DOUBLE:
                    addProperty(new TableDoubleProperty(getTable(), ps, propName));
                    break;
                case INT:
                    addProperty(new TableIntegerProperty(getTable(), ps, propName));
                    break;
                case STRING:
                    addProperty(new TableStringProperty(getTable(), ps, propName));
                    break;
            }

//            p.addPropertyChangeListener(pChangeListener);
//            addProperty(p);
        }
    }
}
