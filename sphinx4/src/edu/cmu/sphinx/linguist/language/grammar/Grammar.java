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

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Random;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;

import edu.cmu.sphinx.linguist.dictionary.Dictionary;
import edu.cmu.sphinx.linguist.dictionary.Word;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.util.Timer;

/**

 * Classes that implement this interface create grammars. A grammar
 * is represented internally as a graph of {@link GrammarNode GrammarNodes}
 * linked together by {@link GrammarArc GrammarArcs}. Calling
 * {@link #getInitialNode() getInitialNode} will return the first node
 * of the grammar graph. To traverse the grammar graph, one should
 * call GrammarNode.getSuccessors, which will return an array of GrammarArcs,
 * from which you can reach the neighboring GrammarNodes.
 * <p>
 * Note that all grammar probabilities are maintained in LogMath log
 * domain.
 */
public abstract class Grammar implements Configurable {
    /**
     * Property to control the the dumping of the grammar
     */
    public final static String PROP_SHOW_GRAMMAR = "showGrammar";
    /**
     * The default value for PROP_SHOW_GRAMMAR.
     */
    public final static boolean PROP_SHOW_GRAMMAR_DEFAULT = false;
    /**
     * Property to control whether grammars are optimized or not
     */
    public final static String PROP_OPTIMIZE_GRAMMAR = "optimizeGrammar";
    /**
     * The default value for PROP_OPTIMIZE_GRAMMAR
     */
    public final static boolean PROP_OPTIMIZE_GRAMMAR_DEFAULT = true;

    /**
     * Property to control whether silence words are inserted into the graph
     */
    public final static String PROP_ADD_SIL_WORDS = "addSilenceWords";
    /**
     * The default value for PROP_ADD_SIL_WORDS
     */
    public final static boolean PROP_ADD_SIL_WORDS_DEFAULT  = false;

    /**
     * Property to control whether filler words are inserted into the graph
     */
    public final static String PROP_ADD_FILLER_WORDS = "addFillerWords";
    /**
     * The default value for PROP_ADD_FILLER_WORDS
     */
    public final static boolean PROP_ADD_FILLER_WORDS_DEFAULT  = false;

