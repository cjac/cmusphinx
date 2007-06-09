/*
 * The contents of this file are subject to the terms of the Common Development
 * and Distribution License (the License). You may not use this file except in
 * compliance with the License.
 *
 * You can obtain a copy of the License at http://www.netbeans.org/cddl.html
 * or http://www.netbeans.org/cddl.txt.
 *
 * When distributing Covered Code, include this CDDL Header Notice in each file
 * and include the License file at http://www.netbeans.org/cddl.txt.
 * If applicable, add the following below the CDDL Header, with the fields
 * enclosed by brackets [] replaced by your own identifying information:
 * "Portions Copyrighted [year] [name of copyright owner]"
 *
 * The Original Software is NetBeans. The Initial Developer of the Original
 * Software is Sun Microsystems, Inc. Portions Copyright 1997-2006 Sun
 * Microsystems, Inc. All Rights Reserved.
 */
package edu.cmu.sphinx.tools.confdesigner;

import org.netbeans.api.visual.laf.LookFeel;
import org.netbeans.api.visual.layout.LayoutFactory;
import org.netbeans.api.visual.model.ObjectState;
import org.netbeans.api.visual.widget.ImageWidget;
import org.netbeans.api.visual.widget.LabelWidget;
import org.netbeans.api.visual.widget.Scene;
import org.netbeans.api.visual.widget.Widget;
import org.openide.ErrorManager;
import org.openide.util.Utilities;

import javax.swing.*;
import java.awt.*;

/**
 * A widget representing image. The origin of the widget is at its top-left corner.
 *
 * @author David Kaspar
 */
// TODO - alignment
public class PortWidget extends Widget {

    private Image image;
    private Image disabledImage;
    private int width, height;
    private boolean paintAsDisabled;

    private static final Image SHAPE_IMAGE = Utilities.loadImage("org/netbeans/modules/visual/resources/vmd-pin.png");

    private static final Color COLOR_NORMAL = new Color(0xBACDF0);
    private static final Color COLOR_HOVERED = Color.BLACK;
    private static final Color COLOR_SELECTED = new Color(0x748CC0);
    private static final Color COLOR_HIGHLIGHTED = new Color(49, 106, 197);

    private ImageWidget imageWidget;
    private LabelWidget labelWidget;


    /**
     * Creates an image widget.
     *
     * @param scene the scene
     * @param label the label
     */
    public PortWidget(Scene scene, String label) {
        super(scene);


        LookFeel lookFeel = getScene().getLookFeel();

        setLayout(LayoutFactory.createHorizontalFlowLayout(LayoutFactory.SerialAlignment.LEFT_TOP, 5));

        imageWidget = new ImageWidget(scene);
        setImage(SHAPE_IMAGE);
        addChild(imageWidget);

        labelWidget = new LabelWidget(scene);
        setLabel(label);

        labelWidget.setFont(scene.getDefaultFont().deriveFont(14.0f));
        addChild(labelWidget);

        setState(ObjectState.createNormal());
    }


    /**
     * Sets a label.
     *
     * @param label the label
     */
    public final void setLabel(String label) {
        labelWidget.setLabel(label);
    }


    /**
     * Returns the image widget part of the icon node widget.
     *
     * @return the image widget
     */
    public final ImageWidget getImageWidget() {
        return imageWidget;
    }


    /**
     * Returns the label widget part of the icon node widget.
     *
     * @return the label widget
     */
    public final LabelWidget getLabelWidget() {
        return labelWidget;
    }


    /**
     * Returns an image.
     *
     * @return the image
     */
    public Image getImage() {
        return image;
    }


    /**
     * Sets an image
     *
     * @param image the image
     */
    public void setImage(Image image) {
        if (this.image == image)
            return;
        int oldWidth = width;
        int oldHeight = height;

        this.image = image;
        this.disabledImage = null;
        width = image != null ? image.getWidth(null) : 0;
        height = image != null ? image.getHeight(null) : 0;

        if (oldWidth == width && oldHeight == height)
            repaint();
        else
            revalidate();
    }


    /**
     * Returns whether the label is painted as disabled.
     *
     * @return true, if the label is painted as disabled
     */
    public boolean isPaintAsDisabled() {
        return paintAsDisabled;
    }


    public Widget getPort() {
        return imageWidget;

    }

    /**
     * Sets whether the label is painted as disabled.
     *
     * @param paintAsDisabled if true, then the label is painted as disabled
     */
    public void setPaintAsDisabled(boolean paintAsDisabled) {
        boolean repaint = this.paintAsDisabled != paintAsDisabled;
        this.paintAsDisabled = paintAsDisabled;
        if (repaint)
            repaint();
    }


    /**
     * Implements the widget-state specific look of the widget.
     *
     * @param previousState the previous state
     * @param state         the new state
     */
    public void notifyStateChanged(ObjectState previousState, ObjectState state) {
        if (state.isHovered())
            setForeground(COLOR_HOVERED);
        else if (state.isSelected())
            setForeground(COLOR_SELECTED);
        else if (state.isHighlighted())
            setForeground(COLOR_HIGHLIGHTED);
        else if (state.isFocused())
            setForeground(COLOR_HOVERED);
        else
            setForeground(COLOR_NORMAL);

        //todo enable me!
//        if (state.isSelected ()) {
//            setControlPointShape (PointShape.SQUARE_FILLED_SMALL);
//            setEndPointShape (PointShape.SQUARE_FILLED_BIG);
//        } else {
//            setControlPointShape (PointShape.NONE);
//            setEndPointShape (POINT_SHAPE_IMAGE);
//        }
    }


    /**
     * Calculates a client area of the image
     *
     * @return the calculated client area
     */
    protected Rectangle calculateClientArea() {
        if (image != null)
            return new Rectangle(0, 0, width, height);
        return super.calculateClientArea();
    }


    /** Paints the image widget. */
    protected void paintWidget() {
        if (image == null)
            return;
        Graphics2D gr = getGraphics();
        if (image != null) {
            if (paintAsDisabled) {
                if (disabledImage == null) {
                    disabledImage = GrayFilter.createDisabledImage(image);
                    MediaTracker tracker = new MediaTracker(getScene().getView());
                    tracker.addImage(disabledImage, 0);
                    try {
                        tracker.waitForAll();
                    } catch (InterruptedException e) {
                        ErrorManager.getDefault().notify(e);
                    }
                }
                gr.drawImage(disabledImage, 0, 0, null);
            } else
                gr.drawImage(image, 0, 0, null);
        }
    }

}
