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

import java.util.Iterator;
import java.util.Vector;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Collections;
import java.util.Comparator;

/**
 * Class used to collapse all equivalent paths in a Lattice.  Results in a
 * Lattices that is deterministic (no Node has Edges to two or more
 * equivalent Nodes), and minimal (no Node has Edge from two or more
 * equivalent Nodes).
 */

public class LatticeOptimizerBis {
    protected Lattice lattice;
    private LinkedList<Node>  mesNoeuds;
    private boolean sansPrune=false;
    private boolean pourSaucisse=false;
    /**
     * Create a new Lattice optimizer
     *
     * @param lattice
     */

    public  LatticeOptimizerBis(Lattice lattice, boolean sansPrune){
	this(lattice);
	this.sansPrune=sansPrune;
    }
    public LatticeOptimizerBis(Lattice lattice) {
        this.lattice = lattice;
	lattice.lesNoeuds=null;
	mesNoeuds= new  LinkedList<Node>(lattice.getNodes());
	Collections.sort(mesNoeuds,new Comparator<Node>() {
			     public int compare(Node n1,Node n2) {
				 if (n1.getBeginTime()==n2.getBeginTime())
				     return  n1.getEndTime()- n2.getEndTime();
				 return n1.getBeginTime()- n2.getBeginTime();
			     }
			 });
	
    }

    private boolean  verifPost ( Node n) {
	double cumul=0.0;
	for (Edge e : n.getEnteringEdges())
	    cumul += lattice.getLogMath().logToLinear((float) e.getPscore());
	double prevu= lattice.getLogMath().logToLinear((float)n.getPosterior());
	if ((Math.abs(cumul-prevu)/(0.0001+ prevu))>0.01 &&(sansPrune || prevu>0.005)) {
	    System.err.format("verifPost :%s  %f %f\n",n.toString(),cumul,prevu);
	    return false;}
	return true;
    }
    /**
     * Code for optimizing Lattices.  An optimal lattice has all the same
     * paths as the original, but with fewer nodes and edges
     *
     * Note that these methods are all in Lattice so that it is easy to
     * change the definition of "equivalent" nodes and edges.  For example,
     * an equivalent node might have the same word, but start or end at a
     * different time.
     *
     * To experiment with other definitions of equivalent, just create a
     * superclass of Lattice.
     */
    private void deleteNoFinal() {
	// do after prune
	ListIterator<Node>  it= mesNoeuds.listIterator(mesNoeuds.size());
	if  (it.previous()!= lattice.getTerminalNode())
	    throw new Error("deleteNofinal");
	while (it.hasPrevious()) {
	    Node currentNode= it.previous();
	    if (currentNode.getLeavingEdges().size()<=0)
		{ //System.err.println(currentNode);
		    lattice.removeNodeAndEdges(currentNode);
		    it.remove();
		  
		}
	}
    }
    private void deleteNoInit() {

	ListIterator<Node>  it= mesNoeuds.listIterator(0);
	if (it.next()!= lattice.getInitialNode())
	    throw new Error("deleteNoinit");
	while (it.hasNext()) {
	    Node currentNode= it.next();
	    if (currentNode.getEnteringEdges().size()<=0)
		{   //System.err.println(currentNode);
		    lattice.removeNodeAndEdges(currentNode);
		    it.remove();
		}
	}
    
    }
	
    private void voirFin(String lab) {
	if (true) return;
	System.err.println(lab+ sansPrune+ "    start");
	if (true) {
	ListIterator<Node>  it= mesNoeuds.listIterator(mesNoeuds.size());
	int n=10;
	while (n-->0 && it.hasPrevious()) {
	    System.err.print( n+ ": "+it.previous()+ ",");
	    
	}
	System.err.println();
	}
	if (true) {

	for (Node s : mesNoeuds)
	    verifPost(s);
	}
		System.err.println(lab+ sansPrune+ "    fin");
    }