    /**
     * Property that defines the dictionary to use for this grammar
     */
    public final static String PROP_DICTIONARY = "dictionary";
    // ----------------------------
    // Configuration data
    // -----------------------------
    private String name;
    private Logger logger;
    private boolean showGrammar;
    private boolean optimizeGrammar = true;
    private boolean addSilenceWords = false;
    private boolean addFillerWords = false;
    private Dictionary dictionary;
    private GrammarNode initialNode;
    private Set grammarNodes;
    private final static Word[][] EMPTY_ALTERNATIVE = new Word[0][0];
    private Random randomizer = new Random();
    private int maxIdentity = 0;
    private boolean postProcessed = false;
    private boolean idCheck = false;



    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
            throws PropertyException {
        this.name = name;
        registry.register(PROP_DICTIONARY, PropertyType.COMPONENT);
        registry.register(PROP_SHOW_GRAMMAR, PropertyType.BOOLEAN);
        registry.register(PROP_OPTIMIZE_GRAMMAR, PropertyType.BOOLEAN);
        registry.register(PROP_ADD_SIL_WORDS, PropertyType.BOOLEAN);
        registry.register(PROP_ADD_FILLER_WORDS, PropertyType.BOOLEAN);
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
        showGrammar = ps.getBoolean(PROP_SHOW_GRAMMAR,
                PROP_SHOW_GRAMMAR_DEFAULT);
        optimizeGrammar = ps.getBoolean(PROP_OPTIMIZE_GRAMMAR,
                PROP_OPTIMIZE_GRAMMAR_DEFAULT);

        addSilenceWords = ps.getBoolean(PROP_ADD_SIL_WORDS,
                PROP_ADD_SIL_WORDS_DEFAULT);
        addFillerWords = ps.getBoolean(PROP_ADD_FILLER_WORDS,
                PROP_ADD_FILLER_WORDS_DEFAULT);

        dictionary = (Dictionary) ps.getComponent(PROP_DICTIONARY,
                Dictionary.class);
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#getName()
     */
    public String getName() {
        return name;
    }

    /**
     * Create the grammar
     */
    public void allocate() throws IOException {
        dictionary.allocate();
        newGrammar();
        Timer timer = Timer.getTimer("grammarLoad");
        timer.start();
        initialNode = createGrammar();
        postProcessGrammar();
        timer.stop();
    }

    /**
     * Deallocate resources allocated to this grammar
     */
    public void deallocate() {
        initialNode = null;
        grammarNodes = null;
        dictionary.deallocate();
    }

    /**
     * Returns the initial node for the grammar
     * 
     * @return the initial grammar node
     */
    public GrammarNode getInitialNode() {
        return initialNode;
    }

    /**
     * Perform the standard set of grammar post processing. This can
     * include inserting silence nodes and optimizing out empty nodes
     */
    protected void postProcessGrammar() {
        if (!postProcessed) {
            if (addFillerWords) {
                addFillerWords();
            } else if (addSilenceWords) {
                addSilenceWords();
            }

            if (optimizeGrammar) {
                optimizeGrammar();
            }
            dumpStatistics();
            if (showGrammar) {
                dumpGrammar("grammar.gdl");
                dumpRandomSentences("sentences.txt", 100);
                logger.info("Total number of nodes " + grammarNodes.size());
            }
            postProcessed = true;
        }
    }

    /**
     * Dumps statistics for this grammar
     *  
     */
    public void dumpStatistics() {
        if (logger.isLoggable(Level.INFO)) {
            int successorCount = 0;
            logger.info("Num nodes : " + getNumNodes());
            for (Iterator i = grammarNodes.iterator(); i.hasNext();) {
                GrammarNode node = (GrammarNode) i.next();
                successorCount += node.getSuccessors().length;
            }
            logger.info("Num arcs  : " + successorCount);
            logger.info("Avg arcs  : "
                    + ((float) successorCount / getNumNodes()));
        }
    }

    /**
     * Dump a set of random sentences that fit this grammar
     * 
     * @param path
     *                the name of the file to dump the sentences to
     * @param count
     *                dumps no more than this. May dump less than this
     *                depending upon the number of uniqe sentences in the
     *                grammar.
     */
    public void dumpRandomSentences(String path, int count) {
        try {
            Set set = new HashSet();
            PrintWriter out = new PrintWriter(new FileOutputStream(path));
            for (int i = 0; i < count; i++) {
                String s = getRandomSentence();
                if (!set.contains(s)) {
                    set.add(s);
                    out.println(s);
                }
            }
            out.close();
        } catch (IOException ioe) {
            logger.severe("Can't write random sentences to " + path + " "
                    + ioe);
        }
    }


    /**
     * Dump a set of random sentences that fit this grammar
     * 
     * @param count
     *                dumps no more than this. May dump less than this
     *                depending upon the number of uniqe sentences in the
     *                grammar.
     */
    public void dumpRandomSentences(int count) {
        Set set = new HashSet();
        for (int i = 0; i < count; i++) {
            String s = getRandomSentence();
            if (!set.contains(s)) {
                set.add(s);
            }
        }
        List sampleList = new ArrayList(set);
        Collections.sort(sampleList);

        for (Iterator i = sampleList.iterator(); i.hasNext(); ) {
            System.out.println(i.next());
        }
    }

    /**
     * Returns a random sentence that fits this grammar
     * 
     * @return a random sentence that fits this grammar
     */
    public String getRandomSentence() {
        StringBuffer sb = new StringBuffer();
        GrammarNode node = getInitialNode();
        while (!node.isFinalNode()) {
            if (!node.isEmpty()) {
                Word word = node.getWord();
                if (!word.isFiller()) {
                    if (sb.length() > 0) {
                        sb.append(" ");
                    }
                    sb.append(word.getSpelling());
                }
            }
            node = selectRandomSuccessor(node);
        }
        return sb.toString();
    }

    /**
     * Given a node, select a random successor from the set of possible
     * successor nodes
     * 
     * @param node
     *                the node
     * @return a random successor node.
     */
    private GrammarNode selectRandomSuccessor(GrammarNode node) {
        GrammarArc[] arcs = node.getSuccessors();
        int index = randomizer.nextInt(arcs.length);
        return arcs[index].getGrammarNode();
    }

    /**
     * Dumps the grammar
     */
    public void dumpGrammar(String name) {
        getInitialNode().dumpGDL(name);
    }

    /**
     * returns the number of nodes in this grammar
     * 
     * @return the number of nodes
     */
    public int getNumNodes() {
        return grammarNodes.size();
    }

    /**
     * returns the set of of nodes in this grammar
     * 
     * @return the set of nodes
     */
    public Set getGrammarNodes() {
        return grammarNodes;
    }

    /**
     * Prepare to create a new grammar
     */
    protected void newGrammar() {
        maxIdentity = 0;
        grammarNodes = new HashSet();
        initialNode = null;
        postProcessed = false;
    }

    /**
     * Creates a grammar. Subclasses of grammar should implement this method.
     * 
     * @return the initial node for the grammar
     * 
     * @throws java.io.IOException
     *                 if the grammar could not be loaded
     * @throws java.lang.NoSuchMethodException
     *                 if called with inappropriate subclass.
     */
    protected abstract GrammarNode createGrammar() throws IOException;

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

    /**
     * Gets the dictionary for this grammar
     * 
     * @return the dictionary
     */
    protected Dictionary getDictionary() {
        return dictionary;
    }

    /**
     * Returns a new GrammarNode with the given set of alternatives.
     * 
     * @param identity
     *                the id for this node
     * @param alts
     *                the set of alternative word lists for this GrammarNode
     */
    protected GrammarNode createGrammarNode(int identity, String[][] alts) {
        GrammarNode node;
        Word[][] alternatives = new Word[alts.length][];
        for (int i = 0; i < alternatives.length; i++) {
            alternatives[i] = new Word[alts[i].length];
            for (int j = 0; j < alts[i].length; j++) {
                Word word = getDictionary().getWord(alts[i][j]);
                // Pronunciation[] pronunciation =
                // word.getPronunciations(null);
                if (word == null) {
                    alternatives = EMPTY_ALTERNATIVE;
                    break;
                } else {
                    alternatives[i][j] = word;
                }
            }
        }
        node = new GrammarNode(identity, alternatives);
        add(node);

        return node;
    }

    /**
     * Returns a new GrammarNode with the given single word. If the word is not
     * in the dictionary, an empty node is created. The grammar id is automatically
     * assigned
     * 
     * @param word
     *                the word for this grammar node
     */

    protected GrammarNode createGrammarNode(String word) {
        GrammarNode node =  createGrammarNode(maxIdentity + 1, word);
        return node;
    }

    /**
     * Creates an empty  grammar node in this grammar. The gramar ID is 
     * automatically assigned.
     * 
     * @param isFinal
     *                if true, this is a final node
     * 
     * @return the grammar node
     */
    protected GrammarNode createGrammarNode(boolean isFinal) {
        return createGrammarNode(maxIdentity + 1, isFinal);
    }

    /**
     * Returns a new GrammarNode with the given single word. If the word is not
     * in the dictionary, an empty node is created
     * 
     * @param identity
     *                the id for this node
     * @param word
     *                the word for this grammar node
     */
    protected GrammarNode createGrammarNode(int identity, String word) {
        GrammarNode node = null;
        Word[][] alternatives = EMPTY_ALTERNATIVE;
        Word wordObject = getDictionary().getWord(word);
        // Pronunciation[] pronunciation = wordObject.getPronunciations(null);
        if (wordObject != null) {
            alternatives = new Word[1][];
            alternatives[0] = new Word[1];
            alternatives[0][0] = wordObject;
            node = new GrammarNode(identity, alternatives);
            add(node);
        } else {
            node = createGrammarNode(identity, false);
            logger.warning("Can't find pronunciation for " + word);
        }
        return node;
    }

    /**
     * Creates a grammar node in this grammar with the given identity
     * 
     * @param identity
     *                the identity of the node
     * @param isFinal
     *                if true, this is a final node
     * 
     * @return the grammar node
     */
    protected GrammarNode createGrammarNode(int identity, boolean isFinal) {
        GrammarNode node;
        node = new GrammarNode(identity, isFinal);
        add(node);
        return node;
    }

    /**
     * Adds the given grammar node to the set of nodes for this grammar
     * 
     * @param node
     *                the grammar node
     */
    private void add(GrammarNode node) {
        if (node.getID() > maxIdentity) {
            maxIdentity = node.getID();
        }

        // check to see if there is already a node with the given ID.
        if (idCheck) {
            for (Iterator i = grammarNodes.iterator(); i.hasNext();) {
                GrammarNode gn = (GrammarNode) i.next();
                if (gn.getID() == node.getID()) {

                    throw new Error("DUP ID " + gn + " and " + node);
                }
            }
        }

        grammarNodes.add(node);

    }

    /**
     * Eliminate unnecessary nodes from the grammar. This method goes through
     * the grammar and looks for branches to nodes that have no words and have
     * only a single exit and bypasses these nodes.
     *  
     */
    private void optimizeGrammar() {
        Set nodes = getGrammarNodes();
        for (Iterator i = nodes.iterator(); i.hasNext();) {
            GrammarNode g = (GrammarNode) i.next();
            g.optimize();
        }
    }


    /**
     * Adds an optional silence word after every non-filler word in the
     * grammar
     */
    private void addSilenceWords() {
        Set nodes =  new HashSet(getGrammarNodes());
        for (Iterator i = nodes.iterator(); i.hasNext();) {
            GrammarNode g = (GrammarNode) i.next();
            if (!g.isEmpty() && !g.getWord().isFiller()) {
                GrammarNode silNode = createGrammarNode(maxIdentity + 1,
                        dictionary.getSilenceWord().getSpelling());

                GrammarNode branchNode = g.splitNode(maxIdentity + 1);
                add(branchNode);

                g.add(silNode, 0.00f);
                silNode.add(branchNode, 0.0f);
                silNode.add(silNode, 0.0f);
            }
        }
    }

    /**
     * Adds an optional filler word loop after every 
     *  non-filler word in the
     *  grammar
     */
    private void addFillerWords() {
        Set nodes =  new HashSet(getGrammarNodes());

        Word[] fillers = getInterWordFillers();

        if (fillers.length == 0) {
            return;     
        }

        for (Iterator i = nodes.iterator(); i.hasNext();) {
            GrammarNode wordNode = (GrammarNode) i.next();
            if (!wordNode.isEmpty() && !wordNode.getWord().isFiller()) {
                GrammarNode wordExitNode = wordNode.splitNode(maxIdentity + 1);
                add(wordExitNode);
                GrammarNode fillerStart = createGrammarNode(false);
                GrammarNode fillerEnd = createGrammarNode(false);
                fillerEnd.add(fillerStart, 0.0f);
                fillerEnd.add(wordExitNode, 0.0f);
                wordNode.add(fillerStart, 0.0f); 

                for (int j = 0; j < fillers.length; j++) {
                    GrammarNode fnode = createGrammarNode(maxIdentity + 1,
                            fillers[j].getSpelling());
                    fillerStart.add(fnode, 0.0f);
                    fnode.add(fillerEnd, 0.0f);
                }
            }
        }
    }

    /**
     * Gets the set of fillers after filtering out fillers that don't go
     * between words.
     *
     * @return the set of inter-word fillers
     */
    private Word[] getInterWordFillers() {
        List fillerList = new ArrayList();
        Word[] fillers = dictionary.getFillerWords();

        for (int i = 0; i < fillers.length; i++) {
            if (fillers[i] == dictionary.getSentenceStartWord()) {
                continue;
            } else if (fillers[i] == dictionary.getSentenceEndWord()) {
                continue;
            } else {
                fillerList.add(fillers[i]);
            }
        }
        return (Word[]) fillerList.toArray(new Word[fillerList.size()]);
    }
}
