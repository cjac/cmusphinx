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
package edu.cmu.sphinx.result;

import edu.cmu.sphinx.linguist.dictionary.Word;

import edu.cmu.sphinx.result.Lattice;
import edu.cmu.sphinx.result.Edge;
import edu.cmu.sphinx.util.LogMath;

import java.util.LinkedList;
import java.util.List;
import java.util.StringTokenizer;
import java.util.Iterator;
import java.util.Vector;
import java.util.Collection;
import java.util.Collections;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Comparator;
import java.util.HashSet;
/**
 * Nodes are part of Lattices.  The represent theories that words were spoken over a given time.
 */
public class Node implements  NodeEdge{
    protected static int nodeCount = 0; // used to generate unique IDs for new Nodes.

    protected String id;
    protected Word word;
    protected String variant;
    protected int beginTime = -1;
    protected int endTime = -1;
    protected int lastBeginTime=-10;
    protected int firstEndTime = -1;
    protected Vector <Edge> enteringEdges;
    protected Vector <Edge> leavingEdges;
    protected double forwardScore;
    protected double backwardScore;
    protected double posterior;
    protected Word best;    
    {
        enteringEdges = new Vector<Edge>();
        leavingEdges = new Vector<Edge>();
        nodeCount++;
    }

    /**
     * Create a new Node
     *
     * @param word the word of this node
     * @param beginTime the start time of the word
     * @param endTime the end time of the word
     */
    protected Node(Word word, int beginTime, int endTime) {
        this(getNextNodeId(), word, beginTime, endTime);
    }

    /**
     * Create a new Node with given ID.
     * Used when creating a Lattice from a .LAT file
     *
     * @param id
     * @param word
     * @param beginTime
     * @param endTime
     */
    protected Node(String id, Word word, int beginTime, int endTime) 
    {this(id,word,beginTime,endTime,null);
    }
    protected Node(String id, Word word, int beginTime, int endTime,String variant) {
        this.id = id;
        this.word = word;
        this.beginTime = beginTime;
        this.endTime = endTime;
        this.forwardScore = LogMath.getLogZero();
        this.backwardScore = LogMath.getLogZero();
        this.posterior = LogMath.getLogZero();
	this.variant =variant;
    }

    /**
     * Get a unique ID for a new Node.
     * Used when creating a Lattice from a .LAT file
     *
     * @return the unique ID for a new node
     */
    protected static String getNextNodeId() {
        return Integer.toString(nodeCount);
    }
    private String getVariant() {
	return variant==null ? "" : variant;
    }
    /**
     * Test if a node has an Edge to a Node
     * @param n
     * @return unique Node ID
     */
    protected boolean hasEdgeToNode(Node n) {
        return getEdgeToNode(n) != null;
    }

    /**
     * given a node find the edge to that node
     *
     * @param n the node of interest
     *
     * @return the edge to that node or <code> null</code>  if no edge
     * could be found.
     */
    public Edge getEdgeToNode(Node n) {
        for (Iterator j = leavingEdges.iterator(); j.hasNext();) {
            Edge e = (Edge) j.next();
            if (e.getToNode() == n) {
                return e;
            }
        }
        return null;
    }

    /**
     * Test is a Node has an Edge from a Node
     *
     * @param n
     * @return true if this node has an Edge from n
     */
    protected boolean hasEdgeFromNode(Node n) {
        return getEdgeFromNode(n) != null;
    }

    /**
     * given a node find the edge from that node
     *
     * @param n the node of interest
     *
     * @return the edge from that node or <code> null</code>  if no edge
     * could be found.
     */
    public Edge getEdgeFromNode(Node n) {
        for (Iterator <Edge> j = enteringEdges.iterator(); j.hasNext();) {
            Edge e = j.next();
            if (e.getFromNode() == n) {
                return e;
            }
        }
        return null;
    }

    protected void setWord( Word w) {

	word=w;
    }


    /**
     * Test if a Node has all Edges from the same Nodes and another Node.
     *
     * @param n
     * @return true if this Node has Edges from the same Nodes as n
     */
    protected boolean hasEquivalentEnteringEdges(Node n) {
        if (enteringEdges.size() != n.getEnteringEdges().size()) {
            return false;
        }
        for (Iterator <Edge> i = n.enteringEdges.iterator(); i.hasNext();) {
            Edge e =  i.next();
            Node fromNode = e.getFromNode();
            if (!hasEdgeFromNode(fromNode)) {
                return false;
            }
        }

	if (false) 
	    for (Iterator <Edge> i = n.enteringEdges.iterator(); i.hasNext();) {
		Edge e =  i.next();
		Node fromNode = e.getFromNode();
		if (!hasEdgeFromNode(fromNode)) {
		    System.err.println("  il y a tres gros gros probleme ");
		}
	    }




        return true;
    }



