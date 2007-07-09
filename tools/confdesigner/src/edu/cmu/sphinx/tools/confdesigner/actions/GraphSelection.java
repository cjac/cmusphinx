package edu.cmu.sphinx.tools.confdesigner.actions;

import edu.cmu.sphinx.util.props.ConfigurationManager;

import java.awt.*;
import java.awt.datatransfer.*;
import java.io.IOException;
import java.util.Map;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
class GraphSelection implements Transferable, ClipboardOwner {

    private ConfigurationManager subGraphCM;
    private Map<String, Point> nodeLocations;


    public GraphSelection(ConfigurationManager subGraphCM, Map<String, Point> nodeLocations) {
        this.nodeLocations = nodeLocations;
        assert subGraphCM != null;
        this.subGraphCM = subGraphCM;
    }


    public DataFlavor[] getTransferDataFlavors() {
        return new DataFlavor[0];
    }


    public boolean isDataFlavorSupported(DataFlavor flavor) {
        return false;
    }


    public Object getTransferData(DataFlavor flavor) throws UnsupportedFlavorException, IOException {
        return null;
    }


    public void lostOwnership(Clipboard clipboard, Transferable contents) {

    }


    public ConfigurationManager getSubGraphCM() {
        return subGraphCM;
    }


    public Map<String, Point> getNodeLocations() {
        return nodeLocations;
    }
}

