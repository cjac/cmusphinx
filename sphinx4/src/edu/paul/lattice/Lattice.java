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

import java.io.InputStreamReader;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.OutputStreamWriter;
import java.io.File;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.io.IOException;
import java.io.LineNumberReader;
import java.io.PrintWriter;
import java.util.Collection;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.ArrayList;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;
import java.util.Set;
import java.util.StringTokenizer;
import java.util.Vector;
import edu.cmu.sphinx.result.Result;
import edu.cmu.sphinx.decoder.search.AlternateHypothesisManager;
import edu.cmu.sphinx.decoder.search.Token;
import edu.cmu.sphinx.linguist.WordSearchState;
import edu.cmu.sphinx.linguist.SearchState;
import edu.cmu.sphinx.linguist.dictionary.Dictionary;
import edu.cmu.sphinx.linguist.dictionary.Pronunciation;
import edu.cmu.sphinx.linguist.dictionary.Word;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.linguist.language.ngram.LanguageModel;
import edu.cmu.sphinx.linguist.WordSequence;
/**
 * <p>
 * Provides recognition lattice results. Lattices are created from
 * {@link edu.cmu.sphinx.result.Result Results}
 * which can be partial or final.
 * </p>
 * <p>
 * Lattices describe all theories considered by the Recognizer that have not
 * been pruned out.  Lattices are a directed graph containing 
 * {@link edu.cmu.sphinx.result.Node Nodes} and
 * {@link edu.cmu.sphinx.result.Edge Edges}.
 * A Node that correponds to a theory that a word was spoken over a particular
 * period of time.  An Edge that corresponds to the score of one word following
 * another.  The usual result transcript is the sequence of Nodes though the
 * Lattice with the best scoring path. Lattices are a useful tool for 
 * analyzing "alternate results".
 * </p>
 * <p>
 * A Lattice can be created from a Result that has a full token tree
 * (with its corresponding AlternativeHypothesisManager).
 * Currently, only the
 * {@link edu.cmu.sphinx.decoder.search.WordPruningBreadthFirstSearchManager}
 * has an AlternativeHypothesisManager. Furthermore, the lattice
 * construction code currently only works for linguists where the
 * {@link edu.cmu.sphinx.linguist.WordSearchState} returns false on the
 * <code>isWordStart</code> method, i.e., where the word states appear
 * at the end of the word in the linguist. <i>Therefore, lattices should
 * only be created from Result from the
 * {@link edu.cmu.sphinx.linguist.lextree.LexTreeLinguist} and the
 * {@link edu.cmu.sphinx.decoder.search.WordPruningBreadthFirstSearchManager}.
 * </i>
 * </p>
 * <p>
 * Lattices can also be created from a collapsed
 * {@link edu.cmu.sphinx.decoder.search.Token} tree and its 
 * AlternativeHypothesisManager. This is what 'collapsed' means.
 * Normally, between two word tokens is a series of tokens for other types
 * of states, such as unit or HMM states. Using 'W' for word tokens,
 * 'U' for unit tokens, 'H' for HMM tokens, a token chain can look like:
 * </p>
 * <pre>
 * W - U - H - H - H - H - U - H - H - H - H - W
 * </pre>
 * <p>
 * Usually, HMM tokens contains acoustic scores, and word tokens contains
 * language scores. If we want to know the total acoustic and language 
 * scores between any two words, it is unnecessary to keep around the
 * unit and HMM tokens. Therefore, all their acoustic and language scores
 * are 'collapsed' into one token, so that it will look like:
 * </p>
 * <pre>
 * W - P - W
 * </pre>
 * <p>
 * where 'P' is a token that represents the path between the two words,
 * and P contains the acoustic and language scores between the two words.
 * It is this type of collapsed token tree that the Lattice class is
 * expecting. Normally, the task of collapsing the token tree is done
 * by the
 * {@link edu.cmu.sphinx.decoder.search.WordPruningBreadthFirstSearchManager}.
 * A collapsed token tree can look like:
 * </p>
 * <pre>
 *                             "cat" - P - &lt;/s&gt;
 *                            / 
 *                           P
 *                          /
 * &lt;s&gt; - P - "a" - P - "big"
 *                          \
 *                           P
 *                            \
 *                             "dog" - P - &lt;/s&gt;
 * </pre>
 * <p>
 * When a Lattice is constructed from a Result, the above collapsed token tree 
 * together with the alternate hypothesis of "all" instead of "a",
 * will be converted into a Lattice that looks like the following:
 * <pre>
 *       "a"           "cat"
 *     /     \        /     \
 * &lt;s&gt;          "big"         - &lt;/s&gt;
 *     \     /        \     /
 *      "all"          "dog"
 * </pre>
 * <p>
 * Initially, a lattice can have redundant nodes, i.e., nodes referring to
 * the same word and that originate from the same parent node. These
 * nodes can be collapsed using the {@link LatticeOptimizer}.
 * </p>
 *
 */
public class Lattice {

    protected Node initialNode;
    protected Node terminalNode;
    protected HashSet <Edge>  edges;
    protected Map<String,Node> nodes;
    protected double logBase;
    protected LogMath logMath;
    private float wipLocal;
    private Set visitedWordTokens;
    private AlternateHypothesisManager loserManager;
    private Map<Token,Integer> nodesId;
    private int countNode=0;
    /**
     * Create an empty Lattice.
     */
    protected Lattice() {
        edges = new HashSet();
        nodes = new HashMap<String,Node>();
    }

