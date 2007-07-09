package edu.cmu.sphinx.tools.confdesigner.actions;

import edu.cmu.sphinx.tools.confdesigner.*;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.ConfigurationManagerUtils;
import org.netbeans.api.visual.widget.Widget;

import javax.swing.*;
import java.awt.*;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.FlavorEvent;
import java.awt.datatransfer.FlavorListener;
import java.awt.datatransfer.Transferable;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.util.*;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class PasteSubGraphAction extends AbstractAction implements FlavorListener {

    private SessionManager sesMan;
    private Clipboard clipBoard;


    public PasteSubGraphAction(SessionManager sesMan, Clipboard clipBoard) {
        this.sesMan = sesMan;
        this.clipBoard = clipBoard;

        putValue(NAME, "Paste");
        putValue(ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_V, KeyEvent.CTRL_MASK));
        putValue(MNEMONIC_KEY, KeyEvent.VK_V);

        setEnabled(clipBoard.getContents(null) != null);

        clipBoard.addFlavorListener(this);
    }


    public void actionPerformed(ActionEvent e) {

        Transferable contents = clipBoard.getContents(null);
        if (!(contents instanceof GraphSelection))
            return;

        GraphSelection selection = (GraphSelection) contents;

        //clone the selection
        try {
            selection = new GraphSelection((ConfigurationManager) selection.getSubGraphCM().clone(), selection.getNodeLocations());
        } catch (CloneNotSupportedException e1) {
            e1.printStackTrace();
        }

        SceneContext activeContext = sesMan.getActiveScene();
        SceneController controller = activeContext.getSceneController();

        GraphSelection fixedSelection = fixNameDoubles(controller.getCm(), selection);
        Map<String, Point> nodeLocations = fixedSelection.getNodeLocations();


        new GraphLoader(controller).mergeIntoScene(fixedSelection.getSubGraphCM());

        // determine most left border of locations

        Comparator<Point> pointComp = new Comparator<Point>() {
            public int compare(Point o1, Point o2) {
                return o1.getX() < o2.getX() ? -1 : 1;
            }
        };
        int mostRightPoint = (int) Collections.max(fixedSelection.getNodeLocations().values(), pointComp).getX();
        int mostLeftPoint = (int) Collections.min(fixedSelection.getNodeLocations().values(), pointComp).getX();
        Point offset = new Point(mostRightPoint - mostLeftPoint + 150, 150);

        // set the node locations of the newly added nodes
        HashSet<ConfNode> addedNodes = new HashSet<ConfNode>();
        ConfigScene scene = controller.getScene();

        for (String addNodeName : fixedSelection.getNodeLocations().keySet()) {
            Widget widget = scene.findWidgetByName(addNodeName);
            addedNodes.add(scene.findNodeByName(addNodeName));

            Point nodeLoc = nodeLocations.get(addNodeName).getLocation();

            nodeLoc.translate((int) offset.getX(), (int) offset.getY());
            widget.setPreferredLocation(new Point(nodeLoc));
        }

        controller.getScene().validate();

        //select inserted nodes
        controller.getScene().userSelectionSuggested(addedNodes, false);
    }


    /** Rename all component in order to allow a configuration manager fusion. */
    private GraphSelection fixNameDoubles(ConfigurationManager cm, GraphSelection selection) {
        Map<String, String> renames = ConfigurationManagerUtils.fixDuplicateNames(cm, selection.getSubGraphCM());

        Map<String, Point> renamedLocations = new HashMap<String, Point>();
        renamedLocations.putAll(selection.getNodeLocations());

        for (String oldName : renames.keySet()) {
            Point point = renamedLocations.remove(oldName);
            renamedLocations.put(renames.get(oldName), point);
        }

        return new GraphSelection(selection.getSubGraphCM(), renamedLocations);
    }


    public void flavorsChanged(FlavorEvent e) {
        setEnabled(clipBoard.getContents(null) != null);
    }
}
