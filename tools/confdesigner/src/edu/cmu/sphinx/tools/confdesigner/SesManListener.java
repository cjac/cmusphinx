package edu.cmu.sphinx.tools.confdesigner;

/**
 * DOCUMENT ME!
 *
 * @author Holger Brandl
 */
public interface SesManListener {

    void newActiveScene(SceneContext activeScene);


    void removedScene(SceneContext sc);


    void addedScene(SceneContext sc);
}