    /**
     * Create an empty Lattice.
     */
    protected Lattice(LogMath logMath) {
	this();
	this.logMath = logMath;
    }

    public Lattice (Result result) {
	this(result,false);
    }
    /**
     * Create a Lattice from a Result.
     *
     * The Lattice is created from the Token tree referenced by the Result.
     * The Lattice is then optimized to all collapse equivalent paths.
     *
     * @param result the result to convert into a lattice
     */
    public Lattice(Result result, boolean theBestOnly) {
	this(result.getLogMath());
	visitedWordTokens = new HashSet();
        nodesId= new HashMap<Token,Integer> ();
        countNode=0; 
        loserManager = result.getAlternateHypothesisManager();
        if (loserManager != null) {
            loserManager.purge();
        }
	Token theBest=null;
	if (theBestOnly) theBest= result.getBestFinalToken();
        for (Iterator i = result.getResultTokens().iterator(); i.hasNext();) {
            Token token = (Token) i.next();
	    if (theBestOnly && token != theBest) continue;
	    //            while (token != null && !token.isWord()) {
	    //token = token.getPredecessor();
	    //            }
	    //  System.err.println("in lattice "+ token + " w="+ token.getSearchState());
	    if (terminalNode == null) {
		terminalNode = new Node(getNodeID(result.getBestToken()),
					token.getSearchState(),
					(token.isWord() && ((WordSearchState) token.getSearchState()).isWordStart()) ?token.getFrameNumber():-1 ,
					result.getFrameNumber());
		addNode(terminalNode);
	    }
	    collapseWordToken(token);
	}
	System.err.println(" vu node:" + countNode + " token:" + visitedWordTokens.size()); //+" "+ countWord(result));
	
    }
    /**
     * Returns the node corresponding to the given word token.
     *
     * @param token the token which we want a node of
     *
     * @return the node of the given token
     */
    private Node getNode(Token token) {
        if (token.isWord() && token.getWord().isSentenceEndWord()) {
            return terminalNode;
        }
        Node node =  nodes.get(getNodeID(token));
        if (node == null) {
	    SearchState wordState = 
	        token.getSearchState();
            
	    int startFrame = -1;
            int endFrame = -1;

            if ((wordState instanceof WordSearchState) &&((WordSearchState) wordState).isWordStart()) {
                startFrame = token.getFrameNumber();
            } else {
                endFrame = token.getFrameNumber();
                {Token pred=token.getPredecessor();
		    // while (pred!=null && !pred.isWord())
		    //pred=pred.getPredecessor();
		    if (pred!=null)
			startFrame= pred.getFrameNumber()+1;
		}
            }

            node = new Node(getNodeID(token), token.getSearchState(),
			    startFrame, endFrame);
            addNode(node);
        }
        return node;
    }

    
    /**
     * Collapse the given word-ending token. This means collapsing all
     * the unit and HMM tokens that correspond to the word represented
     * by this token into an edge of the lattice.
     *
     * @param token the word-ending token to collapse
     */
    private void collapseWordToken(Token token) {
        if (visitedWordTokens.contains(token)) {
            return;
        }
        visitedWordTokens.add(token);
        collapseWordPath(getNode(token), token.getPredecessor(),
                         token.getScore()-token.getPredecessor().getScore(),
			 token.getLanguageScore()+token.getInsertionProbability() -token.getPredecessor().getLanguageScore()-
			 token.getPredecessor().getInsertionProbability());
        if (loserManager != null) {
            List list = loserManager.getAlternatePredecessors(token);
            if (list != null) {
                for (Iterator i = list.iterator(); i.hasNext();) {
                    Token loser = (Token) i.next();
		    if (false)
			// modif 21/09/2007 wordprunning change
			// pour recuperer score corect sur le loser.
			//le loser a le meme wordstate que le token.

                    collapseWordPath(getNode(token), loser, token.getScore()-loser.getScore(),
                                    
                                     token.getLanguageScore()+token.getInsertionProbability()
				     -loser.getLanguageScore()-
				      loser.getInsertionProbability());
		    else
			collapseWordPath(getNode(token), loser.getPredecessor(),
                         loser.getScore()-loser.getPredecessor().getScore(),
			 loser.getLanguageScore()+loser.getInsertionProbability()- 
					 loser.getPredecessor().getLanguageScore()-
					 loser.getPredecessor().getInsertionProbability());



                }
            }
        }
    }

    /**
     * @param parentWordNode the 'toNode' of the returned edge
     * @param token the predecessor token of the token represented by
     *              the parentWordNode
     * @param acousticScore the acoustic score until and including the
     *                      parent of token
     * @param languageScore the language score until and including the
     *                      parent of token
     */
    private void collapseWordPath(Node parentWordNode, Token token,
                                  float acousticScore, float languageScore) {
  
	//	System.err.println("wp: "+token);
        if (true || token.isWord()) {
            /*
             * If this is a word, create a Node for it, and then create an
             * edge from the Node to the parentWordNode
             */
            Node fromNode = getNode(token);
            addEdge(fromNode, parentWordNode,
                    (double)acousticScore, (double)languageScore);

            if (token.getPredecessor() != null) {
                /* Collapse the token sequence ending in this token. */
                collapseWordToken(token);
            } else {
                /* we've reached the sentence start token */
                //assert token.getWord().isSentenceStartWord();
                initialNode = fromNode;
            }
        }//pas de else 
	if (false){
            /*
             * If a non-word token, just add the acoustic and language
             * scores to the current totals, and then move on to the
             * predecessor token.
             */
            acousticScore += token.getAcousticScore();
            languageScore += token.getLanguageScore();
            collapseWordPath(parentWordNode, token.getPredecessor(),
                             acousticScore, languageScore);
            
            /* Traverse the path(s) for the loser token(s). */
            if (loserManager != null) {
                List list = loserManager.getAlternatePredecessors(token);
                if (list != null) {
                    for (Iterator i = list.iterator(); i.hasNext();) {
                        Token loser = (Token) i.next();
                        collapseWordPath(parentWordNode, loser,
                                         acousticScore, languageScore);
                    }
                }
            }
        }
    }

