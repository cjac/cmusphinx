/*
 * Copyright 1999-2004 Carnegie Mellon University.
 * Portions Copyright 2004 Sun Microsystems, Inc.
 * Portions Copyright 2004 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 *
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

package edu.cmu.sphinx.linguist.flatv0;

import edu.cmu.sphinx.linguist.Linguist;
import edu.cmu.sphinx.linguist.SearchGraph;
import edu.cmu.sphinx.linguist.SearchState;
import edu.cmu.sphinx.linguist.UnitSearchState;
import edu.cmu.sphinx.linguist.WordSearchState;

import edu.cmu.sphinx.linguist.acoustic.AcousticModel;
import edu.cmu.sphinx.linguist.acoustic.HMM;
import edu.cmu.sphinx.linguist.acoustic.HMMPosition;
import edu.cmu.sphinx.linguist.acoustic.HMMState;
import edu.cmu.sphinx.linguist.acoustic.HMMStateArc;
import edu.cmu.sphinx.linguist.acoustic.Unit;

import edu.cmu.sphinx.linguist.dictionary.Pronunciation;
import edu.cmu.sphinx.linguist.dictionary.Word;

import edu.cmu.sphinx.linguist.language.grammar.GrammarNode;

import edu.cmu.sphinx.util.LogMath;

import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;

import java.io.IOException;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * Constructs a loop of all the context-independent phones.
 * This loop is used in the static flat linguist for detecting
 * out-of-grammar utterances. A 'phoneInsertionProbability' will be added
 * to the score each time a new phone is entered. To obtain the all-phone
 * search graph loop, simply called the method 
 * {@link #getSearchGraph() getSearchGraph}.
 */
public class CIPhoneLoop {
    private AcousticModel model;
    private LogMath logMath;
    private float logPhoneInsertionProbability;
    private float logOne = LogMath.getLogOne();

    /**
     * Creates the CIPhoneLoop with the given acoustic model
     * and phone insertion probability
     *
     * @param model the acoustic model
     * @param logPhoneInsertionProbability the insertion probability
     */
    public CIPhoneLoop(AcousticModel model, 
            float logPhoneInsertionProbability) {
        this.model = model;
        this.logPhoneInsertionProbability =
            logPhoneInsertionProbability;
    }


    /**
     * Creates a new loop of all the context-independent phones.
     *
     * @return the phone loop search graph
     */
    public SearchGraph getSearchGraph() {
        return new PhoneLoopSearchGraph();
    }

    class PhoneLoopSearchGraph implements SearchGraph {

        private Map existingStates;
        private SentenceHMMState firstState;
        private SentenceHMMState lastState;
	/* c'est pas beau mais ..... */
	public SentenceHMMState getLastState() {
	    return lastState;
	}

        /**
         * Constructs a phone loop search graph.
         */
        public PhoneLoopSearchGraph() {
            existingStates = new HashMap();
            firstState = new UnknownWordState();
            SentenceHMMState branchState = new BranchOutState(firstState);
            //attachState(firstState, branchState, logOne, logOne, logOne);
            
             lastState = new LoopBackState(firstState);
            //lastState.setFinalState(true);
            attachState(lastState, branchState, logOne, logOne, logOne);
            
            for (Iterator i = model.getContextIndependentUnitIterator();
                 i.hasNext(); ) {
                Unit unit = (Unit) i.next();
                UnitState unitState = 
                    new UnitState(unit, HMMPosition.UNDEFINED);
                
                // attach unit state to the branch out state
                attachState(branchState, unitState, logOne, logOne,
                            logPhoneInsertionProbability);
                attachState(firstState, unitState, logOne, logOne,
                            logPhoneInsertionProbability);
                HMM hmm = model.lookupNearestHMM
                    (unitState.getUnit(), unitState.getPosition(), false);
                HMMState initialState = hmm.getInitialState();
                HMMStateState hmmTree = new HMMStateState(unitState, initialState);
                addStateToCache(hmmTree);
                
                // attach first HMM state to the unit state
                attachState(unitState, hmmTree, logOne, logOne, logOne);
                
                // expand the HMM tree
                HMMStateState finalState = expandHMMTree(unitState, hmmTree);
                
                // attach final state of HMM tree to the loopback state
                attachState(finalState, lastState, logOne, logOne, logOne);
            }
        }

