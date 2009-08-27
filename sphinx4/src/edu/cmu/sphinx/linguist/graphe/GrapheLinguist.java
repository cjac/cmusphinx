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

package edu.cmu.sphinx.linguist.graphe;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.logging.Logger;


// import edu.cmu.sphinx.linguist.flat.BranchState
// import edu.cmu.sphinx.linguist.flat.GrammarState
// import edu.cmu.sphinx.linguist.flat.HMMStateState
// import edu.cmu.sphinx.linguist.flat.PronunciationState
// import edu.cmu.sphinx.linguist.flat.UnitState
// import edu.cmu.sphinx.linguist.flat.SentenceHMMState
import edu.cmu.sphinx.linguist.HMMSearchState;
import edu.cmu.sphinx.linguist.Linguist;
import edu.cmu.sphinx.linguist.SearchGraph;
import edu.cmu.sphinx.linguist.SearchState;
import edu.cmu.sphinx.linguist.SearchStateArc;
import edu.cmu.sphinx.linguist.UnitSearchState;
import edu.cmu.sphinx.linguist.WordSearchState;
import edu.cmu.sphinx.linguist.WordSequence;
import edu.cmu.sphinx.linguist.acoustic.AcousticModel;
import edu.cmu.sphinx.linguist.acoustic.HMM;
import edu.cmu.sphinx.linguist.acoustic.HMMState;
import edu.cmu.sphinx.linguist.acoustic.HMMStateArc;
import edu.cmu.sphinx.linguist.acoustic.Unit;
import edu.cmu.sphinx.linguist.acoustic.UnitManager;
import edu.cmu.sphinx.linguist.dictionary.Dictionary;
import edu.cmu.sphinx.linguist.dictionary.Pronunciation;
import edu.cmu.sphinx.linguist.dictionary.Word;
import edu.cmu.sphinx.linguist.language.ngram.LanguageModel;
import edu.cmu.sphinx.linguist.util.HMMPool;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.util.Timer;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;

/**
 *
 *  
 */
public class GrapheLinguist implements Linguist {
    /**
     * A sphinx property used to define the grammar to use when building the
     * search graph
     */
    public final static String PROP_GRAMMAR = "grammar";
    /**
     * A sphinx property used to define the acoustic model to use when building
     * the search graph
     */
    public final static String PROP_ACOUSTIC_MODEL = "acousticModel";

    /**
     * A sphinx property used to define the unit manager to use 
     * when building the search graph
     */
    public final static String PROP_UNIT_MANAGER = "unitManager";

    /**
     * Sphinx property that defines the name of the logmath to be used by this
     * search manager.
     */
    public final static String PROP_LOG_MATH = "logMath";
    /**
     * Sphinx property used to determine whether or not the gstates are dumped. *
     * A sphinx property that determines whether or not full word histories are
     * used to determine when two states are equal.
     */
    public final static String PROP_FULL_WORD_HISTORIES = "fullWordHistories";

    /**
     * The default value for PROP_FULL_WORD_HISTORIES
     */
    public final static boolean PROP_FULL_WORD_HISTORIES_DEFAULT = true;

    /**
     * A sphinx property for the language model to be used by this grammar
     */
    public final static String PROP_LANGUAGE_MODEL = "languageModel";

    /**
     * Property that defines the dictionary to use for this grammar
     */
    public final static String PROP_DICTIONARY = "dictionary";

    /**
     * A sphinx property that defines the size of the arc cache (zero
     * to disable the cache).
     */
    public final static String PROP_CACHE_SIZE = "cacheSize";

    /**
     * Property that defines the dictionary to use for this grammar
     */
    public final static int PROP_CACHE_SIZE_DEFAULT = 0;
    /**
     * le flat linguist qui porte la grammaire qui  porte le graphe
     * qui est sans lm et sans word history
     *
     */
    public final static String PROP_FLAT_LINGUIST="flatLinguist";