    /**
     * Returns an ID for the Node associated with the given token.
     *
     * @param token the token associated with the Node
     *
     * @return an ID for the Node
     *  il faut mieux recrire cela.
     */

    private String getNodeID(Token token) {
	Integer p = nodesId.get(token);
	if (p==null)
	    nodesId.put(token,p=countNode++);
	return p.toString();
       
	///    return Integer.toString(token.hashCode());
    }

    /**
     * Create a Lattice from a HTK  file.
     *      *
     * @param fileName
     */

   //  public Lattice(LogMath logMath,Dictionary dico,  java.io.File fileName) throws IOException {
// 	this();
// 	this.logMath = logMath;
//         Pattern egal,blanc,card,noeud,arc;
//         egal=Pattern.compile("=");
//         blanc=Pattern.compile("\\s+");
//         int nombreNoeud,nombreArc;
// 	//	try
//  {
//             System.err.println("Loading from " + fileName);
	    
//             // load the nodes
//             LineNumberReader in = new LineNumberReader(new InputStreamReader(new GZIPInputStream(new java.io.FileInputStream( fileName))));
//             String line;
// 	    line=in.readLine();
//             if (!line.equals("VERSION=1.0")) {
// 		throw new Error(line);}
//             line=in.readLine();
//             String [] dec=egal.split(line);
//             if (!dec[0].equals("lmscale")) throw new Error(line+"lmscale");
// 	    line=in.readLine();
//             dec=egal.split(line);
// 	    if (!dec[0].equals("wdpenalty")) throw new Error(line+"wip");
// 	    this. wipLocal= logMath.lnToLog(Float.parseFloat(dec[1]));
//             line=in.readLine();
// 	    card=Pattern.compile("N=(\\d+)\\s+L=(\\d+)");
//             Matcher m=card.matcher(line);
// 	    if(!m.find()) throw new Error(line+" cardinaux");;
//             nombreNoeud= Integer.parseInt(m.group(1));
//             nombreArc = Integer.parseInt(m.group(2));
// 	    lesNoeuds= new Node[nombreNoeud];
//             noeud=Pattern.compile("I=(\\d+)\\s+t=(\\S+)\\s+W=(\\S+)\\s+v=(\\d+)");
//             arc = Pattern.compile("J=(\\d+)\\s+S=(\\S+)\\s+E=(\\S+)\\s+a=(\\S+)\\s+l=(\\S+)");
// 	    while ((line = in.readLine()) != null) {
// 		Matcher ml=noeud.matcher(line);
// 		if (ml.find()) {
// 		    int i=Integer.parseInt(ml.group(1));
// 		    int t=Math.round( (Float.parseFloat(ml.group(2))*100));
// 		    String mot=ml.group(3);
// 		    int v=Integer.parseInt(ml.group(4));
//                     if (i<nombreNoeud-1) 
// 			lesNoeuds[i]=addNode(Integer.toString(i),dico.getWord(mot),t,-1);
// 		    else
// 			lesNoeuds[i]=addNode(Integer.toString(i),dico.getWord(mot),t,t);
// 		} else
// 		   {Matcher al=arc.matcher(line);
// 		       if (al.find()) {
// 			   Node debut=lesNoeuds[Integer.parseInt(al.group(2))];
//                            Node fin=lesNoeuds[Integer.parseInt(al.group(3))];
// 			   float a= logMath.lnToLog(Float.parseFloat(al.group(4)));
// 			   float l=logMath.lnToLog(Float.parseFloat(al.group(5)));
// 			   if (debut.getWord().isFiller())
// 			       fillerScore.put(debut.getWord(),l);
//                            addEdge(debut,fin,a,l);
// 		       }
// 		       else 
// 			   throw new Error("SYNTAX ERROR: " + fileName +
// 					   "[" + in.getLineNumber() + "] " + line);
// 		   }
// 	   }
	    
//             setInitialNode(lesNoeuds[0]);
//             setTerminalNode(lesNoeuds[lesNoeuds.length-1]);
//             if (nombreNoeud != nodes.size() ||
// 		nombreArc !=edges.size())
// 		throw new Error("nb noeud et nb arc");
// 	    //final et initial nodes
// 	}

// // 		catch (Exception e) {
// // 		    e.printStackTrace();
// // 		    throw new Error( e.toString());
// //         }
//     }

    
    
    
    
    
//     public Lattice(String fileName) {
//         try {
//             System.err.println("Loading from " + fileName);

//             // load the nodes
//             LineNumberReader in = new LineNumberReader(new FileReader(fileName));
//             String line;
//             while ((line = in.readLine()) != null) {
//                 StringTokenizer tokens = new StringTokenizer(line);
//                 if (tokens.hasMoreTokens()) {
//                     String type = tokens.nextToken();

//                     if (type.equals("edge:")) {
//                         Edge.load(this, tokens);
//                     } else if (type.equals("node:")) {
//                         Node.load(this, tokens);
//                     } else if (type.equals("initialNode:")) {
//                         setInitialNode(getNode(tokens.nextToken()));
//                     } else if (type.equals("terminalNode:")) {
//                         setTerminalNode(getNode(tokens.nextToken()));
//                     } else if (type.equals("logBase:")) {
//                         logBase = Double.parseDouble(tokens.nextToken());
//                     } else {
//                         throw new Error("SYNTAX ERROR: " + fileName +
//                                 "[" + in.getLineNumber() + "] " + line);
//                     }
//                 }
//             }
//             in.close();
//         } catch (Exception e) {
//             throw new Error(e.toString());
//         }
//     }