    static int comp=0;
    public void optimize() {
	//        System.err.println("***");
	//         lattice.dumpAllPaths();
	//         System.err.println("***");
	voirFin("start");
	// lattice.dumpAISee("trucForav"+sansPrune+".gdl","ggggg ",true);
	prune();
	System.err.println("prune ed : " + lattice.getEdges().size()+ " n:" + lattice.getNodes().size());
	voirFin("prune");
	//if (!sansPrune) lattice.dumpAISee("trucFor.gdl","ggggg ",true);
	deleteNoInit();
	System.err.println("final ed : " + lattice.getEdges().size()+ " n:" + lattice.getNodes().size());
	voirFin("noInit");
	deleteNoFinal();
	voirFin("noFinal");
        optimizeForward();
	//	lattice.dumpAISee("trucFor.gdl","ggggg ");
        System.err.println("for ed :" + lattice.getEdges().size()+ " n:" + lattice.getNodes().size());
	//         lattice.dumpAllPaths();
	//         System.err.println("***");
        voirFin("forWard");
        optimizeBackward();
	
        System.err.println("back ed :" + lattice.getEdges().size()+ " n:" + lattice.getNodes().size());
	//         lattice.dumpAllPaths();
	//         System.err.println("***");
	voirFin("backWard");
       	//lattice.dumpAISee("trucFinal"+comp+"_"+sansPrune+ ".gdl","ggggg ",sansPrune);
	//if (!sansPrune) comp++;
    }


    void prune() {
	if (!sansPrune)
	for (Edge e2 : lattice.getCopyEdges())
	    if (!e2.getIsNotPruned())
		{
		    e2.getToNode().removeEnteringEdge(e2);
		    e2.getFromNode().removeLeavingEdge(e2);
		    lattice.removeEdge(e2);
		}
    }

    /**
     * Make the Lattice deterministic, so that no node
     * has multiple outgoing edges to equivalent nodes.
     *
     * Given two edges from the same node to two equivalent nodes,
     * replace with one edge to one node with outgoing edges
     * that are a union of the outgoing edges of the old two nodes.
     *
     *  A --> B --> C
     *   \--> B' --> Y
     *
     *  where B and B' are equivalent.
     *
     *  is replaced with
     *
     *  A --> B" --> C
     *         \--> Y
     *
     *  where B" is the merge of B and B'
     *
     *  Note that equivalent nodes must have the same incomming edges.
     *  For example
     *
     *  A --> B
     *    \
     *     \
     *  X --> B'
     *
     *  B and B' would not be equivalent because the incomming edges
     *  are different
     */
    private int count=0;
    protected void optimizeForward() {


        boolean moreChanges = true;
	if (false) Collections.sort(mesNoeuds,new Comparator<Node>() {
			     public int compare(Node n1,Node n2) {
				 if (n1.getEndTime()==n2.getEndTime())
				     return  n1.getBeginTime()- n2.getBeginTime();
				 return n1.getEndTime()- n2.getEndTime();
			     }
			 });


        while (moreChanges) {
	count++;
        //System.err.println("*** Optimizing forward *** " + lattice.getEdges().size()+ " n:" + lattice.getNodes().size());           
	moreChanges = false;
            // search for a node that can be optimized
            // note that we use getCopyOfNodes to avoid concurrent changes to nodes
	for (ListIterator <Node> i = mesNoeuds.listIterator(0); i.hasNext();) {
                Node n =  i.next();

                // we are iterating down a list of node before optimization
                // previous iterations may have removed nodes from the list
                // therefore we have to check that the node stiff exists
                if (lattice.hasNode(n.getId())) {
                    moreChanges |= optimizeNodeForward(n);
                }
		else i.remove();
            }
        }
    }


    /**
     * Look for 2 "to" edges to equivalent nodes.  Replace the edges
     * with one edge to one node that is a merge of the equivalent nodes
     *
     * nodes are equivalent if they have equivalent from edges, and the
     * same label
     *
     * merged nodes have a union of "from" and "to" edges
     *
     * @param n
     * @return true if Node n required an optimize forward
     */
    protected boolean optimizeNodeForward(Node n) {
        assert lattice.hasNode(n.getId());
	boolean retour=false;
        Vector leavingEdges = new Vector(n.getLeavingEdges());
	boolean valide[]=new boolean[leavingEdges.size()];
	java.util.Arrays.fill(valide,true);
	//System.err.println(" vire les fils de "+ n);
        for (int j = 0; j < leavingEdges.size(); j++) 
	    if (valide[j])
		{
		    Edge e = (Edge) leavingEdges.elementAt(j);
		    for (int k = j + 1; k < leavingEdges.size(); k++)
			if (valide[k] ){
			    Edge e2 = (Edge) leavingEdges.elementAt(k);
			    
			    /*
			     * If these are not the same edge, and they point to
			     * equivalent nodes, we have a hit, return true
			     */
			    assert e != e2;
			    if (true && !lattice.hasNode(e2.getToNode().getId()) ) {valide[k]=false;
				//je l'ai deja vu par un autre arc ??????
				//System.err.println(e2+ " oh " + e2.getToNode()+ " " + j +" "+k);
			    }else 
			    if (equivalentNodesForward(e.getToNode(), e2.getToNode())) {
				mergeNodesAndEdgesForward(n, e, e2);
				valide[k]=false;
				//System.err.println("virer" + j+ " " +k );
				if (false && count >=20) System.err.println("e: "+ e +" e2 "+ e2);
				retour=true;
                }
            }
        }
        /*
         * return false if we did not get a hit
         */
        return retour;
    }

