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

import java.io.IOException;
import java.io.BufferedReader;
import java.io.FileReader;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.List;
import java.io.File;
import java.util.Map;
import java.util.Set;
import java.util.regex.Pattern;
import edu.cmu.sphinx.linguist.dictionary.Dictionary;
import edu.cmu.sphinx.linguist.dictionary.Word;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.util.props.SetId;
import edu.cmu.sphinx.util.SplitNameFile;
import edu.cmu.sphinx.result.Lattice;
import edu.cmu.sphinx.result.LatticeOptimizerBis;
import edu.cmu.sphinx.result.Node;
import edu.cmu.sphinx.result.Edge;
/**
 * Loads a grammar from a file representing a finite-state transducer (FSM) in
 * the 'ARPA' grammar format.
 *
 */
public class LatticeGrammar extends Grammar implements SetId {
    /**
     * The SphinxProperty for the location of the FST n-gram file.
     */
    public final static String PROP_PATH = "path";
    /**
     * The default value for PROP_PATH.
     */
    public final static String PROP_PATH_DEFAULT = "fsmtest";
    
    
    /**
     * Sphinx property that defines the logMath component. 
     */
    
    public final static String PROP_LOG_MATH = "logMath";
    
    // TODO: If this property turns out to be worthwhile, turn this
    // into a full fledged sphinx property
    private boolean addInitialSilenceNode = false;
    
    // TODO: If this property turns out to be worthwhile, turn this
    // into a full fledged sphinx property
    
    // ------------------------------
    // Configuration data
    // -------------------------------
    
    private boolean addOptionalSilence = false;
    private boolean ignoreUnknownTransitions = true;
    private String path;
    private LogMath logMath;

    private Map<String,GrammarNode> nodes;
    private Set expandedNodes = new HashSet();
    private File extension;
    private float prune=0.0f;
    private float seuil=0.0f;
    private float wip=0.7f;
    private float lmWeight=9.5f;

    /**
     * Create class from reference text (not implemented).
     * 
     * @param bogusText
     *                dummy variable
     * 
     * @throws NoSuchMethogException
     *                 if called with reference sentence
     */
    protected GrammarNode createGrammar(String bogusText)
            throws NoSuchMethodException {
        throw new NoSuchMethodException("Does not create "
                + "grammar with reference text");
    }
    /***
     * prepare a charger un nouveau fichier fsm
     * extension nom du fichier
     */

  private String id="";
    public void setId(String id) throws IOException {
	setId(path,id);
    }
    public void setId(String base, String id) throws IOException 
    { this.id=id; 

    this.extension=SplitNameFile.splitNameFile(base,id,".lat.gz");
    this.newGrammar();
    }


   public void dumpGrammar(String name) {
        getInitialNode().dumpGDL(name+id);
    }






    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
            throws PropertyException {
        super.register(name, registry);
        registry.register(PROP_PATH, PropertyType.STRING);
       registry.register("prune", PropertyType.FLOAT);
       registry.register("seuil", PropertyType.FLOAT);
       registry.register("wip", PropertyType.FLOAT);
       registry.register("lmWeight", PropertyType.FLOAT);

       
        registry.register(PROP_LOG_MATH, PropertyType.COMPONENT);
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        super.newProperties(ps);
        path = ps.getString(PROP_PATH, PROP_PATH_DEFAULT);
	prune=ps.getFloat("prune",30f);
	seuil=ps.getFloat("seuil",0.001f);
	wip=ps.getFloat("wip",0.7f);
	lmWeight=ps.getFloat("lmWeight",9.5f);
        logMath = (LogMath) ps.getComponent(PROP_LOG_MATH, LogMath.class);
    }

