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
/**
 * Loads a grammar from a file representing a finite-state transducer (FSM) in
 * the 'ARPA' grammar format.
 *
 */
public class FSMGrammar extends Grammar {
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

    private Map<Integer,GrammarNode> nodes;
    private Set expandedNodes = new HashSet();
    private File extension;
    private float eps=0.0f;
    private float sanseps=0.0f;
    private float aveceps=0.0f;


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

    this.extension=SplitNameFile.splitNameFile(base,id,"saus");
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
       registry.register("eps", PropertyType.FLOAT);
       registry.register("aveceps", PropertyType.FLOAT);
       registry.register("sanseps", PropertyType.FLOAT);
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
	eps=ps.getFloat("eps",0.001f);
	sanseps=ps.getFloat("sanseps",0.001f);
	aveceps=ps.getFloat("aveceps",0.001f);

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
        nodes= new HashMap<Integer,GrammarNode>();

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
        ArrayList <String> list = new ArrayList<String>();
        BufferedReader reader = new BufferedReader(new FileReader(path));
        Pattern p=Pattern.compile("\\s+");
        String aline = null, line = null;
        GrammarNode initialNode=null;
	while ((line = reader.readLine()) != null) {
            if (line.length() > 0) {
		
		list.add(line);
                aline=line;
            }
        }
        reader.close();
        logger.info("!" + aline + "!");
        int numNode=Integer.parseInt(aline); //last line 
        int numAlpha=numNode;
        boolean [] epsNode=new boolean[numNode];
        int [] beginTime=new int[numNode];
	int [] endTime= new int[numNode];
        for (int iLigne=list.size()-2; iLigne >=0 ; iLigne--) {
	    String maligne= list.get(iLigne);
	    String l[]=p.split(maligne);
            if (l.length<3) {
		System.err.println("ligne courte "+maligne+"\n");
		continue;
	    }
            int id1 = Integer.parseInt(l[0]);
            int id2 = Integer.parseInt(l[1]);
            if (l[2].equals("timestamp") && l.length==5) {
		beginTime[id1]=Integer.parseInt(l[3]);
		endTime[id1]=Integer.parseInt(l[4]);
                continue;
	    }
	    float pr;
            float proba = logMath.linearToLog(pr=Float.parseFloat(l[3]));
	    proba =LogMath.getLogOne();
	    if (id2>=numNode) continue ;
            GrammarNode source =  get(id1);
            GrammarNode dest    = get(id2);
            if (l[2].equals("eps")) {
		if (pr<eps) continue;
		epsNode[id1]=true;    
	    }
	    else {
		if(epsNode[id1] && pr<aveceps) continue;//on est a l'envers il faut trier en sens inverse
		if(!epsNode[id1] && pr<sanseps) continue;

		GrammarNode node=createGrammarNode(numAlpha++,l[2]);
		if (l[2].equals("<s>")) initialNode= node;
		else
		    source.add(node,proba);
		if (l[2].equals("</s>")) dest.setFinalNode(true);
		//else 
		node.add(dest,logMath.getLogOne());
                int fils=id2;
		while (epsNode[fils]) {
                    fils=fils+1;
		    if  (beginTime[fils]< endTime[id1]+10) 
			node.add(get(fils),LogMath.getLogOne());
		    // la proba est a modifier par eps proba si on le fait
		}


	    }
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
    private GrammarNode get(int id) {
        
        GrammarNode grammarNode = nodes.get(id);
        if (grammarNode == null) {
            grammarNode = createGrammarNode(id, false);
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