    /**
     * nodes are equivalent forward if they have "from" edges from the same
     * nodes, and have equivalent labels (Token, start/end times)
     *
     * @param n1
     * @param n2
     * @return true if n1 and n2 are "equivalent forwards"
     */
    protected boolean equivalentNodesForward(Node n1, Node n2) {

        assert lattice.hasNode(n1.getId());
        assert lattice.hasNode(n2.getId());

        // do the labels match?
        if (!equivalentNodeLabels(n1, n2)) return false;

        // if they have different number of "from" edges they are not equivalent
        // or if there is a "from" edge with no match then the nodes are not
        // equivalent
        return n1.hasEquivalentEnteringEdges(n2);
    }

    /**
     * given edges e1 and e2 from node n to nodes n1 and n2
     *
     * merge e1 and e2, that is, merge the scores of e1 and e2
     * create n' that is a merge of n1 and n2
     * add n'
     * add edge e' from n to n'
     *
     * remove n1 and n2 and all associated edges
     *
     * @param n
     * @param e1
     * @param e2
     */
    protected void mergeNodesAndEdgesForward(Node n, Edge e1, Edge e2) {
        assert lattice.hasNode(n.getId());
        assert lattice.hasEdge(e1);
        assert lattice.hasEdge(e2);

        assert e1.getFromNode() == n;
        assert e2.getFromNode() == n;

        Node n1 = e1.getToNode();
        Node n2 = e2.getToNode();
	boolean b=false ; //verifPost(n1) && verifPost(n2);
	//        if (n.getBeginTime() >=245)
	//  System.err.println(n  +"\n-----------"+ n1+" :"+ e1  + "\n----------"+n2+ " :"+ e2);
        assert n1.hasEquivalentEnteringEdges(n2); //je crois n2 et pas n1
        assert n1.getWord().equals(n2.getWord());

       
        // merge the scores of e1 and e2 into e1
       //
	e1.setAcousticScore(mergeAcousticScores
                            (e1.getAcousticScore(), e2.getAcousticScore()));
        e1.setLMScore(mergeLanguageScores(e1.getLMScore(),
                                          e2.getLMScore()));
        
	if (n1.equals(n2) || (!sansPrune && pourSaucisse))  /// pour les tests le sansprune
	    //cela est faux dans le cas general
	    e1.setPscore(mergePScores(e1.getPscore(),
				      e2.getPscore()));
	else
	    n1.fusionEntree(n2,lattice.getLogMath());

	if (n1.equals(n2)) {
              e2.getToNode().removeEnteringEdge(e2);
	      e2.getFromNode().removeLeavingEdge(e2);
	      lattice.removeEdge(e2) ;
	      if ( false && b && ! verifPost(n1)) {
		  System.err.format(" j'ai un biais mono: %s  %s \n",n1.toString(), " " + e2);
	      }
	      return;} // c'est le final et .... 
        // add n2's edges to n1
        for (Iterator i = n2.getLeavingEdges().iterator(); i.hasNext();) {
            Edge e = (Edge) i.next();
            e2 = n1.getEdgeToNode( e.getToNode() );
            if ( e2 == null ) {
                e2=lattice.addEdge(n1, e.getToNode(),
				   e.getAcousticScore(), e.getLMScore());
		e2.setPscore(e.getPscore());
		e2.setNoPruned(true&& !sansPrune);
		e2=null;
            } else {
                // if we got here then n1 and n2 had edges to the same node
                // choose the edge with best score
                e2.setAcousticScore
                    (mergeAcousticScores
                     (e.getAcousticScore(), e2.getAcousticScore()));
                e2.setLMScore(mergeLanguageScores(e.getLMScore(), 
                                                  e2.getLMScore())) ;
                e2.setPscore(mergePScores(e.getPscore(), 
                                                  e2.getPscore())) ;

            }
        }
	n1.setPosterior(mergePScores(n1.getPosterior(),n2.getPosterior()));
	if ( false && b && ! verifPost(n1)) {
		System.err.format(" j'ai un biais ici: %s %s %s \n",n1.toString(), n2.toString(), " " + e2);
	    }
        // remove n2 and all associated edges
        lattice.removeNodeAndEdges(n2);
	//System.err.print(" je vire :" +n2);
    }


