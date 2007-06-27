package edu.cmu.sphinx.tools.confdesigner.actions;

import edu.cmu.sphinx.tools.confdesigner.ConfigScene;
import edu.cmu.sphinx.tools.confdesigner.SessionManager;
import sun.reflect.generics.reflectiveObjects.NotImplementedException;

import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.color.ColorSpace;
import java.awt.event.ActionEvent;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

/**
 * Exports a given scene or just the current viewport of a scene into an image file.
 *
 * @author Holger Brandl
 */
public class ExportImageAction extends AbstractAction {

    private SessionManager sesMan;
    private boolean doExportViewOnly;


    public ExportImageAction(SessionManager sesMan, boolean doExportViewOnly) {
        this.sesMan = sesMan;
        this.doExportViewOnly = doExportViewOnly;

        if (doExportViewOnly) {
            putValue(NAME, "Export View Image");
        } else {
            putValue(NAME, "Export Scene Image");
        }
    }


    public void actionPerformed(ActionEvent e) {
        ConfigScene scene = sesMan.getActiveScene().getScene();

        if (doExportViewOnly) {
            throw new NotImplementedException();
        } else {
            Rectangle bounds = scene.getBounds();
            BufferedImage bufIm = new BufferedImage((int) bounds.getWidth(), (int) bounds.getHeight(), ColorSpace.TYPE_RGB);

            JScrollPane jsp = (JScrollPane)
                    SwingUtilities.getAncestorOfClass(JScrollPane.class, scene.getView());
            SwingUtilities.paintComponent(bufIm.createGraphics(), scene.getView(), jsp, 0, 0, (int) bounds.getWidth(), (int) bounds.getHeight());
            scene.validate();

            writeImage(bufIm, "test.png");
        }
    }


    /**
     * Write a BufferedImage to a File.
     *
     * @param image    image to be written
     * @param fileName name of file to be created
     * @throws IOException if an error occurs during writing
     */
    public static void writeImage(BufferedImage image, String fileName) {
        if (fileName == null)
            return;

        if (!fileName.endsWith(".png"))
            fileName += ".png";

        try {
            ImageIO.write(image, "PNG", new File(fileName));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
