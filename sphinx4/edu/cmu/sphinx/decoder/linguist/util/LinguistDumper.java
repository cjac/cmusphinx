/*
 * Copyright 1999-2003 Carnegie Mellon University.  
 * Portions Copyright 2003 Sun Microsystems, Inc.  
 * Portions Copyright 2003 Mitsubishi Electronic Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 * 
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL 
 * WARRANTIES.
 *
 */
package edu.cmu.sphinx.decoder.linguist.util;

import edu.cmu.sphinx.decoder.linguist.SearchState;
import edu.cmu.sphinx.decoder.linguist.UnitSearchState;
import edu.cmu.sphinx.decoder.linguist.WordSearchState;
import edu.cmu.sphinx.decoder.linguist.HMMSearchState;
import edu.cmu.sphinx.decoder.linguist.SearchStateArc;
import edu.cmu.sphinx.decoder.linguist.LinguistProcessor;
import edu.cmu.sphinx.decoder.linguist.Linguist;
import edu.cmu.sphinx.util.SphinxProperties;

import java.io.PrintStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;

import java.util.HashSet;
import java.util.Map;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;


/**
 *  A linguist processor that dumps out the search space in a simple
 *  format. This processor is designed so that it can be easily
 *  extended by replacing the dumpNode and the dumpEdge methods.
 */
public class LinguistDumper implements LinguistProcessor  {
    

    /**
     * A sphinx property name for the destination of the LinguistDumper
     */
    public final static String PROP_FILENAME 
        = "edu.cmu.sphinx.decoder.linguist.util.LinguistDumper.filename";

    
    /**
     * The default value for PROP_FILENAME.
     */
    public final static String PROP_FILENAME_DEFAULT = "linguistDump.txt";


    protected SphinxProperties properties;
    private boolean depthFirst = true;


    /**
     * Dumps the search space hmm in GDL format
     *
     * @param props the set of SphinxProperties
     * @param linguist the linguist
     *
     */
    public void process(SphinxProperties props, Linguist linguist) {
        String fileName = props.getString(PROP_FILENAME, getDefaultName());
        properties = props;

	try {
	    FileOutputStream fos = new FileOutputStream(fileName);
	    PrintStream out = new PrintStream(fos);
	    dumpSearchGraph(out, linguist.getInitialSearchState());
	    out.close();
	} catch (FileNotFoundException fnfe) {
	    System.out.println("Can't dump to file " 
		    + fileName + " " + fnfe);
	}
    }


    /**
     * Returns the sphinx properties for this processor
     * 
     * @return the sphinx properties associated with this dumper
     */
    protected SphinxProperties getProperties() {
        return properties;
    }

    /**
     * Sets whether the traversal is depth first or breadth first
     *
     * @param depthFirst if true traversal is depth first, otherwise
     * the traversal is breadth first
     */
    protected void setDepthFirst(boolean depthFirst) {
	this.depthFirst = depthFirst;
    }

    /**
     * Retreives the default name for the destination dump. This
     * method is typically overridden by derived classes
     *
     *
     * @return the default name for the file.
     */
    protected String getDefaultName() {
	return PROP_FILENAME_DEFAULT;
    }

    /**
     * Called at the start of the dump
     *
     * @param out the output stream.
     */
    protected void startDump(PrintStream out) {
    }

    /**
     * Called at the end of the dump
     *
     * @param out the output stream.
     */
    protected void endDump(PrintStream out) {
    }

    /**
     * Called to dump out a node in the search space
     *
     * @param out the output stream.
     * @param state the state to dump
     * @param level the level of the state
     */
    protected void startDumpNode(PrintStream out, 
                                 SearchState state, int level) {
    }

    /**
     * Called to dump out a node in the search space
     *
     * @param out the output stream.
     * @param state the state to dump
     * @param level the level of the state
     */
    protected void endDumpNode(PrintStream out, SearchState state,
	    int level) {
    }

    /**
     * Dumps an arc
     *
     * @param out the output stream.
     * @param from arc leaves this state
     * @param arc the arc to dump
     * @param level the level of the state
     */
    protected void dumpArc(PrintStream out, SearchState from,
                           SearchStateArc arc, int level) {
    }