    /**
     * Minimize the Lattice deterministic, so that no node
     * has multiple incomming edges from equivalent nodes.
     *
     * Given two edges from equivalent nodes to a single nodes,
     * replace with one edge from one node with incomming edges
     * that are a union of the incomming edges of the old two nodes.
     *
     *  A --> B --> C
     *  X --> B' --/
     *
     *  where B and B' are equivalent.
     *
     *  is replaced with
     *
     *  A --> B" --> C
     *  X --/
     *
     *  where B" is the merge of B and B'
     *
     *  Note that equivalent nodes must have the same outgoing edges.
     *  For example
     *
     *  A --> X
     *    \
     *     \
     *      \
     *  A' --> B
     *
     *  A and A' would not be equivalent because the outgoing edges
     *  are different
     */
    protected void optimizeBackward() {
        //System.err.println("*** Optimizing backward ***");

        boolean moreChanges = true;
        while (moreChanges) {
            moreChanges = false;
            // search for a node that can be optimized
            // note that we use getCopyOfNodes to avoid concurrent changes to nodes
            for (ListIterator<Node> i = mesNoeuds.listIterator(mesNoeuds.size()); i.hasPrevious();) {
                Node n =  i.previous();

                // we are iterating down a list of node before optimization
                // previous iterations may have removed nodes from the list
                // therefore we have to check that the node stiff exists
                if (lattice.hasNode(n.getId())) {
                    moreChanges |= optimizeNodeBackward(n);
		}
		else
		    i.remove();
            }
        }
    }

    /**
     * Look for 2 entering edges from equivalent nodes.  Replace the edges
     * with one edge to one new node that is a merge of the equivalent nodes
     * Nodes are equivalent if they have equivalent to edges, and the same 
     * label. Merged nodes have a union of entering and leaving edges
     *
     * @param n
     * @return true if Node n required opimizing backwards
     */
    protected boolean optimizeNodeBackward(Node n) {
	boolean retour=false;
        Vector enteringEdges = new Vector(n.getEnteringEdges());
	boolean valide[]=new boolean[enteringEdges.size()];
	java.util.Arrays.fill(valide,true);
        for (int j = 0; j < enteringEdges.size(); j++) 
	    if (valide[j] )
		{
		    Edge e = (Edge) enteringEdges.elementAt(j);
		    for (int k = j + 1; k < n.getEnteringEdges().size(); k++)
			if (valide[k]) {
			    Edge e2 = (Edge) enteringEdges.elementAt(k);
			    
			    /*
			     * If these are not the same edge, and they point to
			     * equivalent nodes, we have a hit, return true
			     */
			    assert e != e2;
			    if (true && ! lattice.hasNode(e2.getFromNode().getId()))
				valide[k] =false;
			    else
			    if (equivalentNodesBackward(e.getFromNode(),
							e2.getFromNode())) {
				mergeNodesAndEdgesBackward(n, e, e2);
				valide[k]=false;
				retour= true;
                }
            }
        }
        /*
         * return false if we did not get a hit
         */
        return retour;
    }

    /**
     * nodes are equivalent backward if they have "to" edges to the same nodes,
     * and have equivalent labels (Token, start/end times)
     *
     * @param n1
     * @param n2
     * @return true if n1 and n2 are "equivalent backwards"
     */
    protected boolean equivalentNodesBackward(Node n1, Node n2) {

        assert lattice.hasNode(n1.getId());
        assert lattice.hasNode(n2.getId());

        // do the labels match?
        if (!equivalentNodeLabels(n1, n2)) return false;

        // if they have different number of "to" edges they are not equivalent
        // or if there is a "to" edge with no match then the nodes are not equiv
        return n1.hasEquivalentLeavingEdges(n2);
    }

    /**
     * Is the contents of these Node equivalent?
     *
     * @param n1
     * @param n2
     * @return true if n1 and n2 have "equivalent labels"
     */
    protected boolean equivalentNodeLabels(Node n1, Node n2) {
        return (n1.getWord().equals(n2.getWord()));
		// &&
		// (n1.getBeginTime() == n2.getBeginTime() &&
                // n1.getEndTime() == n2.getEndTime()));
    }

