package edu.cmu.sphinx.tools.executor;

import edu.cmu.sphinx.util.props.PropertySheet;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public interface ExecutorListener {

    public void addedExecutor(PropertySheet executablePS);
    public void removedExecutor(PropertySheet ps);

}