    /**
     * Add an edge from fromNode to toNode.  This method creates the Edge
     * object and does all the connecting
     *
     * @param fromNode
     * @param toNode
     * @param acousticScore
     * @param lmScore
     * @return the new Edge
     */
    public Edge addEdge(Node fromNode, Node toNode,
                        double acousticScore, double lmScore) {
        Edge e = new Edge(fromNode, toNode, acousticScore, lmScore);
        fromNode.addLeavingEdge(e);
        toNode.addEnteringEdge(e);
        edges.add(e);
       	//System.err.println(" b:" + fromNode + " " +toNode);
	return e;    
    }

    protected Node addNode(String id, SearchState word, int beginTime, int endTime) {
	Node n = new Node(id, word, beginTime, endTime);
	addNode(n);
	return n;
    }


    /**
     * Add a Node that represents the theory that a given word was spoken
     * over a given period of time.
     *
     * @param word
     * @param beginTime
     * @param endTime
     * @return the new Node
     */
    public Node addNode(SearchState word, int beginTime, int endTime) {
        Node n = new Node(word, beginTime, endTime);
        addNode(n);
        return n;
    }

     /**
     * Add a Node corresponding to a Token from the result Token tree.
     * Usually, the Token should reference a search state that is a
     * WordSearchState, although other Tokens may be used for debugging.
     * @param token
     * @return the new Node
     */
    protected Node addNode(Token token, int beginTime, int endTime) {
	
        return addNode(Integer.toString(token.hashCode()),
                       token.getSearchState(), beginTime, endTime);
    }

    /**
     * Test to see if the Lattice contains a Node
     *
     * @param node
     * @return true if yes
     */
    boolean hasNode(Node node) {
        return nodes.containsValue(node);
    }

    /**
     * Test to see if the Lattice contains an Edge
     *
     * @param edge
     * @return true if yes
     */
    boolean hasEdge(Edge edge) {
        return edges.contains(edge);
    }

    /**
     * Test to see if the Lattice already contains a Node corresponding
     * to a given Token.
     *
     * @param ID the ID of the Node to find
     * @return true if yes
     */
    protected boolean hasNode(String ID) {
        return nodes.containsKey(ID);
    }

    /**
     * Add a Node to the set of all Nodes
     *
     * @param n
     */
    protected void addNode(Node n) {
        assert !hasNode(n.getId());
        nodes.put(n.getId(), n);
    }

    /**
     * Remove a Node from the set of all Nodes
     *
     * @param n
     */
    protected void removeNode(Node n) {
        assert hasNode(n.getId());
        nodes.remove(n.getId());
    }

    /**
     * Get the Node associated with an ID
     *
     * @param id
     * @return the Node
     */
    protected Node getNode(String id) {
        return  (nodes.get(id));
    }

    /**
     * Get a copy of the Collection of all Nodes.
     * Used by LatticeOptimizer to avoid Concurrent modification of the
     * nodes list.
     *
     * @return a copy of the collection of Nodes
     */
    protected Collection <Node> getCopyOfNodes() {
        return new Vector(nodes.values());
    }

    /**
     * Get the Collection of all Nodes.
     *
     * @return the colllection of all Nodes
     */
    public Collection <Node> getNodes() {
        return nodes.values();
    }

    /**
     * Remove an Edge from the set of all Edges.
     * @param e
     */
    protected void removeEdge(Edge e) {
        edges.remove(e);
    }

    /**
     * Get the set of all Edges.
     *
     * @return the set of all edges
     */
    public Collection <Edge> getEdges() {
        return edges;
    }
    public Collection <Edge> getCopyEdges() {
	return (Collection <Edge>)edges.clone();
    }

    /**
     * Dump the Lattice in the form understood by AiSee
     * (a graph visualization tool).  See http://www.AbsInt.com
     *
     * @param fileName
     * @param title
     */
    public void dumpAISee(String fileName, String title) {
        try {
            System.err.println("Dumping " + title + " to " + fileName);
            FileWriter f = new FileWriter(fileName);
            f.write("graph: {\n");
            f.write("title: \"" + title + "\"\n");
            f.write("display_edge_labels: yes\n");
            /*
            f.write( "colorentry 32: 25 225 0\n");
            f.write( "colorentry 33: 50 200 0\n");
            f.write( "colorentry 34: 75 175 0\n");
            f.write( "colorentry 35: 100 150 0\n");
            f.write( "colorentry 36: 125 125 0\n");
            f.write( "colorentry 37: 150 100 0\n");
            f.write( "colorentry 38: 175 75 0\n");
            f.write( "colorentry 39: 200 50 0\n");
            f.write( "colorentry 40: 225 25 0\n");
            f.write( "colorentry 41: 250 0 0\n");
            f.write( "color: black\n");
            f.write( "orientation: left_to_right\n");
            f.write( "xspace: 10\n");
            f.write( "yspace: 10\n");
            */

            for (Iterator <Node> i = nodes.values().iterator(); i.hasNext();) {
                ((Node) (i.next())).dumpAISee(f);
            }
            for (Edge e : edges) {
		if (true || e.getIsNotPruned())
                e.dumpAISee(f);
            }
            f.write("}\n");
            f.close();
        } catch (IOException e) {
            throw new Error(e.toString());
        }
    }