    /**
     * given edges e1 and e2 to node n from nodes n1 and n2
     *
     * merge e1 and e2, that is, merge the scores of e1 and e2
     * create n' that is a merge of n1 and n2
     * add n'
     * add edge e' from n' to n
     *
     * remove n1 and n2 and all associated edges
     *
     * @param n
     * @param e1
     * @param e2
     */
    protected void mergeNodesAndEdgesBackward(Node n, Edge e1, Edge e2) {
        assert lattice.hasNode(n.getId());
        assert lattice.hasEdge(e1);
        assert lattice.hasEdge(e2);

        assert e1.getToNode() == n;
        assert e2.getToNode() == n;

        Node n1 = e1.getFromNode();
        Node n2 = e2.getFromNode();

        assert n1.hasEquivalentLeavingEdges(n2);
        assert n1.getWord().equals(n2.getWord());

        // merge the scores of e1 and e2 into e1
        e1.setAcousticScore(mergeAcousticScores(e1.getAcousticScore(),
                                                e2.getAcousticScore()));
        e1.setLMScore(mergeLanguageScores(e1.getLMScore(),
                                          e2.getLMScore()));

	if (n1.equals(n2)|| (!sansPrune && pourSaucisse))  // test only a enlever en Prod (le sansPrune)
	    e1.setPscore(mergePScores(e1.getPscore(),
                                          e2.getPscore()));
	else n1.fusionSortie(n2,lattice.getLogMath());

        // add n2's "from" edges to n1
	if (n1.equals(n2)) {
	    e2.getToNode().removeEnteringEdge(e2);
	    e2.getFromNode().removeLeavingEdge(e2);
	    lattice.removeEdge(e2);return;}
        for (Iterator i = n2.getEnteringEdges().iterator(); i.hasNext();) {
            Edge e = (Edge) i.next();
            e2 = n1.getEdgeFromNode( e.getFromNode() );
            if ( e2 == null ) {
                e2=lattice.addEdge(e.getFromNode(), n1,
				   e.getAcousticScore(), e.getLMScore());
		e2.setPscore(e.getPscore());
		e2.setNoPruned(true&&!sansPrune);
		e2=null;
            } else {
                // if we got here then n1 and n2 had edges from the same node
                // choose the edge with best score
                e2.setAcousticScore
                    (mergeAcousticScores(e.getAcousticScore(),
                                         e2.getAcousticScore()));
                e2.setLMScore(mergeLanguageScores(e.getLMScore(),
                                                  e2.getLMScore()));
               e2.setPscore(mergePScores(e.getPscore(),
                                                  e2.getPscore()));
  
           }
        }
	n1.setPosterior(mergePScores(n1.getPosterior(),n2.getPosterior()));
        // remove n2 and all associated edges
        lattice.removeNodeAndEdges(n2);
    }

    /**
     * Remove all Nodes that have no Edges to them (but not <s>)
     */
    protected void removeHangingNodes() {
        for (Iterator i = lattice.getCopyOfNodes().iterator(); i.hasNext();) {
            Node n = (Node) i.next();
            if (lattice.hasNode(n.getId())) {
                if (n == lattice.getInitialNode()) {

                } else if (n == lattice.getTerminalNode()) {

                } else {
                    if (n.getLeavingEdges().size() == 0
                        || n.getEnteringEdges().size() == 0) {
                        lattice.removeNodeAndEdges(n);
                        removeHangingNodes();
                        return;
                    }
                }
            }
        }
    }

    /**
     * Provides a single method to merge acoustic scores, so that changes
     * to how acoustic score are merged can be made at one point only.
     *
     * @param score1 the first acoustic score
     * @param score2 the second acoustic score
     *
     * @return the merged acoustic score
     */
    private double mergeAcousticScores(double score1, double score2) {
        // return lattice.getLogMath().addAsLinear(score1, score2);
        return Math.max(score1, score2);
    }
    private double mergePScores(double score1, double score2) {
	return lattice.getLogMath().addAsLinear((float)score1,(float) score2);
    }

    /**
     * Provides a single method to merge language scores, so that changes
     * to how language score are merged can be made at one point only.
     *
     * @param score1 the first language score
     * @param score2 the second language score
     *
     * @return the merged language score
     */
    private double mergeLanguageScores(double score1, double score2) {
        // return lattice.getLogMath().addAsLinear(score1, score2);
        return Math.max(score1, score2);
    }

    /**
     * Self test for LatticeOptimizer
     *
     * @param args
     */
    public static void main(String[] args) {
        Lattice lattice = new Lattice(args[0]);

        LatticeOptimizer optimizer = new LatticeOptimizer(lattice);

        optimizer.optimize();

        lattice.dump(args[1]);
    }
}
