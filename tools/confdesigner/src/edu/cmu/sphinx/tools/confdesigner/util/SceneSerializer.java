package edu.cmu.sphinx.tools.confdesigner.util;

import com.thoughtworks.xstream.XStream;
import com.thoughtworks.xstream.io.xml.DomDriver;
import org.netbeans.api.visual.widget.Widget;

import java.awt.*;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import edu.cmu.sphinx.tools.confdesigner.ConfigScene;
import edu.cmu.sphinx.tools.confdesigner.ConfNode;

/**
 * Utility methods which allow scene layout persistence.
 *
 * @author Holger Brandl
 */
public class SceneSerializer {

    public static void saveLayout(ConfigScene scene, File file) {
        Collection<ConfNode> nodes = scene.getNodes();
        Map<String, Point> nodeLocations = new HashMap<String, Point>();

        for (ConfNode node : nodes) {
            Widget widget = scene.findWidget(node);
            Point location = widget.getPreferredLocation();
            nodeLocations.put(node.getInstanceName(), location);
        }

        XStream xStream = new XStream(new DomDriver());
        try {
            xStream.toXML(nodeLocations, new FileOutputStream(file));
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

            Map<String, Point> nodeLocations = (Map<String, Point>) xStream.fromXML(new FileInputStream(file));
            for (ConfNode node : nodes) {
                Widget widget = scene.findWidget(node);
                widget.setPreferredLocation(nodeLocations.get(node.getInstanceName()));
            }

            scene.revalidate();
            scene.validate();
            scene.revalidate();
            scene.validate();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }

}