    /**
     * Dump the Lattice as a .LAT file
     *
     * @param out
     * @throws IOException false  PrinterWriter do not throw IOexception
     */
    public  void dump(File file) { // throws IOException {
	try {
	    System.err.println("Dumping   to " + file);

            PrintWriter out =new PrintWriter( new OutputStreamWriter(
								     new GZIPOutputStream(new java.io.FileOutputStream(file))));


	    out.println("initialNode: " + initialNode.getId());
	    out.println("terminalNode: " + terminalNode.getId());
	    out.println("logBase: " + logMath.getLogBase());
	    //System.err.println( "Dumping to " + out );
	    for (Iterator <Node> i = nodes.values().iterator(); i.hasNext();) {
		((Node) (i.next())).dump(out);
	    }
	    for (Iterator i = edges.iterator(); i.hasNext();) {
		((Edge) (i.next())).dump(out);
	    }
	    out.close();
	}
	
	catch (IOException e) {
            throw new Error(e.toString());
        }
    }


    /**
     * Remove a Node and all Edges connected to it.  Also remove those
     * Edges from all connected Nodes.
     *
     * @param n
     */
    protected void removeNodeAndEdges(Node n) {

        //System.err.println("Removing node " + n + " and associated edges");
        for (Iterator i = n.getLeavingEdges().iterator(); i.hasNext();) {
            Edge e = (Edge) (i.next());
            e.getToNode().removeEnteringEdge(e);
            //System.err.println( "\tRemoving " + e );
            edges.remove(e);
        }
        for (Iterator i = n.getEnteringEdges().iterator(); i.hasNext();) {
            Edge e = (Edge) (i.next());
            e.getFromNode().removeLeavingEdge(e);
            //System.err.println( "\tRemoving " + e );
            edges.remove(e);
        }
        //System.err.println( "\tRemoving " + n );
        nodes.remove(n.getId());

        assert checkConsistency();
    }

    /**
     * Remove a Node and cross connect all Nodes with Edges to it.
     *
     * For example given
     *
     * Nodes A, B, X, M, N
     * Edges A-->X, B-->X, X-->M, X-->N
     *
     * Removing and cross connecting X would result in
     *
     * Nodes A, B, M, N
     * Edges A-->M, A-->N, B-->M, B-->N
     *
     * @param n
     */
    protected void removeNodeAndCrossConnectEdges(Node n) {
        System.err.println("Removing node " + n + " and cross connecting edges");
        for (Iterator i = n.getEnteringEdges().iterator(); i.hasNext();) {
            Edge ei = (Edge) (i.next());
            for (Iterator j = n.getLeavingEdges().iterator(); j.hasNext();) {
                Edge ej = (Edge) (j.next());
                addEdge(ei.getFromNode(), ej.getToNode(),
                        ei.getAcousticScore(), ei.getLMScore());
            }
        }
        removeNodeAndEdges(n);

        assert checkConsistency();
    }

    /**
     * Get the initialNode for this Lattice.  This corresponds usually to
     * the <s> symbol
     *
     * @return the initial Node
     */
    public Node getInitialNode() {
        return initialNode;
    }

    /**
     * Set the initialNode for this Lattice.  This corresponds usually to
     * the <s> symbol
     *
     * @param p_initialNode
     */
    public void setInitialNode(Node p_initialNode) {
        initialNode = p_initialNode;
    }

    /**
     * Get the terminalNode for this Lattice.  This corresponds usually to
     * the </s> symbol
     *
     * @return the initial Node
     */
    public Node getTerminalNode() {
        return terminalNode;
    }

    /**
     * Set the terminalNode for this Lattice.  This corresponds usually to
     * the </s> symbol
     *
     * @param p_terminalNode
     */
    public void setTerminalNode(Node p_terminalNode) {
        terminalNode = p_terminalNode;
    }

    /**
     * Edge scores are usually log-likelyhood.  Get the log base.
     *
     * @return the log base
     */
    public double getLogBase() {
        return logMath.getLogBase();
    }

    /**
     * @return Returns the logMath object used in this lattice.
     */
    public LogMath getLogMath() {
        return logMath;
    }

    /**
     * Sets the LogMath to use.
     *
     * @param logMath the LogMath to use
     */
    public void setLogMath(LogMath logMath) {
        this.logMath = logMath;
    }
    
    /**
     * Dump all paths through this Lattice.  Used for debugging.
     */
    public void dumpAllPaths() {
        for (Iterator i = allPaths().iterator(); i.hasNext();) {
            System.out.println(i.next());
        }
    }

    /**
     * Generate a List of all paths through this Lattice.
     *
     * @return a lists of lists of Nodes
     */
    public List allPaths() {
        return allPathsFrom("", initialNode);
    }