 protected boolean hasEquivalentEnteringEdgesbis(Node n) {
        if (enteringEdges.size() != n.getEnteringEdges().size()) {
            return false;
        }
	 HashSet<Node> s1= new  HashSet<Node>(10) , s2= new HashSet<Node>(10);
        for (Iterator <Edge> i = enteringEdges.iterator(); i.hasNext();) {
            Edge e =  i.next();
            s1.add(e.getFromNode());
        }

	if (true) 
	    for (Iterator <Edge> i = n.enteringEdges.iterator(); i.hasNext();) {
		Edge e =  i.next();
		s2.add( e.getFromNode());
	    }
	return s1.equals(s2);
 }



    protected void  fusionEntree(Node n, LogMath  logMath) {
        assert (enteringEdges.size() == n.getEnteringEdges().size());
	    for (Iterator <Edge> i = n.enteringEdges.iterator(); i.hasNext();) {
		Edge e1 =  i.next();
		Node fromNode = e1.getFromNode();
		Edge e= getEdgeFromNode(fromNode);
		e.setPscore(logMath.addAsLinear((float)e.getPscore(),(float)e1.getPscore()));
	    }
	
    }



    /**
     * Test if a Node has all Edges to the same Nodes and another Node.
     *
     * @param n the node of interest
     * @return true if this Node has all Edges to the sames Nodes as n
     */
    public boolean hasEquivalentLeavingEdges(Node n) {
        if (leavingEdges.size() != n.getLeavingEdges().size()) {
            return false;
        }
        for (Iterator <Edge> i = n.leavingEdges.iterator(); i.hasNext();) {
            Edge e =  i.next();
            Node toNode = e.getToNode();
            if (!hasEdgeToNode(toNode)) {
                return false;
            }
        }
        return true;
    }

    public void  fusionSortie(Node n, LogMath logMath) {
	assert  (leavingEdges.size() == n.getLeavingEdges().size());
        for (Iterator <Edge> i = n.leavingEdges.iterator(); i.hasNext();) {
            Edge e1 =  i.next();
            Node toNode = e1.getToNode();
	    Edge e= getEdgeToNode(toNode);
	    e.setPscore(logMath.addAsLinear((float)e.getPscore(),(float)e1.getPscore()));
        }
    }

    /**
     * Get the Edges to this Node
     *
     * @return Edges to this Node
     */
    public Collection <Edge>  getEnteringEdges() {
        return enteringEdges;
    }

    /**
     * Get the Edges from this Node
     *
     * @return Edges from this Node
     */
    public Collection <Edge> getLeavingEdges() {
        return leavingEdges;
    }

    /**
     * Returns a copy of the Edges from this Node, so that the underlying
     * data structure will not be modified.
     *
     * @return a copy of the edges from this node
     */
    public Collection<Edge> getCopyOfLeavingEdges() {
        return new Vector<Edge>(leavingEdges);
    }

    /**
     * Add an Edge from this Node
     *
     * @param e
     */
    protected void addEnteringEdge(Edge e) {
        enteringEdges.add(e);
    }

    /**
     * Add an Edge to this Node
     *
     * @param e
     */
    protected void addLeavingEdge(Edge e) {
        leavingEdges.add(e);
    }

    /**
     * Remove an Edge from this Node
     *
     * @param e
     */
    protected void removeEnteringEdge(Edge e) {
        enteringEdges.remove(e);
    }

    /**
     * Remove an Edge to this Node
     *
     * @param e the edge to remove
     */
    public void removeLeavingEdge(Edge e) {
        leavingEdges.remove(e);
    }

    protected void removeLeavingEdges() {
	leavingEdges.clear();
    }
    /**
     * Get the ID associated with this Node
     *
     * @return the ID
     */
    public String getId() {
        return id;
    }
    public int getIdInt() {
	return Integer.parseInt(id);
    }
    /**
     * Get the word associated with this Node
     *
     * @return the word
     */
    public Word getWord() {
        return word;
    }
    
    /**
     * Get the frame number when the word began
     *
     * @return the begin frame number, or -1 if the frame number is unknown
     */
    public int getBeginTime() {
        if (beginTime == -1) {
            calculateBeginTime();
        }
        return beginTime;
    }

