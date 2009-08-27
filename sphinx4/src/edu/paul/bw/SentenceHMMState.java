/*
 * Copyright 1999-2002 Carnegie Mellon University.  
 * Portions Copyright 2002 Sun Microsystems, Inc.  
 * Portions Copyright 2002 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 * 
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL 
 * WARRANTIES.
 *
 */

package edu.paul.bw;


import java.util.List;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Set;
import java.util.HashSet;
import java.util.Map;
import java.util.LinkedHashMap;
import java.util.Comparator;
import java.util.TreeSet;
import java.io.Serializable;
import java.io.FileWriter;
import java.io.IOException;

import edu.cmu.sphinx.linguist.WordSequence;
import edu.cmu.sphinx.linguist.dictionary.Word;


/**
 * Represents a single state in an SentenceHMM
 */
public abstract class  SentenceHMMState implements Serializable, SearchState  {
    private static int globalStateNumber = 000;

    private int stateNumber;

    // a number of separate variables are maintained in 'fields'
    // inorder to reduce the size of the SentenceHMM
    private int startFrame=0;
    private int endFrame= Integer.MAX_VALUE;
    private int fields;  
    private String name;
    private int nodeId;
    SentenceHMMState direct;
    private  Map arcs;
    private Map arcsPred;
    private  SentenceHMMStateArc[] successorArray;
    private  SentenceHMMStateArc[] predecessorArray;
    


    /**
     * Creates a SentenceHMMState
     *
     * @param name the name of the current SentenceHMMState
     * @param parent the parent of the current instance
     * @param which the index of the current instance
     *
     */
    protected SentenceHMMState(String name,int nodeId, int which) {
        this();
	this.name = name +nodeId+"."+ which;
	setWhich(which);
	setNodeId(nodeId);
    }
    protected SentenceHMMState(String name,int nodeId, int which,int startFrame, int endFrame) {
	this(name,nodeId,which);
	this.startFrame=startFrame;
	this.endFrame=endFrame;
    }



    /**
     * Empty contructor
     */
    protected SentenceHMMState() {
	stateNumber = globalStateNumber++;
        this.arcs = new LinkedHashMap();
	this.arcsPred=new LinkedHashMap();
    }


    /**
     * Determines if this state marks the beginning of a word
     *
     * @return true if the state marks the beginning of a word
     */
    private boolean wordStart;
    public boolean isWordStart() {
	return wordStart;
    }

    /**
     * Sets the 'wordStart' flag
     *
     * @param wordStart <code>true</code> if this state marks the
     * beginning of a word.
     */
    public void setWordStart(boolean wordStart) {
	this.wordStart=wordStart;
    }

    /**
     * Determines if this state is a shard state
     *
     * @return true if the state marks the beginning of a word
     */
    /**
     * Returns the word associated with the particular unit
     *
     * @return the word associated with this state, or null if there
     * is no word associated with this state.
     */
    public Word getAssociatedWord() {
	Word word = null;
	SentenceHMMState state = this;

       if (state instanceof WordState) {
	    WordState wordState = (WordState) state;
	    word = wordState.getWord();
	}
	return word;
    }

    /**
     * Retrieves a short label describing the type of this state.
     * Typically, subclasses of SentenceHMMState will implement this
     * method and return a short (5 chars or less) label
     *
     * @return the short label.
     */
    public String getTypeLabel() {
	return "state";
    }


    /**
     * Gets the word history for this state.
     *
     * @return the word history.
     */
    //TODO Not implemented
    public WordSequence getWordHistory() {
        return WordSequence.EMPTY;
    }
    
    /**
     * Reports an invalid message
     *
     * @param msg the message to display
     */
    private void report(String msg) {
	System.out.println("Invalid state " + getTitle() + "-" + msg);
    }


    public SentenceHMMState getDirect() {
	return direct;
    }
    public void setDirect(SentenceHMMState direct) {
	this.direct=direct;
    }

    public int getStartFrame() {
	return startFrame;
    }
    public int getEndFrame() {
	return endFrame;
    }
    /**
     * Gets the number of successors
     *
     * @return the number of successors
     */
    public int getNumSuccessors() {
	return arcs.size();
    }
    public int getNumPredecessor() {
	return arcsPred.size();
    }
    /**
     * Gets a successor to this search state
     *
     * @return the set of successors
     */
     public SearchStateArc[]  getSuccessors() {
         return (SearchStateArc[]) getSuccessorArray();
     }


     /**
      * Returns the succesors as SentenceHMMStateArc 
      *
      * @return the successors
      */
     private SentenceHMMStateArc[] getSuccessorArray() {
	if (successorArray == null) {
	    successorArray = new SentenceHMMStateArc[arcs.size()];
	    arcs.values().toArray(successorArray);
	}
	return successorArray;
     }

   public SearchStateArc[]  getPredecessors() {
         return (SearchStateArc[]) getPredecessorArray();
     }


