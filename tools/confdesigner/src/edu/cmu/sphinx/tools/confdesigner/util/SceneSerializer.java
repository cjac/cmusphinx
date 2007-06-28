package edu.cmu.sphinx.tools.confdesigner.util;

import com.thoughtworks.xstream.XStream;
import com.thoughtworks.xstream.io.xml.DomDriver;
import edu.cmu.sphinx.tools.confdesigner.ConfNode;
import edu.cmu.sphinx.tools.confdesigner.ConfigScene;
import org.netbeans.api.visual.widget.LabelWidget;
import org.netbeans.api.visual.widget.Widget;

import java.awt.*;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * Utility methods which allow scene layout persistence.
 *
 * @author Holger Brandl
 */
public class SceneSerializer {

    private static final String NODE_LOCATIONS = "nodeLocations";
    private static final String BACK_LABELS = "backLabels";
    private static final String BACK_LABELS_COLORS = "backLabelsColors";


    public static void saveLayout(ConfigScene scene, File file) {
        Map<String, Object> sceneProps = new HashMap<String, Object>();

        Collection<ConfNode> nodes = scene.getNodes();
        Map<String, Point> nodeLocations = new HashMap<String, Point>();

        for (ConfNode node : nodes) {
            Widget widget = scene.findWidget(node);
            Point location = widget.getPreferredLocation();
            nodeLocations.put(node.getInstanceName(), location);
        }

        sceneProps.put(NODE_LOCATIONS, nodeLocations);

        HashMap<String, Rectangle> bckndLabels = new HashMap<String, Rectangle>();
        HashMap<String, Color> bckndLabelColors = new HashMap<String, Color>();
        for (LabelWidget labelWidget : scene.getBckndLabels()) {
            String labelText = labelWidget.getLabel();

            while (bckndLabels.containsKey(labelText))
                labelText += ".";

            Rectangle bounds = labelWidget.getBounds();
            Point location = labelWidget.getPreferredLocation();
            bckndLabels.put(labelText, new Rectangle(location, new Dimension((int) bounds.getWidth(), (int) bounds.getHeight())));
            bckndLabelColors.put(labelText, (Color) labelWidget.getBackground());
        }

        sceneProps.put(BACK_LABELS, bckndLabels);
        sceneProps.put(BACK_LABELS_COLORS, bckndLabelColors);

        XStream xStream = new XStream(new DomDriver());
        try {
            xStream.toXML(sceneProps, new FileOutputStream(file));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }


    public static void loadLayout(ConfigScene scene, File file) {
        if (!file.isFile())
            return;
        XStream xStream = new XStream(new DomDriver());
        try {
            Collection<ConfNode> nodes = scene.getNodes();

            Map<String, Object> sceneProps = (Map<String, Object>) xStream.fromXML(new FileInputStream(file));

            Map<String, Point> nodeLocations = (Map<String, Point>) sceneProps.get(NODE_LOCATIONS);

            for (ConfNode node : nodes) {
                Widget widget = scene.findWidget(node);
                widget.setPreferredLocation(nodeLocations.get(node.getInstanceName()));
            }

            HashMap<String, Rectangle> bckndLabels = (HashMap<String, Rectangle>) sceneProps.get(BACK_LABELS);
            HashMap<String, Color> bckndLabelColors = (HashMap<String, Color>) sceneProps.get(BACK_LABELS_COLORS);
            if (bckndLabels != null) {
                for (String backLabel : bckndLabels.keySet()) {
                    Rectangle bounds = bckndLabels.get(backLabel);
                    Color color = bckndLabelColors.get(backLabel);
                    scene.addBckndLabel(backLabel, bounds.getLocation(), new Dimension((int) bounds.getWidth(), (int) bounds.getHeight()), color);
                }
            }

            scene.revalidate();
            scene.validate();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }

}