    /**
     * Get the frame number when the word ends
     *
     * @return the end time, or -1 if the frame number if is unknown
     */
    private boolean pasFait=true;
    public int getEndTime() {
        if (endTime==-1&& pasFait)
	    calculateEndTime();
	pasFait=false;
        return endTime;
    }
 public int getFirstEndTime() {
	if (firstEndTime==-1) { 
	    Iterator e = leavingEdges.iterator();
            firstEndTime=Integer.MAX_VALUE;
	    while (e.hasNext()) {
		Edge edge = (Edge)e.next();
		if (edge.getToNode().getBeginTime()-1 <firstEndTime) {
                firstEndTime = edge.getToNode().getBeginTime()-1;
		}

	    }
	}
	return firstEndTime;
	}
 public int getLastBeginTime() {
	if (lastBeginTime==-10) { 
	    Iterator<Edge>  e = enteringEdges.iterator();
            lastBeginTime=getBeginTime();
	    while (e.hasNext()) {
		Edge edge = e.next();
		if (edge.getFromNode().getEndTime()+1 >lastBeginTime) {
                lastBeginTime  = edge.getFromNode().getEndTime()+1;
		}

	    }
	}
	return lastBeginTime;
	}

    /**
     * Returns a description of this Node that contains the word, the
     * start time, and the end time.
     *
     * @return a description of this Node
     */
    public String toString() { 
	String posterior = "" + getPosterior();
        if (getPosterior() == LogMath.getLogZero()) {
            posterior = "log zero";}
        return ("N"+id+"(" + word.getSpelling() + "," + getBeginTime() + "|" + 
		getEndTime()+"|"+posterior + ")");
    }

    /**
     * Internal routine when dumping Lattices as AiSee files
     *
     * @param f
     * @throws IOException
     */
    void dumpAISee(FileWriter f, LogMath logMath) throws IOException {
        String posterior = "" + logMath.logToLinear((float)getPosterior());
        if (getPosterior() == LogMath.getLogZero()) {
            posterior = "ln0";
        }
        f.write("node: { title: \"" + id + "\" label: \""
                + id + getWord() + "[" + getBeginTime() + "," + getEndTime() + 
                " p:" + posterior + "]\" }\n");
    }

    /**
     * Internal routine used when dumping Lattices as .LAT files
     * @param f
     * @throws IOException
     */
    void dump(PrintWriter f) { // throws IOException {
	String posterior = "" + getPosterior();
        if (getPosterior() == LogMath.getLogZero()) {
            posterior = "log zero";
	}
        f.println("node: " + id + " " + word.getSpelling() + 
		  " a:" + getForwardScore() + " b:" + getBackwardScore()+
		  " p:" + posterior + " first:" + getBeginTime() + " last:" + getEndTime());
    }

    void dumpS3(PrintWriter f, int i) { // throws IOException {
	id=""+i;
        f.println(id + " " + word.getSpelling() +getVariant()+ 
		  " "+ getBeginTime() + " " + getEndTime() + " "+ getEndTime());
    }

    /**
     * Internal routine used when loading Lattices from .LAT files
     * @param lattice
     * @param tokens
     */
    static void load(Lattice lattice, StringTokenizer tokens) {

        String id = tokens.nextToken();
        String label = tokens.nextToken();

        lattice.addNode(id, label, 0, 0);
    }
    /**
     * @return Returns the backwardScore.
     */
    public double getBackwardScore() {
        return backwardScore;
    }
    /**
     * @param backwardScore The backwardScore to set.
     */
    public void setBackwardScore(double backwardScore) {
        this.backwardScore = backwardScore;
    }
    /**
     * @return Returns the forwardScore.
     */
    public double getForwardScore() {
        return forwardScore;
    }
    /**
     * @param forwardScore The forwardScore to set.
     */
    public void setForwardScore(double forwardScore) {
        this.forwardScore = forwardScore;
    }
    /**
     * @return Returns the posterior probability of this node.
     */
    public double getPosterior() {
        return posterior;
    }
    /**
     * @param posterior The node posterior probability to set.
     */
    public void setPosterior(double posterior) {
        this.posterior = posterior;
    }
    
    /**
     * @see java.lang.Object#hashCode()
     */
    public int hashCode() {
        return id.hashCode();
    }
    
    public void setBest (Word best) {
	this.best=best;
    }
    public Word getBest() {
	return best;
    }
    public boolean getIsNotPruned() {

	for (Edge e : getLeavingEdges())
            if (e.getNoPruned() ) return true;
	return getLeavingEdges().size()==0; //the last is not pruned
    }    
    /**
     * Assumes ids are unique node identifiers
     * 
     * @see java.lang.Object#equals(java.lang.Object)
     */
    public boolean equals(Object obj) {
        return id.equals(((Node)obj).getId());
    }

