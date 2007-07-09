package edu.cmu.sphinx.tools.confdesigner;

import org.netbeans.api.visual.action.ActionFactory;
import org.netbeans.api.visual.action.MoveStrategy;
import org.netbeans.api.visual.widget.Widget;

import java.awt.*;

/**
 * Implements a proxy which can be switch between grid-strategy and free-move strategy.
 *
 * @author Holger Brandl
 */
public class SwitchableMoveStratgy implements MoveStrategy {

    private MoveStrategy gridStrategy;
    private MoveStrategy freeMoveStrategy;

    private boolean useGridStrategy;


    public SwitchableMoveStratgy() {
        gridStrategy = ActionFactory.createSnapToGridMoveStrategy(20, 20);

        freeMoveStrategy = ActionFactory.createFreeMoveStrategy();
    }


    public Point locationSuggested(Widget widget, Point point, Point point1) {
        if (useGridStrategy) {
            return gridStrategy.locationSuggested(widget, point, point1);
        } else {
            return freeMoveStrategy.locationSuggested(widget, point, point1);
        }
    }


    public boolean isUseGridStrategy() {
        return useGridStrategy;
    }


    public void setUseGridStrategy(boolean useGridStrategy) {
        this.useGridStrategy = useGridStrategy;
    }
}
