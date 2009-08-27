
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
package edu.paul.lattice;

import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.StringTokenizer;
import  edu.cmu.sphinx.linguist.dictionary.Word;
/**
 * Edges are part of Lattices.  They connect Nodes, and contain the score
 * associated with that sequence.
 */

public class Edge  {
    protected double acousticScore;
    protected double lmScore;
    protected Node fromNode;
    protected Node toNode;
    protected double pscore;
    protected boolean noPruned;
    protected int beginTime=-1;
    protected int endTime=-1;
    /**
     * Create an Edge from fromNode to toNode with acoustic and
     * Language Model scores.
     *
     * @param fromNode
     * @param toNode
     * @param acousticScore
     * @param lmScore
     */
    protected Edge( Node fromNode, Node toNode,
                    double acousticScore, double lmScore ) {
        this.acousticScore = acousticScore;
        this.lmScore = lmScore;
        this.fromNode = fromNode;
        this.toNode = toNode;
    }

    public String toString() {
        return "Edge(" + fromNode + "-->" + toNode + "[" + acousticScore
                + "," + lmScore +","+pscore+ "])";
    }
    /**
     * Internal routine used when dumping a Lattice as a .LAT file
     * @param f
     * @throws IOException
     */
    void dump(PrintWriter f) { //  throws IOException {
        f.println( "edge: " + fromNode.getId() + " " + toNode.getId() );
    }

    /**
     * Internal routine used when dumping a Lattice as an AiSee file
     * @param f
     * @throws IOException
     */
    void dumpAISee(FileWriter f) throws IOException {
        f.write( "edge: { sourcename: \"" + fromNode.getId()
                + "\" targetname: \"" + toNode.getId()
                + "\" label: \"" + acousticScore + "," + lmScore + "\" }\n" );
    }

    /**
     * Get the acoustic score associated with an Edge
     * @return the score
     */
    public double getAcousticScore() {
        return acousticScore;
    }

    /**
     * Get the language model score associated with an Edge
     * @return the score
     */
    public double getLMScore() {
        return lmScore;
    }

    /**
     * Get the "from" Node associated with an Edge
     * @return the Node
     */
    public Node getFromNode() {
        return fromNode;
    }

    /**
     * Get the "to" Node associated with an Edge
     * @return the Node
     */
    public Node getToNode() {
        return toNode;
    }
    public int getBeginTime(){
	if (beginTime!=-1)
	    return beginTime;
	return beginTime=getFromNode().getBeginTime();
    }
  public int getEndTime(){
	if (endTime!=-1)
	    return endTime;
	return endTime=getToNode().getBeginTime();
    }
    public Word  getWord() {
	return getFromNode().getWord();
    }
    public void setPscore(double p) {
	pscore=p;
    }
    public double getPosterior(){
	return pscore;
    }
    public double getPscore() {
	return pscore;
    }
    public int getFirstEndTime() {
	return getEndTime();
    }

    /**
     * Sets the acoustic score
     *
     * @param v the acoustic score.
     */
    public void setAcousticScore(double v) {
        acousticScore = v;
    }

    /**
     * Sets the language model score 
     *
     * @param v the lm score.
     */
    public void setLMScore(double v) {
        lmScore = v;
    }
    public void setNoPruned( boolean b) {
	this.noPruned=b;
    }
    public boolean getNoPruned() {
	return noPruned;
    }
    public boolean getIsNotPruned(){
	return noPruned;
    }

    /**
     * Returns true if the given edge is equivalent to this edge.
     * Two edges are equivalent only if they have their 'fromNode'
     * and 'toNode' are equivalent, and that their acoustic and language
     * scores are the same.
     *
     * @param other the Edge to compare this Edge against
     *
     * @return true if the Edges are equivalent; false otherwise
     */
    public boolean isEquivalent(Edge other) {
        /*
         * TODO: Figure out why there would be minute differences
         * in the acousticScore. Therefore, the equality of the acoustic
         * score is judge based on whether the difference is bigger than 1.
         */
        double diff = Math.abs(acousticScore) * 0.00001;
        return ((Math.abs(acousticScore - other.getAcousticScore()) <= diff &&
                 lmScore == other.getLMScore()) &&
                (fromNode.isEquivalent(other.getFromNode()) &&
                 toNode.isEquivalent(other.getToNode())));
    }
}
