package edu.cmu.sphinx.tools.confdesigner;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public class ConfEdge {

    private ConfPin source;
    private ConfPin target;


    public ConfEdge(ConfPin source, ConfPin target) {
        assert source != null;
        assert target != null;
        
        this.source = source;
        this.target = target;
    }


    public ConfPin getSource() {
        return source;
    }


    public ConfPin getTarget() {
        return target;
    }


    public String toString() {
        return source.toString() + "-->"+target.toString();
    }
}