     /**
      * Returns the succesors as SentenceHMMStateArc 
      *
      * @return the successors
      */
     private SentenceHMMStateArc[] getPredecessorArray() {
	if (predecessorArray == null) {
	    predecessorArray = new SentenceHMMStateArc[arcsPred.size()];
	    arcsPred.values().toArray(predecessorArray);
	}
	return predecessorArray;
     }

    public abstract boolean isStateZero();
    /**
     * Determines if this state is an emitting state
     *
     * @return true if the state is an emitting state
     */
    public boolean isEmitting() {
	return false;
    }

    /**
     * Determines if this is a final state
     * 
     * @return true if this is a final state
     */
    private boolean isFinal;
    public boolean isFinal() {
	return isFinal;
    }

    /**
     * Sets this is to be final state
     * 
     * @param  state true if this is a final state
     */
    public void setFinalState(boolean state) {
	isFinal=state;
    }

    /**
     * Determines if this state is a unit state
     *
     * @return <code>true</code> if the state is a unit state.
     */
    public boolean isUnit() {
	return false;
    }

    public abstract void dumpAISee(FileWriter f) throws IOException ;

    /**
     * Returns any annotation for this state
     *
     * @return the annotation
     */
    protected String getAnnotation() {
	return "";
    }
  public void  setAlpha(float alpha) {
      throw new Error("setAlpha not implemented :"+this + " class : "+ this.getClass());
    }
    public float getAlpha() {
	  throw new Error("getAlpha not implemented :"+this + " class : "+ this.getClass());
    }
    public void  setBeta(float beta) {
	 throw new Error("setBeta not implemented :"+this + " class : "+ this.getClass());
    }
    public float getBeta() {
	 throw new Error("getBeta not implemented :"+this + " class : "+ this.getClass());	
    }
    public void  setProbability(float probability) {
       	 throw new Error("setProba not implemented :"+this + " class : "+ this.getClass());
    }
    public float getProbability() {
	throw new Error("getProba not implemented :"+this + " class : "+ this.getClass());
    }


    /// les memes en double for linear
 public void  setAlpha(double alpha) {
      throw new Error("setAlpha not implemented :"+this + " class : "+ this.getClass());
    }
    public double getAlphaDouble() {
	  throw new Error("getAlpha not implemented :"+this + " class : "+ this.getClass());
    }
    public void  setBeta(double beta) {
	 throw new Error("setBeta not implemented :"+this + " class : "+ this.getClass());
    }
    public double getBetaDouble() {
	 throw new Error("getBeta not implemented :"+this + " class : "+ this.getClass());	
    }
    public void  setProbability(double probability) {
       	 throw new Error("setProba not implemented :"+this + " class : "+ this.getClass());
    }
    public double getProbabilityDouble() {
	throw new Error("getProba not implemented :"+this + " class : "+ this.getClass());
    }



 public void  setPhoneAcc(float phoneAcc) {
      throw new Error("setPhoneAcc not implemented :"+this + " class : "+ this.getClass());
    }
    public float getPhoneAcc() {
	  throw new Error("getPhoneAcc not implemented :"+this + " class : "+ this.getClass());
    }
 public void  setAlphaPrime(double alphaPrime) {
      throw new Error("setAlphaPrime not implemented :"+this + " class : "+ this.getClass());
    }
    public double getAlphaPrime() {
	  throw new Error("getAlphaPrime not implemented :"+this + " class : "+ this.getClass());
    }

    public void  setBetaPrime(double betaPrime) {
      throw new Error("setBetaPrime not implemented :"+this + " class : "+ this.getClass());
    }
    public double getBetaPrime() {
	  throw new Error("getBetaPrime not implemented :"+this + " class : "+ this.getClass());
    }


    public void  setGammaMpe(double gammaMpe) {
      throw new Error("setGammaMpe not implemented :"+this + " class : "+ this.getClass());
    }
    public double getGammaMpe() {
	  throw new Error("getGammaMpe not implemented :"+this + " class : "+ this.getClass());
    }
















    /**
     * Dumps this state
     */
    private void dump() {
      if (this instanceof HMMStateState && this.isEmitting()) 
	  return;
      System.out.println(" ---- " + getTitle() + " - "+this+"::"+
			 hashCode()+":" + getClass().getName()+" o:"+
			 getOrder());
        for (int i = 0; i < getSuccessors().length; i++) {
	    SentenceHMMStateArc arc = (SentenceHMMStateArc) getSuccessors()[i];
	    SearchState tr=arc.getToState();
	    System.out.println("   -> " +
			       tr.toString() + " " +
			       
			       tr.hashCode()+":" + tr.getClass().getName() );
	}
    }




    /**
     * Validates this SentenceHMMState and all successors
     *
     */
    public void validateAll() {
        // TODO fix me
    }


    /**
     * Gets the name for this state
     *
     * @return the name
     */
    public String getName() {
	return name;
    }

    /**
     * Returns a pretty name for this HMM
     *
     * @return a pretty name
     */
    public String getPrettyName() {
	return getName();
    }

