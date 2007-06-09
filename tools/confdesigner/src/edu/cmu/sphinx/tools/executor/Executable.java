package edu.cmu.sphinx.tools.executor;

import edu.cmu.sphinx.util.props.Configurable;

/**
 * A component which can be directly executed within the S4Designer.
 *
 * @author Holger Brandl
 */
public interface Executable extends Configurable {

    /** Executes this executor. */
    public void doExecute();
}