    /**
     * Internal routine used to generate all paths starting at a given node.
     *
     * @param path
     * @param n
     * @return a list of lists of Nodes
     */
    protected List allPathsFrom(String path, Node n) {
        String p = path + " " + n.getWord();
        List l = new LinkedList();
        if (n == terminalNode) {
            l.add(p);
        } else {
            for (Iterator i = n.getLeavingEdges().iterator(); i.hasNext();) {
                Edge e = (Edge) i.next();
                l.addAll(allPathsFrom(p, e.getToNode()));
            }
        }
        return l;
    }
    
    boolean checkConsistency() {
        for (Iterator <Node> i = nodes.values().iterator(); i.hasNext();) {
            Node n = (Node) i.next();
            for (Iterator j = n.getEnteringEdges().iterator(); j.hasNext();) {
                Edge e = (Edge) j.next();
                if (!hasEdge(e)) {
                    throw new Error("Lattice has NODE with missing FROM edge: "
                                    + n + "," + e);
                }
            }
            for (Iterator j = n.getLeavingEdges().iterator(); j.hasNext();) {
                Edge e = (Edge) j.next();
                if (!hasEdge(e)) {
                    throw new Error("Lattice has NODE with missing TO edge: " +
                                    n + "," + e);
                }
            }
        }
        for (Iterator i = edges.iterator(); i.hasNext();) {
            Edge e = (Edge) i.next();
            if (!hasNode(e.getFromNode())) {
                throw new Error("Lattice has EDGE with missing FROM node: " +
                                e);
            }
            if (!hasNode(e.getToNode())) {
                throw new Error("Lattice has EDGE with missing TO node: " + e);
            }
            if(!e.getToNode().hasEdgeFromNode(e.getFromNode())) {
                throw new Error("Lattice has EDGE with TO node with no corresponding FROM edge: " + e);
            }
            if(!e.getFromNode().hasEdgeToNode(e.getToNode())) {
                throw new Error("Lattice has EDGE with FROM node with no corresponding TO edge: " + e);
            }
        }
        return true;
    }

    protected void sortHelper(Node n, List sorted, Set visited) {
        if (visited.contains(n)) {
            return;
        }
        visited.add(n);
        if (n == null) {
            throw new Error("Node is null");
        }
        Iterator e = n.getLeavingEdges().iterator();
        while (e.hasNext()) {
            sortHelper(((Edge)e.next()).getToNode(),sorted,visited);
        }
        sorted.add(n);
    }
    
    /**
     * Topologically sort the nodes in this lattice.
     * 
     * @return Topologically sorted list of nodes in this lattice.
     */
    public List sortNodes() {
        Vector sorted = new Vector(nodes.size());
        sortHelper(initialNode,sorted,new HashSet());
        Collections.reverse(sorted);
        return sorted;
    }
    
    
    /**
     * Compute the utterance-level posterior for every node in the lattice, 
     * i.e. the probability that this node occurs on any path through the 
     * lattice. Uses a forward-backward algorithm specific to the nature of
     * non-looping left-to-right lattice structures.
     * 
     * Node posteriors can be retrieved by calling getPosterior() on Node 
     * objects.
     * 
     * @param languageModelWeight the language model weight that was used
     *        in generating the scores in the lattice
     */
    public void computeNodePosteriors(float languageModelWeight) {
        computeNodePosteriors(languageModelWeight, true );// a voir
    }


    /**
     * Compute the utterance-level posterior for every node in the lattice, 
     * i.e. the probability that this node occurs on any path through the 
     * lattice. Uses a forward-backward algorithm specific to the nature of
     * non-looping left-to-right lattice structures.
     * 
     * Node posteriors can be retrieved by calling getPosterior() on Node 
     * objects.
     * 
     * @param languageModelWeight the language model weight that was used
     *        in generating the scores in the lattice
     * @param useAcousticScoresOnly use only the acoustic scores to compute
     *               the posteriors, ignore the language weight and scores
     */
    public void computeNodePosteriors(float languageModelWeight,
                                      boolean useAcousticScoresOnly) {      
        //forward
        initialNode.setForwardScore(LogMath.getLogOne());
        List sortedNodes = sortNodes();
        assert sortedNodes.get(0) == initialNode;
        ListIterator n = sortedNodes.listIterator();
        while (n.hasNext()) {            
            Node currentNode = (Node)n.next();
            Collection currentEdges = currentNode.getLeavingEdges();
            for (Iterator i = currentEdges.iterator();i.hasNext();) {
                Edge edge = (Edge)i.next();
                double forwardProb = edge.getFromNode().getForwardScore();
                forwardProb += computeEdgeScore
                    (edge, languageModelWeight, useAcousticScoresOnly);
                edge.getToNode().setForwardScore
                    (logMath.addAsLinear
                     ((float)forwardProb,
                      (float)edge.getToNode().getForwardScore()));
            }
        }
        
        //backward
        terminalNode.setBackwardScore(LogMath.getLogOne());
        assert sortedNodes.get(sortedNodes.size()-1) == terminalNode;
        n = sortedNodes.listIterator(sortedNodes.size()-1);
        while (n.hasPrevious()) {            
            Node currentNode = (Node)n.previous();
            Collection currentEdges = currentNode.getLeavingEdges();
            for (Iterator i = currentEdges.iterator();i.hasNext();) {
                Edge edge = (Edge)i.next();
                double backwardProb = edge.getToNode().getBackwardScore();
                backwardProb += computeEdgeScore
                    (edge, languageModelWeight, useAcousticScoresOnly);
                edge.getFromNode().setBackwardScore
                    (logMath.addAsLinear((float)backwardProb,
                        (float)edge.getFromNode().getBackwardScore()));
            }
        }
        
        //inner
        double normalizationFactor = terminalNode.getForwardScore();
        for(Iterator i=nodes.values().iterator();i.hasNext();) {
            Node node = (Node)i.next();
            node.setPosterior((node.getForwardScore() + 
                               node.getBackwardScore()) - normalizationFactor);
        }
    }

