

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

package edu.cmu.sphinx.linguist.language.grammar;

import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;
import java.io.IOException;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.util.props.SetId;
/**
 * Creates a grammar from a reference sentence. It is a constrained
 * grammar that represents the sentence only.
 *
 * Note that all grammar probabilities are maintained in the LogMath
 * log base
 */
public class ForcedAlignerGrammar extends Grammar implements SetId {
    private String id="";
    GrammarNode firstNode=null;;
    public void setId(String id) throws IOException {
	this.id=id;
	this.newGrammar();
    }
    public void setId(String base, String id) throws IOException 
    { throw new Error("pas implemete");
    }

 

    private Map<String,GrammarNode> nodes = new HashMap<String,GrammarNode>();
    /**
     * Create class from reference text (not implemented).
     *
     */
    protected GrammarNode createGrammar()
    {return createGrammarLocal("<s> " + id + " </s>");
    }
    /**
     * Creates the grammar
     *
     */
    protected GrammarNode createGrammarLocal(String referenceText)
        {
	GrammarNode initialNode = null;
	GrammarNode finalNode = null;
	final float logArcProbability = LogMath.getLogOne();
	String nodeName;

	StringTokenizer tok = new StringTokenizer(referenceText);
	int nodeId = 0;

	// first pass just creates the grammar nodes

	// Create initial node
	logger.info("la phrase "+ referenceText);
	//	int initialID = nodeId++;
        //nodeName = "G" + initialID;
	//initialNode = createGrammarNode(initialID, false);
	//assert initialNode != null;
	//nodes.put(nodeName, initialNode);
	int myMilieu=milieu; GrammarNode nodeMilieu;
	// Create a node for each word in the sentence
	while (tok.hasMoreTokens()) {

	    String token;
	    token = tok.nextToken();
	    if (myMilieu>0 && token.equals("motInconnu")) {
		    myMilieu=nodeId;
		    continue;
		}
	    nodeName = "G" + nodeId;
	    GrammarNode node = (GrammarNode) nodes.get(nodeName);
	    if (node == null) {
		if (true) {
		    logger.fine("Creating "
				       + nodeName + " word is " 
				       + token+ "  id:" + nodeId);
		}
		node = createGrammarNode(nodeId, token);
		nodes.put(nodeName, node);
	    } 
	    nodeId++;
	}
	
	    
	initialNode= nodes.get("G0");
	// create the final node
	finalNode = get(nodeId);
	finalNode.setFinalNode(true);// j'insiste
	// Now that we have all the grammar nodes, reprocess the nodes

	// Reprocessing is simply adding arcs to nodes, in sequence, with
	// probability of one (linear scale).

	// Second pass, add all of the arcs

	for (int i = 0; i < nodeId; i++) {
	    if (i==myMilieu-1) continue;
	    int thisID = i;
	    int nextID = i + 1;

	    GrammarNode thisNode = get(thisID);
	    GrammarNode nextNode = get(nextID);

	    thisNode.add(nextNode, logArcProbability);
	}
	if (myMilieu>0) {
	    int p=myMilieu-1;
	    GrammarNode thisNode = get(p);
	    GrammarNode nextNode = get(p+1);
	    GrammarNode node = createGrammarNode(-100, "ph100");
	    logger.info("creating" + node);
	    thisNode.add(node, logArcProbability);
	    node.add(nextNode, logArcProbability);

	}

	
	nodes.clear();
	return initialNode;
    }



    /**
     * Given an id returns the associated grammar node
     *
     * @param id the id of interest
     *
     * @return the grammar node or null if none could be found with
     * the proper id
     */
    private GrammarNode get(int id) {
	String name = "G" + id;
	GrammarNode grammarNode =  (GrammarNode) nodes.get(name);
	if (grammarNode == null) {
	    grammarNode = createGrammarNode(id, false);
	    nodes.put(name, grammarNode);
	}

	return grammarNode;
    }
}
