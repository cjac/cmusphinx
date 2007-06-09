package edu.cmu.sphinx.tools.confdesigner;

import org.openide.util.Utilities;

import java.awt.*;
import java.awt.image.BufferedImage;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class ConfigSceneUtils {

    public static void initGrids(ConfigScene scene){
       Image sourceImage = Utilities.loadImage("test/resources/paper_grid17.png"); // NOI18N
       int width = sourceImage.getWidth(null);
       int height = sourceImage.getHeight(null);
       BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
       Graphics2D graphics = image.createGraphics();
       graphics.drawImage(sourceImage, 0, 0, null);
       graphics.dispose();
       TexturePaint PAINT_BACKGROUND = new TexturePaint(image, new Rectangle(0, 0, width, height));
       scene.setBackground(PAINT_BACKGROUND);
       scene.repaint();
       scene.revalidate(false);
       scene.validate();
   }
}