    private String cachedName=null;
    /**
     * Returns the string representation of this object
     */
    public String toString() {
	if (cachedName == null) {
	    StringBuffer sb = new StringBuffer();
	    if (isEmitting()) {
		sb.append("*");
	    }
	    sb.append(getName());

	    String base =  (isEmitting() ? "*" : "") + getName()
		+ getWhich() + (isFinal() ? "!" : "");


	    if (isFinal()) {
		sb.append("!");
	    }
	    cachedName = sb.toString();
	}
	return cachedName;
    }

    /*
      * Returns a pretty version of the string representation 
      * for this object
      *
      * @return a pretty string
      */
     public String toPrettyString() {
         return toString();
     }


    /**
     * Gets the fullName for this state
     *
     * @return the full name for this state
     */
    private String fullName;
    public String getFullName() {
	if (fullName == null) {
	    
		fullName =  getName();
	}

	return fullName;
    }
 public void connect(SentenceHMMStateArc arc) {
	if (successorArray != null) {
	    successorArray = null;
	}
	rawConnect(arc);
    }


    /**
     * Connects the arc to this sentence hmm, but don't affect the
     * predecessor relation ship
     *
     * @param arc the arc to the next state
     */
    private void rawConnect(SentenceHMMStateArc arc) {
        SentenceHMMState state = (SentenceHMMState) arc.getToState();
 	arcs.put(state.getKey(), arc);
    }

    public void connectPred(SentenceHMMStateArc arc) {
	if (predecessorArray != null) {
	    predecessorArray = null;
	}
	rawConnectPred(arc);
    }


    /**
     * Connects the arc to this sentence hmm, but don't affect the
     * predecessor relation ship
     *
     * @param arc the arc to the next state
     */
    private void rawConnectPred(SentenceHMMStateArc arc) {
        SentenceHMMState state = (SentenceHMMState) arc.getFromState();
 	arcsPred.put(state.getKey(), arc);
    }




    /**
     * Gets the signature for this state
     *
     * @return the signature
     */
    public String getSignature() {
        return getFullName();
    }
    public static int getKey(int wich,int nodeId) {
	return wich+13*nodeId;
    }
    // meme formule 

    public int getKey() {
	return wich+13*getNodeId();
    }

    /**
     *  gets the title (fullname + stateNumber) 
     * for this state
     *
     * @return the title
     */
    public String getTitle() {
	return getFullName() + ":" + stateNumber;
	// return getSignature() + ":" + stateNumber;
    }

    private int wich;
    /**
     * Retrieves the index for this state
     * words
     *
     * @return the index
     */
    public int getWhich() {
	return wich;
    }


    /**
     * Sets the index for this state
     * 
     * @param which the index for this state
     */
    public void setWhich(int which) {
	this.wich= which;
    }
    public void setNodeId(int nodeId) {
	this.nodeId=nodeId;
    }
    public int getNodeId() {
	return nodeId;
    }

    /**
     * Retrieves the parent sate
     *
     * @return the parent state (or null if this state does not have a
     * parent state).
     */
    public SentenceHMMState getParent() {
	return null;
    }

    /**
     * Sets the parent state for this state
     *
     * @param parent the parent state
     */
    private void setParent(SentenceHMMState parent) {
	return;
    }


    /**
     * Searches the set of arcs for an arc that points to a state with
     * an identical value
     *
     * @param state the state to search for
     *
     * @return the arc or null if none could be found.
     */
    public SentenceHMMStateArc findArc(SentenceHMMState state) {
	SentenceHMMStateArc arc =
 	    (SentenceHMMStateArc) arcs.get(state.getKey());
	return arc;
    }



     /**
     * Sets the state number for this state
     *
     * @param stateNumber the state number
     */
    private void setStateNumber(int stateNumber) {
	this.stateNumber = stateNumber;
    }

    /**
     * Gets the state number for this state
     *
     * @return  the state number
     */
    private int getStateNumber() {
	return stateNumber;
    }

    /**
     * Collect all states starting from the given start state
     *
     * @param start the state to start the search from
     *
     * @return the set of collected state
     */
    static public Set collectStates(SentenceHMMState start) {
	Set visitedStates = new HashSet();
	List queue = new LinkedList();

	queue.add(start);
	visitedStates.add(start);
	while (queue.size() > 0) {
	    SentenceHMMState state = (SentenceHMMState) queue.remove(0);
	    //visitedStates.add(state);
            //state.dump();// on ne dump pas 
	    SentenceHMMStateArc[] successors = state.getSuccessorArray();
	    for (int i = 0; i < successors.length; i++) {
		SentenceHMMStateArc arc = successors[i];
		SentenceHMMState nextState = (SentenceHMMState) arc.getToState();
		if (!visitedStates.contains(nextState)) {
		    queue.add(nextState);visitedStates.add(nextState);//paul
		}
	    }
	}
	return visitedStates;
    }

     /**
      * Returns the order of this particular state
      *
      * @return the state order for this state
      */
    abstract public int getOrder();
}