    /**
     * Creates the grammar.
     * 
     * @return the initial node for the grammar.
     */
    protected GrammarNode createGrammar()
	throws IOException {

	GrammarNode initialNode = null;
	GrammarNode finalNode = null;
        nodes= new HashMap<String,GrammarNode>();

	// first pass create the FST nodes
	 initialNode =createNodes(extension);

	// create the final node:
	//	finalNode = createGrammarNode(++maxNodeId, Dictionary.SILENCE_SPELLING);
        //finalNode.setFinalNode(true);

	// replace each word node with a pair of nodes, which
	// consists of the word node and a new dummy end node, which is
        // for adding null or backoff transitions
	//maxNodeId = expandWordNodes(maxNodeId);
	return initialNode;
    }
    /**
     * Reads the FST file in the given path, and creates the nodes in the FST
     * file.
     * 
     * @param path
     *                the path of the FST file to read
     * 
     * @return the highest ID of all nodes
     */
    private GrammarNode createNodes(File path) throws IOException  {
	GrammarNode initialNode;
	Lattice lat= new Lattice(logMath,getDictionary(),path);
	lat.computeNodePosteriors(lmWeight,false,prune,seuil,wip,null,lmWeight);
	LatticeOptimizerBis op = new LatticeOptimizerBis(lat);
	op.optimize(); //on peut optimiser mais il faut refaire et c'est pas simple
	int maxi=0;
	for (Node n : lat.getNodes()) {
	    int val =n.getIdInt();
	    maxi=Math.max(val,maxi);   
	    GrammarNode m=  createGrammarNode(val,n.getWord());
	    nodes.put(n.getId(),m);
	    logger.fine("n:" +get(n.getId())+ "  m:"+ m.isEmpty() +" " + get(n.getId()).isEmpty()+"n ori:" +n.getWord() );
	}
	initialNode= get(lat.getInitialNode().getId());
	logger.info("init" + initialNode);
	GrammarNode leDernier=createGrammarNode(maxi+1,false);
	leDernier.setFinalNode(true);
	get(lat.getTerminalNode().getId()).add(leDernier, LogMath.getLogOne() );        
	for (Edge e : lat.getEdges()) 
	    if (e.getIsNotPruned()) {
		get(e.getFromNode().getId()).add(get(e.getToNode().getId()),
						 LogMath.getLogOne());
	    }
	return initialNode;
    }


   


    /**
     * Expand each of the word nodes into a pair of nodes, as well as adding an
     * optional silence node between the grammar node and its end node.
     * 
     * @param maxNodeID
     *                the node ID to start with for the new nodes
     * 
     * @return the last (or maximum) node ID
     */
    private int expandWordNodes(int maxNodeID) {
        Collection allNodes = nodes.values();
        String[][] silence = { {Dictionary.SILENCE_SPELLING}};
        for (Iterator i = allNodes.iterator(); i.hasNext();) {
            GrammarNode node = (GrammarNode) i.next();
            // if it has at least one word, then expand the node
            if (node.getNumAlternatives() > 0) {
                GrammarNode endNode = createGrammarNode(++maxNodeID, false);
                node.add(endNode, LogMath.getLogOne());
                // add an optional silence
                if (addOptionalSilence) {
                    GrammarNode silenceNode = createGrammarNode(++maxNodeID,
                            silence);
                    node.add(silenceNode, LogMath.getLogOne());
                    silenceNode.add(endNode, LogMath.getLogOne());
                }
                expandedNodes.add(node);
            }
        }
        return maxNodeID;
    }

    /**
     * Converts the probability from -ln to logmath
     * 
     * @param lnProb
     *                the probability to convert. Probabilities in the arpa
     *                format in negative natural log format. We convert them to
     *                logmath.
     * 
     * @return the converted probability in logMath log base
     */
    private float convertProbability(float lnProb) {
        return logMath.lnToLog(-lnProb);
    }

    /**
     * Given an id returns the associated grammar node
     * 
     * @param id
     *                the id of interest
     * 
     * @return the grammar node or null if none could be found with the proper
     *         id
     */
    private GrammarNode get(String id) {
        
        GrammarNode grammarNode = nodes.get(id);
        if (grammarNode == null) {
            grammarNode = createGrammarNode(Integer.parseInt(id), false);
            nodes.put(id, grammarNode);
        }
        return grammarNode;
    }

    /**
     * Determines if the node has a word
     * 
     * @param node
     *                the grammar node of interest
     * 
     * @return true if the node has a word
     *  
     */
    private boolean hasWord(GrammarNode node) {
        return (node.getNumAlternatives() > 0);
    }

    /**
     * Gets the word from the given grammar ndoe
     * 
     * @param node
     *                the node of interest
     * 
     * @return the word (or null if the node has no word)
     */
    private String getWord(GrammarNode node) {
        String word = null;
        if (node.getNumAlternatives() > 0) {
            Word[][] alternatives = node.getAlternatives();
            word = alternatives[0][0].getSpelling();
        }
        return word;
    }

    /**
     * Determines if the given node has an end node associated with it.
     * 
     * @param node
     *                the node of interest
     * 
     * @return <code>true</code> if the given node has an end node.
     */
    private boolean hasEndNode(GrammarNode node) {
        return (expandedNodes.contains(node));
    }

    /**
     * Retrieves the end node associated with the given node
     * 
     * @param node
     *                the node of interest
     * 
     * @return the ending node or null if no end node is available
     */
    private GrammarNode getEndNode(GrammarNode node) {
	GrammarArc[] arcs = node.getSuccessors();
	assert arcs != null && arcs.length > 0;
	return arcs[0].getGrammarNode();
    }
}