    /**
     * Dumps the search graph 
     *
     * @param out place to dump the output
     * @param startingState the initial state of the search space
     */
    private void dumpSearchGraph(PrintStream out, 
                                 SearchState startingState) {
	List queue = new LinkedList();
	Set visitedStates = new HashSet();

	startDump(out);
	queue.add(new StateLevel(startingState, 0));

	while (queue.size() > 0) {
	    StateLevel stateLevel = (StateLevel) queue.remove(0);
	    int level = stateLevel.getLevel();
	    SearchState state = stateLevel.getState();

            equalCheck(state);


	    if (!visitedStates.contains(state)) {
		visitedStates.add(state);

		startDumpNode(out, state, level);
		SearchStateArc[] arcs = state.getSuccessors();


		for (int i = arcs.length - 1; i >= 0; i--) {
		    SearchState nextState = arcs[i].getState();
		    dumpArc(out, state, arcs[i], level);
		    if (depthFirst) {
			// if depth first, its a stack
			queue.add(0, new StateLevel(nextState, level + 1));
		    } else {
			queue.add(new StateLevel(nextState, level + 1));
		    }
		}
		endDumpNode(out, state, level);
	    }
	}
	endDump(out);
    }

    Map eqStates = new HashMap();
    Map eqSigs = new HashMap();

    /**
     * This is a bit of test/debugging code that ensures that the
     * states that have equal signatures are also considered to be
     * object.equals and vice versa.. This method will dump out any
     * states where this contract is not true
     *
     * @param state the state to check
     */
    private void equalCheck(SearchState state) {
        SearchState eqState = (SearchState) eqStates.get(state);
        SearchState eqSig = (SearchState) eqSigs.get(state.getSignature());


        if ((eqState == null || eqSig == null) && !(eqState == eqSig)) {
            System.out.println("Missing one: ");
            System.out.println("  state val: " + state);
            System.out.println("  state sig: " + state.getSignature());

            System.out.println("  eqState val: " + eqState);
            System.out.println("  eqSig val: " + eqSig);

            if (eqState != null) {
                System.out.println("   eqState sig: " + eqState.getSignature());
            }
            if (eqSig != null) {
                System.out.println("   eqSig sig: " + eqSig.getSignature());
            }

        }


        if (eqState == null) {
            eqStates.put(state, state);
            eqState = state;
        }

        if (eqSig == null) {
            eqSigs.put(state.getSignature(), state);
            eqSig = state;
        }

        if (!eqState.getSignature().equals(state.getSignature())) {
            System.out.println("Sigs mismatch for: ");
            System.out.println("  state sig: " + state.getSignature());
            System.out.println("  eqSig sig: " + eqSig.getSignature());
            System.out.println("  state val: " + state);
            System.out.println("  eqSig val: " + eqSig);
        }

        if (!eqState.equals(state)) {
            System.out.println("obj mismatch for: ");
            System.out.println("  state sig: " + state.getSignature());
            System.out.println("  eqSig sig: " + eqSig.getSignature());
            System.out.println("  state val: " + state);
            System.out.println("  eqSig val: " + eqSig);
        }
    }
}




/**
 * A class for bundling together a SearchState and its level.
 */
class StateLevel {
    private int level;
    private SearchState state;

    /**
     * Constructs a StateLevel from its primitive components.
     *
     * @param state the state to be bundled in the StateLevel
     * @param level the level of the state
     */
    StateLevel(SearchState state, int level) {
	this.state = state;
	this.level = level;
    }

    /**
     * Returns the state
     *
     * @return the state
     */
    SearchState getState() {
	return state;
    }

    /**
     * Returns the level
     *
     * @return the level.
     */
    int getLevel() {
	return level;
    }

    /**
     * Returns a string representation of the object
     *
     * @return a string representation
     */
    public String toString() {
	return "" + level + " " + state.getSignature() +
	    " 1 " + getTypeLabel(state);
    }


    /**
     * Retrieves a type label for a state
     * 
     * @param state the state of interest
     *
     * @return a label for the type of state (one of Unit, Word, HMM
     * or other
     *
     */
    public String getTypeLabel(SearchState state) {
        if (state instanceof UnitSearchState) {
            return "Unit";
        }

        if (state instanceof WordSearchState) {
            return "Word";
        }

        if (state instanceof HMMSearchState) {
            return "HMM";
        }
        return "other";
    }
}