    // just for detailed debugging
    private final static boolean tracing = false;
    private final static SearchStateArc[] EMPTY_ARC = new SearchStateArc[0];

    // ----------------------------------
    // Subcomponents that are configured
    // by the property sheet
    // -----------------------------------
    private LanguageModel languageModel;
    private AcousticModel acousticModel;
    private LogMath logMath;
    private Dictionary dictionary;
    private UnitManager unitManager;
    private Linguist flatLinguist;
    // ------------------------------------
    // Data that is configured by the
    // property sheet
    // ------------------------------------
    private String name;
    private Logger logger;
    private boolean fullWordHistories = true;
    private boolean cacheEnabled = false;
    private int maxArcCacheSize = 0;

    private float languageWeight;
    private float logOne=LogMath.getLogOne();

    // ------------------------------------
    // Data used for building and maintaining
    // the search graph
    // -------------------------------------
    private Word sentenceEndWord;
    private Word[] sentenceStartWordArray;
    private SearchGraph searchGraph;
    private ArcCache arcCache = new ArcCache();

    private int cacheTrys;
    private int cacheHits;

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
	throws PropertyException {
        this.name = name;
        
        registry.register(PROP_ACOUSTIC_MODEL, PropertyType.COMPONENT);
        registry.register(PROP_LOG_MATH, PropertyType.COMPONENT);
        registry.register(PROP_LANGUAGE_MODEL, PropertyType.COMPONENT);
        registry.register(PROP_DICTIONARY, PropertyType.COMPONENT);
        registry.register(PROP_FLAT_LINGUIST, PropertyType.COMPONENT);
        
        registry.register(PROP_FULL_WORD_HISTORIES, PropertyType.BOOLEAN);
        registry.register(PROP_LANGUAGE_WEIGHT, PropertyType.FLOAT);
        registry.register(PROP_CACHE_SIZE, PropertyType.INT);
        registry.register(PROP_UNIT_MANAGER, PropertyType.COMPONENT);
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
        acousticModel = (AcousticModel) ps.getComponent(PROP_ACOUSTIC_MODEL,
							AcousticModel.class);
        logMath = (LogMath) ps.getComponent(PROP_LOG_MATH, LogMath.class);
        unitManager = (UnitManager) ps.getComponent(PROP_UNIT_MANAGER,
						    UnitManager.class);
        languageModel = (LanguageModel) ps.getComponent(PROP_LANGUAGE_MODEL,
							LanguageModel.class);
        dictionary = (Dictionary) ps.getComponent(PROP_DICTIONARY,
						  Dictionary.class);
        flatLinguist = (Linguist) ps.getComponent(PROP_FLAT_LINGUIST,
						    Linguist.class);
        
        fullWordHistories = ps.getBoolean(PROP_FULL_WORD_HISTORIES,
					  PROP_FULL_WORD_HISTORIES_DEFAULT);
        languageWeight = ps.getFloat(PROP_LANGUAGE_WEIGHT,
				     PROP_LANGUAGE_WEIGHT_DEFAULT);
        int newMaxArcCacheSize = ps.getInt(PROP_CACHE_SIZE,
					   PROP_CACHE_SIZE_DEFAULT);
        
        // if the new size of the arc cache is less than before
        // just clear out the cache, since we can easily grow it
        // but not easily shrink it.
        if (newMaxArcCacheSize < maxArcCacheSize) {
            arcCache = new ArcCache();
        }
        maxArcCacheSize = newMaxArcCacheSize;
        cacheEnabled = maxArcCacheSize > 0;
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#getName()
     */
    public String getName() {
        return name;
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.linguist.Linguist#allocate()
     */
    public void allocate() throws IOException {
        dictionary.allocate();
        acousticModel.allocate();//cela sera fait avant la lecture du first fsm 
				      
	//  flatLinguist.allocate();
        languageModel.allocate();
        compileGrammar();

    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.linguist.Linguist#deallocate()
     */
    public void deallocate() {
        flatLinguist.deallocate();
        flatLinguist=null;
        logMath = null;
        dictionary = null;
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.linguist.Linguist#getSearchGraph()
     */
    private SearchState flatBase=null;
    public SearchGraph getSearchGraph() {
        if (flatBase == flatLinguist.getSearchGraph().getInitialState())
	    return searchGraph;
	else
	    return  searchGraph = new GrapheSearchGraph(getInitialSearchState());
    }

    /**
     * 
     * Called before a recognition
     */
    public void startRecognition() {
      try {   flatLinguist.allocate();// je le mets la pour etre que le fsm soit positionne
      }
      catch (IOException io) {
            logger.severe("I/O error during decoding: " + io.getMessage());
        };
        languageModel.start();

        flatLinguist.startRecognition();
    }

    /**
     * Called after a recognition
     */
    public void stopRecognition() {
        languageModel.stop();
        flatLinguist.stopRecognition();
    }

    /**
     * Retrieves the language model for this linguist
     * 
     * @return the language model (or null if there is none)
     */
    public LanguageModel getLanguageModel() {
        return languageModel;
    }

    /**
     * retrieves the initial language state
     * 
     * @return the initial language state
     */
    private SearchState getInitialSearchState() {
	flatBase= flatLinguist.getSearchGraph().getInitialState();
        SearchState resu=flatBase;
        Set<SearchState> set= new HashSet<SearchState>();
	List<SearchState> queue = new LinkedList<SearchState>();
	queue.add(flatBase);//on fait sauter le <s> acoustique
	while (queue.size() > 0) {
	     resu =  queue.remove(0);
	     if (resu.getOrder()==0) break;
	    SearchStateArc[] successors = resu.getSuccessors();
	    for (SearchStateArc arc: successors){
		SearchState  nextState=  arc.getState();
		if (!set.contains(nextState)) {
		    queue.add(nextState);set.add(nextState);//paul
		}
	    }
	}
	

	if (resu instanceof WordSearchState) 
	    return new GrapheLinguistWordState(resu, 
	       WordSequence.getWordSequence(sentenceStartWordArray).trim(						     languageModel.getMaxDepth() - 1), logOne, logOne);
	else 

	    return new GrapheLinguistInitState(resu,WordSequence
						   .getWordSequence(sentenceStartWordArray).trim(
												 languageModel.getMaxDepth() - 1));


// 	    if (resu instanceof UnitSearchState)
//                 return new GrapheLinguistUnitState(resu, WordSequence
// 						   .getWordSequence(sentenceStartWordArray).trim(
// 												 languageModel.getMaxDepth() - 1), logOne, logOne);
// 	    else
// 	    if (resu instanceof HMMSearchState)
//                 return new GrapheLinguistHMMState(resu, WordSequence
// 			.getWordSequence(sentenceStartWordArray).trim(
// 								      languageModel.getMaxDepth() - 1),
// 						  logOne, logOne, logOne);
// 	    else
// 		return new GrapheLinguistState(resu, WordSequence
// 					       .getWordSequence(sentenceStartWordArray).trim(
// 						    languageModel.getMaxDepth() - 1),
// 					       logOne, logOne, logOne);
    }
          
    

    /**
     * Compiles the n-gram into a lex tree that is used during the search
     */
    protected void compileGrammar() {
        
        sentenceEndWord = dictionary.getSentenceEndWord();
        sentenceStartWordArray = new Word[1];
        sentenceStartWordArray[0] = dictionary.getSentenceStartWord();
	//	searchGraph = new GrapheSearchGraph(getInitialSearchState()); attente du fsm positionne
    }


    class GrapheSearchGraph implements SearchGraph {
        /**
         * An array of classes that represents the order in which the states
         * will be returned.
         */

        private SearchState initialState;

        /**
         * Constructs a search graph with the given initial state
         * 
         * @param initialState
         *                the initial state
         */
        GrapheSearchGraph(SearchState initialState) {
            this.initialState = initialState;
        }

        /*
         * (non-Javadoc)
         * 
         * @see edu.cmu.sphinx.linguist.SearchGraph#getInitialState()
         */
        public SearchState getInitialState() {
            return initialState;
        }

        /*
         * (non-Javadoc)
         * 
         * @see edu.cmu.sphinx.linguist.SearchGraph#getSearchStateOrder()
         */
        public int getNumStateOrder() {
            return 7;
        }
    }

    /**
     * The GraphLinguist returns lanague states to the search manager. This
     * class forms the base implementation for all language states returned.
     * This GraphState keeps track of the probability of entering this state (a
     * language+insertion probability) as well as the unit history. The unit
     * history consists of the LexTree nodes that correspond to the left,
     * center and right contexts.
     * 
     * This is an abstract class, subclasses must implement the getSuccessorss
     * method.
     */
    abstract class GrapheState implements SearchState, SearchStateArc {
        private SearchState  node;
        private WordSequence wordSequence;
        /**
         * Creates a LexTreeState.
         * 
         * @param node
         *                the node associated with this state
         * 
         * @param wordSequence
         *                the history of words up until this point
         *  
         */
        GrapheState( SearchState node, WordSequence wordSequence) {
            this.node = node;
            this.wordSequence = wordSequence;
        }

        /**
         * Gets the unique signature for this state. The signature building
         * code is slow and should only be used for non-time-critical tasks
         * such as plotting states.
         * 
         * @return the signature
         */
        public String getSignature() {
            return "lts-" + node.hashCode() + "-ws-" + wordSequence;
        }

        /**
         * Generate a hashcode for an object
         * 
         * @return the hashcode
         */
        private int hashCode=-1;
        public int hashCode() {
            if (hashCode==-1) {
		hashCode = fullWordHistories
                    ? wordSequence.hashCode() * 37
		    : 37*wordSequence.getNewWord().hashCode();
		hashCode += node.hashCode();
	    }
            return hashCode;
        }

        /**
         * Determines if the given object is equal to this object
         * 
         * @param o
         *                the object to test
         * @return <code>true</code> if the object is equal to this
         */
        public boolean equals(Object o) {
            if (o == this) {
                return true;
            } else if (o instanceof GrapheState) {
                GrapheState other = (GrapheState) o;
		//paul modif 
		// au mini one word dans history 
		// == is fast 
                return node == other.node && 
		    (fullWordHistories ? wordSequence
		     .equals(other.wordSequence) : 
		     wordSequence.getNewWord().equals(other.wordSequence.getNewWord()));


            } else {
                return false;
            }
        }

        /**
         * Gets a successor to this search state
         * 
         * @return the sucessor state
         */
        public SearchState getState() {
            return this;
        }

        /**
         * Gets the composite probability of entering this state
         * 
         * @return the log probability
         */
        public float getProbability() {
            return getLanguageProbability() + getAcousticProbability()
		+ getInsertionProbability();
        }

        /**
         * Gets the language probability of entering this state
         * 
         * @return the log probability
         */
        public float getLanguageProbability() {
            return logOne;
        }

        /**
         * Gets the language probability of entering this state
         * 
         * @return the log probability
         */
        public float getAcousticProbability() {
            return logOne;
        }

        /**
         * Gets the insertion probability of entering this state
         * 
         * @return the log probability
         */
        public float getInsertionProbability() {
            return logOne;
        }

        /**
         * Determines if this is an emitting state
         * 
         * @return <code>true</code> if this is an emitting state.
         */
        public boolean isEmitting() {
            return node.isEmitting();
        }

        /**
         * Determines if this is a final state
         * 
         * @return <code>true</code> if this is an final state.
         */
        public boolean isFinal() {
            return node.isFinal();
        }

        /**
         * Gets the hmm tree node representing the unit
         * 
         * @return the unit lex node
         */
        protected SearchState  getNode() {
            return node;
        }

        /**
         * Returns the word sequence for this state
         * 
         * @return the word sequence
         */
        public WordSequence getWordHistory() {
            return wordSequence;
        }

        public Object getLexState() {
            return node;
        }

        /**
         * Returns the list of successors to this state
         * 
         * @return a list of SearchState objects
         */
        public SearchStateArc[] getSuccessors() {
            SearchStateArc[] arcs = getCachedArcs();
            if (arcs == null) {
                arcs = getSuccessors(node);
                putCachedArcs(arcs); 
            } 
            return arcs;
        }

        /**
         * Returns the list of successors to this state
         * 
         * @return a list of SearchState objects
         */
        protected SearchStateArc[] getSuccessors(SearchState theNode) {
            SearchStateArc[] nodes = theNode.getSuccessors();
            SearchStateArc[] arcs = new SearchStateArc[nodes.length];
            // System.out.println("Arc: "+ this);
            for (int i = 0; i < arcs.length; i++) {
                SearchState nextNode = nodes[i].getState();
                //  System.out.println(" " + nextNode);
                if (nextNode instanceof WordSearchState) {
                    arcs[i] = createWordStateArc((WordSearchState) nextNode,nodes[i], this);
                } else if (nextNode instanceof UnitSearchState) {
                    arcs[i] = createUnitStateArc((UnitSearchState) nextNode,nodes[i], this);
                } else 
		    if (nextNode instanceof HMMSearchState) {
			arcs[i]= createHMMStateArc((HMMSearchState) nextNode,nodes[i],this);
		    }
		    else {
			arcs[i] = createStateArc( nextNode,nodes[i], this);
		    }
            }
            return arcs;
        }

        /**
         * Creates a word search state for the given word node
         * 
         * @param wordNode
         *                the wordNode
         * 
         * @return the search state for the wordNode
         */
        protected SearchStateArc createWordStateArc(WordSearchState wordNode,
						    SearchStateArc flatArc, SearchState previous) {
            // System.out.println("CWSA " + wordNode + " fup " + fixupProb);
            float probability = logOne;
            float arcProbability = logOne;
            Word nextWord = wordNode.getPronunciation().getWord();
            WordSequence nextWordSequence = wordSequence;
            if (!nextWord.isFiller()) {
                nextWordSequence = wordSequence.addWord(nextWord, languageModel
							.getMaxDepth());
                probability = languageModel.getProbability(nextWordSequence);
		// System.out.println("LP " + nextWordSequence + " " +
                // logProbability);
                probability *= languageWeight;}
	    else
		probability = logOne;
	    arcProbability = probability + flatArc.getLanguageProbability();
	    // fin du if not filler before 
	    return new GrapheLinguistWordState(wordNode,
					      nextWordSequence.trim(languageModel.getMaxDepth() - 1), flatArc.getInsertionProbability(), arcProbability);
            
        }

        /**
         * Creates a unit search state for the given unit node
         * 
         * @param hmmNode
         *                the unit node
         * 
         * @return the search state
         */
        SearchStateArc createStateArc(SearchState hmmNode,SearchStateArc flatArc , GrapheState previous) {
	    // System.out.println("CUSA " + hmmNode);
            // if we want a unit state create it, otherwise
            // get the first hmm state of the unit
	    return new GrapheLinguistState(hmmNode, getWordHistory(),flatArc.getAcousticProbability(),
					   flatArc.getInsertionProbability(),flatArc.getLanguageProbability());
        }
	SearchStateArc createHMMStateArc(HMMSearchState hmmNode,SearchStateArc flatArc , GrapheState previous) {
	    // System.out.println("CUSA " + hmmNode);
            // if we want a unit state create it, otherwise
            // get the first hmm state of the unit
	    return new GrapheLinguistHMMState(hmmNode, getWordHistory(),flatArc.getAcousticProbability(),flatArc.getInsertionProbability(),flatArc.getLanguageProbability());
	    
        }
        /**
         * Creates a unit search state for the given unit node
         * 
         * @param endNode
         *                the unit node
         * @param previous
         *                the previous state
         * 
         * @return the search state
         */
        SearchStateArc createUnitStateArc(UnitSearchState  endNode,SearchStateArc flatArc , GrapheState previous) {
            
            return new GrapheLinguistUnitState(endNode, getWordHistory(),flatArc.getInsertionProbability(),flatArc.getLanguageProbability());
        }

        /**
         * Returns the string representation of this object
         * 
         * @return the string representation
         */
        public String toString() {
            return "lt-" + node + " " + getProbability() + "{" + wordSequence
		+ "} :" + getOrder();
        }

        /**
         * Returns a pretty version of the string representation for this
         * object
         * 
         * @return a pretty string
         */
        public String toPrettyString() {
            return toString();
        }

        /**
         * Gets the successor arcs for this state from the cache
         *
         * @return the next set of arcs for this state, or null if
         * none can be found or if caching is disabled.
         */
        SearchStateArc[] getCachedArcs() {
            if (cacheEnabled) {
                SearchStateArc[] arcs = (SearchStateArc[]) arcCache.get(this);
                if (arcs != null) {
                    cacheHits++;
                }
                if (++cacheTrys % 1000000 == 0) {
                    System.out.println("Hits: " + cacheHits 
				       + " of " + cacheTrys + " " + 
				       ((float) cacheHits) / cacheTrys * 100f);
                }
                return arcs;
            } else {
                return null;
            }
        }

        /**
         * Puts the set of arcs into the cache
         *
         * @param arcs the arcs to cache.
         */
        void putCachedArcs(SearchStateArc[] arcs) {
            if (cacheEnabled) {
                arcCache.put(this, arcs);
            }
        }

        public int getOrder() {
	    return node.getOrder();
	}
    }

    /**
     * Represents a unit in the search space
     */
    public class GrapheLinguistUnitState extends GrapheState
	implements UnitSearchState {

        float logLanguageProbability;
        float logInsertionProbability;

        /**
         * Constructs a LexTreeUnitState
         * 
         * @param wordSequence
         *                the history of words
         */
        GrapheLinguistUnitState(SearchState node, WordSequence wordSequence,
				float insertionProbability,
				float languageProbability) {
            super(node, wordSequence);
            logLanguageProbability = languageProbability;
            logInsertionProbability = insertionProbability;
            // System.out.println("LTEUS " + logLanguageProbability + " " +
            // logInsertionProbability);
        }

        /**
         * Returns the base unit assciated with this state
         * 
         * @return the base unit
         */
        public Unit getUnit() {
            return ((UnitSearchState)getNode()).getUnit();
        }

        /**
         * Generate a hashcode for an object
         * 
         * @return the hashcode
         */
	// public int hashCode() {
        //    return super.hashCode() * 17 + 423;
        //} je croyais que c'etait fait

        /**
         * Gets the acoustic probability of entering this state
         * 
         * @return the log probability
         */
        public float getInsertionProbability() {
            return logInsertionProbability;
        }

        /**
         * Gets the language probability of entering this state
         * 
         * @return the log probability
         */
        public float getLanguageProbability() {
            return logLanguageProbability;
        }

        /**
         * Determines if the given object is equal to this object
         * 
         * @param o
         *                the object to test
         * @return <code>true</code> if the object is equal to this
         */
        public boolean equals(Object o) {
            if (o == this) {
                return true;
            } else if (o instanceof GrapheLinguistUnitState) {
                GrapheLinguistUnitState other = (GrapheLinguistUnitState) o;
                return super.equals(o);
            } else {
                return false;
            }
        }

        /**
         * Returns the unit node for this state
         * 
         * @return the unit node
         */
      
        /**
         * Returns the list of successors to this state
         * 
         * @return a list of SearchState objects
         */
	//       public SearchStateArc[] getSuccessors() {
	//  SearchStateArc[] arcs = getCachedArcs();
        //    if (arcs == null) {
        //        HMMNode[] nodes = getHMMNodes(getEndNode());
        //        arcs = new SearchStateArc[nodes.length];
	//
	//  if (generateUnitStates) {
	//                     for (int i = 0; i < nodes.length; i++) {
	//                         arcs[i] = new LexTreeUnitState(nodes[i], 
	//                             getWordHistory(), getSmearTerm(), 
	//                             getSmearProb(), logOne, logOne,
	//                             this.getNode());
	//                     }
	//                 } else {
	//                     for (int i = 0; i < nodes.length; i++) {
	//                         HMM hmm = nodes[i].getHMM();
	//                         arcs[i] = new LexTreeHMMState(nodes[i], 
	//                                 getWordHistory(), getSmearTerm(), 
	//                                 getSmearProb(), hmm.getInitialState(), 
	//                                 logOne, logOne, logOne, this.getNode());
	//                     }
	//                 }
	//                 putCachedArcs(arcs);
	//             }
	//             return arcs;
	//         }

        public String toString() {
            return super.toString() + " Unit";
        }
	
    }
    
    /**
     * Represents a HMM state in the search space
     */
    public class GrapheLinguistHMMState extends GrapheState implements HMMSearchState {
	
        private float logLanguageProbability;
        private float logInsertionProbability;
        private float logAcousticProbability;
	
	
        /**
         * Constructs a LexTreeHMMState
         * 
         * @param hmmNode
         *                the hmm state associated with this unit
         * 
         * @param wordSequence
         *                the word history
         * 
         * @param languageProbability
         *                the probability of the transition
         * @param insertionProbability
         *                the probability of the transition
         * @param acousticProbability
         *                the probability of the transition occuring
         */
        GrapheLinguistHMMState(SearchState hmmNode, WordSequence wordSequence,
			       float acousticProbability, float insertionProbability,
			       float languageProbability ) {
            super(hmmNode, wordSequence);
            this.logLanguageProbability = languageProbability;
            this.logInsertionProbability = insertionProbability;
            this.logAcousticProbability = acousticProbability;
        }

        /**
         * Gets the ID for this state
         * 
         * @return the ID
         */
        public String getSignature() {
            return super.getSignature() + "-HMM-";
        }
	
        /**
         * returns the hmm state associated with this state
         * 
         * @return the hmm state
         */
        public HMMState getHMMState() {
            return ((HMMSearchState)getNode()).getHMMState();
        }
	
	/**
         * Determines if the given object is equal to this object
         * 
         * @param o
         *                the object to test
         * @return <code>true</code> if the object is equal to this
         */
        public boolean equals(Object o) {
            if (o == this) {
                return true;
            } else if (o instanceof GrapheLinguistHMMState) {
                GrapheLinguistHMMState other = (GrapheLinguistHMMState) o;
                return super.equals(o);
            } else {
                return false;
            }
        }

        /**
         * Gets the acoustic probability of entering this state
         * 
         * @return the log probability
         */
        public float getAcousticProbability() {
            return logAcousticProbability;
        }
	
	/**
	 * Gets the language probability of entering this state
	 * 
	 * @return the log probability
	 */
	public float getLanguageProbability() {
	    return logLanguageProbability;
        }
	
	/**
	 * Gets the language probability of entering this state
	 * 
	 * @return the log probability
	 */
	public float getInsertionProbability() {
	    return logInsertionProbability;
	}
	
	
    }
    
    /**
     * Represents a non emitting hmm state
     */
    public class GrapheLinguistState extends GrapheState {
	private float logLanguageProbability;
        private float logInsertionProbability;
        private float logAcousticProbability;
	
        /**
         * Constructs a NonEmittingLexTreeHMMState
         * 
         * @param hmmState
         *                the hmm state associated with this unit
         * 
         * @param wordSequence
         *                the word history
         * 
         * @param probability
         *                the probability of the transition occuring
         */
        GrapheLinguistState(SearchState node, WordSequence wordSequence,
			    float acousticProbability, float insertionProbability,
			    float languageProbability ) {
            super(node, wordSequence);
            this.logLanguageProbability = languageProbability;
            this.logInsertionProbability = insertionProbability;
            this.logAcousticProbability = acousticProbability;
        }

    }

    /**
     * Represents a word state in the search space
     */
    public class GrapheLinguistInitState extends  GrapheState implements WordSearchState {
	GrapheLinguistInitState(SearchState node,WordSequence wordSequence) {
	    super(node, wordSequence);
	}
	public  Pronunciation getPronunciation() {
	    return sentenceStartWordArray[0].getPronunciations(null)[0];
	}
	public boolean isFinal() {
	    return false;
	}
	public boolean isWordStart() {
	    return true;
	}
	public String toString() {
	    return "WS " + getProbability() + "{" + getWordHistory()
		+ "} :"+ getNode() +":"+ getOrder();
	    //   return super.toString() + "unit pre" +lastNode + "fin pre";
	}

    }
    public class GrapheLinguistWordState extends GrapheState
	implements
	    WordSearchState {
  
	private float logLanguageProbability;
        private float logInsertionProbability;
  
     

        /**
         * Constructs a LexTreeWordState
         * 
         * @param wordNode
         *                the word node
         * 
         * @param wordSequence
         *                the sequence of words triphone context
         * 
         * @param logProbability
         *                the probability of this word occuring
         *  
         */
        GrapheLinguistWordState(SearchState  wordNode,
				WordSequence wordSequence, float insertionProba,
				float logProbability) {

            super(wordNode, wordSequence);
            // System.out.println("LTWS " + wordSequence);
	    this.logInsertionProbability = insertionProba;
            this.logLanguageProbability = logProbability;
        }

        /**
         * Gets the word pronunciation for this state
         * 
         * @return the pronunciation for this word
         */
        public Pronunciation getPronunciation() {
            return ((WordSearchState) getNode()).getPronunciation();
        }

        /**
         * Determines if this is a final state
         * 
         * @return <code>true</code> if this is an final state.
         */
 
        /**
         * Gets the unique signature for this state. The signature building
         * code is slow and should only be used for non-time-critical tasks
         * such as plotting states.
         * 
         * @return the signature
         */
        public String getSignature() {
            return super.getSignature() ;
        }

        /**
         * Determines if the given object is equal to this object
         * 
         * @param o
         *                the object to test
         * @return <code>true</code> if the object is equal to this
         */
        public boolean equals(Object o) {
            if (o == this) {
                return true;
            } else if (o instanceof GrapheLinguistWordState) {
                GrapheLinguistWordState other = (GrapheLinguistWordState) o;
                return super.equals(o);
            } else {
                return false;
            }
        }

        /**
         * Gets the language probability of entering this state
         * 
         * @return the log probability
         */
        public float getLanguageProbability() {
            return logLanguageProbability;
        }
	public float getInsertionProbability() {
            return logInsertionProbability;
        }

        /**
         * Returns true if this LexTreeWordState indicates the start of a word.
         * Returns false if this LexTreeWordState indicates the end of a word.
         *
         * @return true if this LexTreeWordState indicates the start of a word,
         *         false if this LexTreeWordState indicates the end of a word
         */
        public boolean isWordStart() {
            return ((WordSearchState) getNode()).isWordStart();
        }       
	public boolean isFinal() {
	    return ((WordSearchState) getNode()).getPronunciation().getWord().isSentenceEndWord(); //je ne fais pas d'acoutique
	    // a optimiser dans flatLinguist 
	}
	public String toString() {
	    return "WS " + getProbability() + "{" + getWordHistory()
		+ "} :"+ getNode() +":"+ getOrder();
	    //   return super.toString() + "unit pre" +lastNode + "fin pre";
	}
    }


    class ArcCache extends LinkedHashMap {
        protected boolean removeEldestEntry(Map.Entry eldest) {
            return size() > maxArcCacheSize;
        }
    }
    
}