    /**
     * Calculates the begin time of this node, in the event that the
     * begin time was not specified. The begin time is the latest of the
     * end times of its predecessor nodes.
     */
    private void calculateBeginTime() {
        beginTime = 0;
        Iterator e = enteringEdges.iterator();
        while (e.hasNext()) {
            Edge edge = (Edge)e.next();
            if (edge.getFromNode().getEndTime() > beginTime) {
                beginTime = edge.getFromNode().getEndTime();
            }
        }
    }
    private void calculateEndTime() {
        endTime = -2;
        Iterator e = leavingEdges.iterator();
        while (e.hasNext()) {
            Edge edge = (Edge)e.next();
            if (edge.getToNode().getBeginTime()-1 >endTime) {
                endTime = edge.getToNode().getBeginTime()-1;
            }
        }
    }
        
    void setEndTime(int t) {
	getEndTime();
	if (endTime==-1)
	    endTime=t;
	else
	    throw new  Error("set endtime :" +endTime + " with "+ t);
    }

    /**
     * Get the nodes at the other ends of outgoing edges of this node.
     * 
     * @return a list of child nodes selon pruned
     */
    public List<Node> getChildNodes(boolean pruned) {
        LinkedList childNodes = new LinkedList();
        Iterator e = leavingEdges.iterator();
        while (e.hasNext()) {
            Edge edge = (Edge)e.next();
            if (pruned || edge.getNoPruned())
		childNodes.add(edge.getToNode());
        }
        return childNodes;
    }
    
    
    protected boolean isAncestorHelper(List<Node> children, Node node) {
        Iterator <Node> i = children.iterator();
        while(i.hasNext()) {
            Node n = i.next();
            if (parcours.contains(n)) continue;
            
            if (n.equals(node)) {
                return true;
            }
            parcours.add(n);
            if ( (n.getFirstEndTime() <= node.getLastBeginTime()) && 
		 isAncestorHelper(n.getChildNodes(false),node)) {
                return true;
            }
        }
        return false;
    }
    private static java.util.Set parcours =new java.util.HashSet<Node>();
    /**
     * Check whether this node is an ancestor of another node.
     * 
     * @param node the Node to check
     * @return whether this node is an ancestor of the passed in node.
     */
    public boolean isAncestorOf(NodeEdge n) {
	return isAncestorOf(n.getFromNode());
    }
    public Node getFromNode() {
	return this;
    }
    
    public boolean isAncestorOf(Node node) {
        if (this.equals(node)) {
            return true; // node is its own ancestor
        }
         parcours.clear();
        return (getFirstEndTime() <= node.getLastBeginTime()) &&  
	    isAncestorHelper(this.getChildNodes(false),node);
    }
    
    /**
     * Check whether this node has an ancestral relationship with another node
     * (i.e. either this node is an ancestor of the other node, or vice versa)
     * 
     * @param node the Node to check for a relationship
     * @return whether a relationship exists
     */
    public boolean hasAncestralRelationship(Node node) {
        return this.isAncestorOf(node) || node.isAncestorOf(this);
    }

    /**
     * Returns true if the given node is equivalent to this node.
     * Two nodes are equivalent only if they have the same word,
     * the same number of entering and leaving edges,
     * and that their begin and end times are the same.
     *
     * @param other the Node we're comparing to
     *
     * @return true if the Node is equivalent; false otherwise
     */
    public boolean isEquivalent(Node other) {
        return
            ((word.getSpelling().equals(other.getWord().getSpelling()) &&
              (getEnteringEdges().size() == other.getEnteringEdges().size() &&
               getLeavingEdges().size() == other.getLeavingEdges().size())) &&
             (beginTime == other.getBeginTime() &&
              endTime == other.getEndTime()));
    }

    /**
     * Returns a leaving edge that is equivalent to the given edge.
     * Two edges are eqivalent if Edge.isEquivalent() returns true.
     *
     * @param edge the Edge to compare the leaving edges of this node against
     *
     * @return an equivalent edge, if any; or null if no equivalent edge
     */
    public Edge findEquivalentLeavingEdge(Edge edge) {
        for (Iterator i = leavingEdges.iterator(); i.hasNext(); ) {
            Edge e = (Edge) i.next();
            if (e.isEquivalent(edge)) {
                return e;
            }
        }
        return null;
    }



    public void tri( int sens) {
	final int cle=sens;
	Collections.sort(enteringEdges,new Comparator <Edge> () {
		public int compare(Edge e1, Edge e2) {
		    return cle*(e1.getFromNode().getBeginTime()-e2.getFromNode().getBeginTime());
		}
	    });
	Collections.sort(leavingEdges,new Comparator <Edge> () {
		public int compare(Edge e1, Edge e2) {
		    return cle*(e2.getFromNode().getBeginTime()-e1.getFromNode().getBeginTime());
		}
	    });
	
    }
}