   /**
     * Compute the utterance-level posterior for every node in the lattice, 
     * i.e. the probability that this node occurs on any path through the 
     * lattice. Uses a forward-backward algorithm specific to the nature of
     * non-looping left-to-right lattice structures.
     * 
     * Node posteriors can be retrieved by calling getPosterior() on Node 
     * objects.
     * 
     * @param languageModelWeight the language model weight that was used
     *        in generating the scores in the lattice
     * @param useAcousticScoresOnly use only the acoustic scores to compute
     *               the posteriors, ignore the language weight and scores
     */
    Node[] lesNoeuds;
    public void computeNodePosteriors(float languageModelWeight,
                                      boolean useAcousticScoresOnly, float pourcent,float seuilabs, float wip,LanguageModel lm,float scale) {      
	if (lm!= null) lm.start();
        //forward
        if (wip>0) wip =logMath.linearToLog(wip);
	else
	    wip = wipLocal;
	if (lesNoeuds == null) {
	    lesNoeuds= getNodes().toArray(new Node[0]);
	    Arrays.sort(lesNoeuds,new Comparator<Node>() {
				 public int compare(Node n1,Node n2) {
				     if (n1.getBeginTime()==n2.getBeginTime())
					 return  n1.getEndTime()- n2.getEndTime();
				     return n1.getBeginTime()- n2.getBeginTime();
				 }
			     });

	}
	Node []  sortedNodes = lesNoeuds; // c'est htk graphe
	for (Node currentNode : sortedNodes){ 
	    currentNode.setForwardScore(LogMath.getLogZero());
	    currentNode.setBackwardScore(LogMath.getLogZero());
	}
        initialNode.setForwardScore(LogMath.getLogOne());
        if ( sortedNodes[0] != initialNode) {
	    System.err.println ("proble "+ sortedNodes.length+ " "+initialNode +"  first:" + sortedNodes[0]);
	    return;}

        int mina =(sortedNodes[sortedNodes.length-1].getEndTime()- sortedNodes[0].getBeginTime());
	for (Node currentNode : sortedNodes) {
            if (currentNode.getForwardScore() == LogMath.getLogZero())
		continue;
	    if (lm!=null && currentNode.getWord().isFiller()) {
		double mini= LogMath.getLogZero();
		Word best=null;
		for (Edge e : currentNode.getEnteringEdges())
		    {if (e.getFromNode().getForwardScore()>mini){
			    mini=e.getFromNode().getForwardScore();
			    if (e.getFromNode().getBest()!=null)
				best=e.getFromNode().getBest();
			    else best=e.getFromNode().getWord();
			}
		    }
		currentNode.setBest(best);
	    }
            Collection currentEdges = currentNode.getLeavingEdges();
            for (Iterator i = currentEdges.iterator();i.hasNext();) {
                Edge edge = (Edge)i.next();
                if (lm!=null) {
		    if(edge.getToNode().getWord().isFiller()&& (!edge.getToNode().getWord().isSentenceEndWord())){
			Float l= null;//fillerScore.get(edge.getToNode().getWord());
			if (l!=null)
			edge.setLMScore(l);
		    }
		    else {
		       
			edge.setLMScore(lm.getProbability(WordSequence.getWordSequence(currentNode.getBest()==null? currentNode.getWord():currentNode.getBest(),edge.getToNode().getWord())));
		    }
		}
			

                double forwardProb = edge.getFromNode().getForwardScore();
                forwardProb += computeEdgeScore
                    (edge, languageModelWeight, useAcousticScoresOnly,wip,scale);
                edge.getToNode().setForwardScore
                    (logMath.addAsLinear
                     ((float)forwardProb,
                      (float)edge.getToNode().getForwardScore()));
            }
        }
        if (lm!=null) lm.stop();
        //backward
        
        terminalNode.setBackwardScore(LogMath.getLogOne());
        assert sortedNodes[sortedNodes.length-1] == terminalNode;
        double normalizationFactor = terminalNode.getForwardScore();
        for (int iNode = sortedNodes.length-1; iNode>=0 ;iNode--) {            
            Node currentNode =  sortedNodes[iNode];
            Collection currentEdges = currentNode.getLeavingEdges();
            for (Iterator i = currentEdges.iterator();i.hasNext();) {
                Edge edge = (Edge)i.next();
                double backwardProb = edge.getToNode().getBackwardScore();
		if (backwardProb == LogMath.getLogZero() )
		    continue;
                backwardProb += computeEdgeScore
                    (edge, languageModelWeight, useAcousticScoresOnly,wip,scale);
                edge.getFromNode().setBackwardScore
                    (logMath.addAsLinear((float)backwardProb,
					 (float)edge.getFromNode().getBackwardScore()));
            }
	    if ( currentNode.getForwardScore() != LogMath.getLogZero() && 
		 currentNode.getBackwardScore() != LogMath.getLogZero())
		currentNode.setPosterior((currentNode.getForwardScore() + 
					  currentNode.getBackwardScore()) - normalizationFactor);
	    else removeNodeAndEdges(currentNode);
        }
	System.err.println("alpha :" +sortedNodes[sortedNodes.length-1].getForwardScore() +
			   " beta:" +sortedNodes[0].getBackwardScore());
        lesNoeuds=null;// c'est plus l'etat 
        //inner
 //        double normalizationFactor = terminalNode.getForwardScore();
//         for(Iterator i=nodes.values().iterator();i.hasNext();) {
//             Node node = (Node)i.next();
//             node.setPosterior((node.getForwardScore() + 
//                                node.getBackwardScore()) - normalizationFactor);
//         } 
        Edge[] lesEdges = (Edge []) getEdges().toArray(new Edge[0]);
	for (Edge e: lesEdges) {
	    e.setPscore((e.getFromNode().getForwardScore()+
			 computeEdgeScore(e, languageModelWeight, useAcousticScoresOnly,wip,scale) +
			e.getToNode().getBackwardScore())- normalizationFactor);
        }
	System.err.println("nb Edge "+ lesEdges.length);
        Arrays.sort(lesEdges, new Comparator<Edge>() {
	    public int compare(Edge o1, Edge o2) {
		double l= -o1.getPscore() +o2.getPscore();
		if (l<0.0) return -1;
                if (l>0) return 1;
                return 0;
	    }
	});

	mina= Math.min(lesEdges.length-1,Math.max(mina,(int)Math.round(pourcent*(lesEdges.length-1)/100.0)));
	System.err.println (lesEdges[0]+ " " +lesEdges[lesEdges.length-1]+" seuil"
			    + lesEdges[mina]+ " post:"+ logMath.logToLinear((float)lesEdges[mina].getPosterior() )  + " nombre:" + mina);
	float seuilAbsolu=logMath.linearToLog(seuilabs);	
        for (int i=0 ; i<=mina;i++){Edge e2= lesEdges[i];
	if(e2.getPosterior()<seuilAbsolu) { System.err.println("post: nb" +i); break;}
	    e2.setNoPruned(true);
	}

    }