        /**
         * Retrieves initial search state
         *
         * @return the set of initial search state
         */
        public SearchState getInitialState() {
            return firstState;
        }

        /**
         * Returns the number of different state types maintained
         * in the search graph
         *
         * @return the number of different state types
         */
        public int getNumStateOrder() {
            return 5;
        }

        /**
         * Checks to see if a state that matches the given state already exists
         * 
         * @param state  the state to check
         * 
         * @return true if a state with an identical signature already exists.
         */
        private SentenceHMMState getExistingState(SentenceHMMState state) {
            return (SentenceHMMState) existingStates.get(state.getSignature());
        }
        
        /**
         * Adds the given state to the cache of states
         * 
         * @param state
         *                the state to add
         */
        private void addStateToCache(SentenceHMMState state) {
            existingStates.put(state.getSignature(), state);
        }
        
        /**
         * Expands the given hmm state tree
         * 
         * @param parent  the parent of the tree
         * @param tree    the tree to expand
         * 
         * @return the final state in the tree
         */
        private HMMStateState expandHMMTree(UnitState parent,
                                            HMMStateState tree) {
            HMMStateState retState = tree;
            HMMStateArc[] arcs = tree.getHMMState().getSuccessors();
            for (int i = 0; i < arcs.length; i++) {
                HMMStateState newState;
                if (arcs[i].getHMMState().isEmitting()) {
                    newState = new HMMStateState
                        (parent, arcs[i].getHMMState());
                } else {
                    newState = new NonEmittingHMMState
                        (parent, arcs[i].getHMMState());
                }
                SentenceHMMState existingState = getExistingState(newState);
                float logProb = arcs[i].getLogProbability();
                if (existingState != null) {
                    attachState(tree, existingState, logProb, logOne, logOne);
                } else {
                    attachState(tree, newState, logProb, logOne, logOne);
                    addStateToCache(newState);
                    retState = expandHMMTree(parent, newState);
                }
            }
            return retState;
        }
        
        protected void attachState(SentenceHMMState prevState,
                                   SentenceHMMState nextState, 
                                   float logAcousticProbability,
                                   float logLanguageProbability, 
                                   float logInsertionProbability) {
            SentenceHMMStateArc arc = new SentenceHMMStateArc
                (nextState,
                 logAcousticProbability,
                 logLanguageProbability,
                 logInsertionProbability);
            prevState.connect(arc);
        }
    }
}

class UnknownWordState extends SentenceHMMState implements WordSearchState {

    public Pronunciation getPronunciation() {
        return Word.UNKNOWN.getPronunciations()[0];
    }

    public int getOrder() {
        return 4;
    }

    public String getName() {
        return "UnknownWordState";
    }

    /**
     * Returns true if this UnknownWordState indicates the start of a word.
     * Returns false if this UnknownWordState indicates the end of a word.
     *
     * @return true if this UnknownWordState indicates the start of a word,
     *         false if this UnknownWordState indicates the end of a word
     */
    public boolean isWordStart() {
        return true;
    }
}

class LoopBackState extends SentenceHMMState {
    
    LoopBackState(SentenceHMMState parent) {
        super("CIPhonesLoopBackState", parent, 0);
    }

    public int getOrder() {
        return 1;
    }
}

class BranchOutState extends SentenceHMMState {
    
    BranchOutState(SentenceHMMState parent) {
        super("BranchOutState", parent, 0);
    }

    public int getOrder() {
        return 2;
    }
}