    /**
     * Computes the score of an edge.
     *
     * @param edge the edge which score we want to compute
     * @param languageModelWeight the language model weight to use
     *
     * @return the score of an edge/languageModelWeight
     */
    private double computeEdgeScore(Edge edge, float languageModelWeight,
                                    boolean useAcousticScoresOnly) {
        if (useAcousticScoresOnly) {
            return edge.getAcousticScore();
        } else {
            return (edge.getAcousticScore() + edge.getLMScore())/languageModelWeight;
        }
    }
    private double computeEdgeScore(Edge edge, float languageModelWeight,
                                    boolean useAcousticScoresOnly,float wip,float scale) {
        if (useAcousticScoresOnly) {
            return (edge.getAcousticScore()+wip)/scale;
        } else {
            return (edge.getAcousticScore()+wip +languageModelWeight*edge.getLMScore())/scale;
        }
    }


    /**
     * Returns true if the given Lattice is equivalent to this Lattice.
     * Two lattices are equivalent if all their nodes and edges are
     * equivalent.
     *
     * @param other the Lattice to compare this Lattice against
     *
     * @return true if the Lattices are equivalent; false otherwise
     */
    public boolean isEquivalent(Lattice other) {
        return checkNodesEquivalent(initialNode, other.getInitialNode());
    }

    
    /**
     * Returns true if the two lattices starting at the given two nodes
     * are equivalent. It recursively checks all the child nodes until
     * these two nodes until there are no more child nodes.
     *
     * @param n1 starting node of the first lattice
     * @param n2 starting node of the second lattice
     *
     * @return true if the two lattices are equivalent
     */
    private boolean checkNodesEquivalent(Node n1, Node n2) {
        assert n1 != null && n2 != null;

        boolean equivalent = n1.isEquivalent(n2);
        if (equivalent) {
            Collection leavingEdges = n1.getCopyOfLeavingEdges();
            Collection leavingEdges2 = n2.getCopyOfLeavingEdges();

            System.out.println("# edges: " + leavingEdges.size() + " " + 
                               leavingEdges2.size());

            for (Iterator i = leavingEdges.iterator(); i.hasNext(); ) {
                
                Edge edge = (Edge) i.next();
                
                /* find an equivalent edge from n2 for this edge */
                Edge e2 = n2.findEquivalentLeavingEdge(edge);
                
                if (e2 == null) {
                    System.out.println
                        ("Equivalent edge not found, lattices not equivalent.");
                    return false;
                } else {
                    if (!leavingEdges2.remove(e2)) {
                        /*
                         * if it cannot be removed, then the leaving edges
                         * are not the same
                         */
                        System.out.println
                            ("Equivalent edge already matched, lattices not equivalent.");
                        return false;
                    } else {
                        /* recursively check the two child nodes */
                        equivalent &= checkNodesEquivalent
                            (edge.getToNode(), e2.getToNode());
                        if (equivalent == false) {
                            return false;
                        }
                    }
                }
            }
            if (leavingEdges2.size() != 0) {
                System.out.println("One lattice has too many edges.");
                return false;
            }
        }
        return equivalent;
    }

     

}
