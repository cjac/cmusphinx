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
package edu.cmu.sphinx.linguist.dictionary;

import java.io.IOException;
import java.io.InputStream;
import java.util.regex.Pattern;
import java.net.URL;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.logging.Logger;
import java.util.logging.Level;

import edu.cmu.sphinx.linguist.acoustic.Context;
import edu.cmu.sphinx.linguist.acoustic.Unit;
import edu.cmu.sphinx.linguist.acoustic.UnitManager;
import edu.cmu.sphinx.util.ExtendedStreamTokenizer;
import edu.cmu.sphinx.util.StreamFactory;
import edu.cmu.sphinx.util.Timer;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;

/**
 * Creates a dictionary by reading in an ASCII-based Sphinx-3
 * format dictionary. Each line of the dictionary specifies the
 * word, followed by spaces or tab, followed by the pronuncation
 * (by way of the list of phones) of the word. Each word can have
 * more than one pronunciations.
 * For example, a digits dictionary will look like:
 *
 * <pre>
 * ONE                  HH W AH N
 * ONE(2)               W AH N
 * TWO                  T UW
 * THREE                TH R IY
 * FOUR                 F AO R
 * FIVE                 F AY V
 * SIX                  S IH K S
 * SEVEN                S EH V AH N
 * EIGHT                EY T
 * NINE                 N AY N
 * ZERO                 Z IH R OW
 * ZERO(2)              Z IY R OW
 * OH                   OW
 * </pre>
 * <p>
 * In the above example, the words "one" and "zero" have two pronunciations
 * each.
 * <p>
 * This dictionary will read in all the words and its pronunciation(s)
 * at startup. Therefore, if the dictionary is big, it will take longer
 * to load and will consume more memory.
 */
public class MajusculeDictionary implements Dictionary {
    // ----------------------------------
    // configuration variables
    // ----------------------------------
    private String name;
    private  Logger logger;
    private boolean addSilEndingPronunciation;
    private boolean allowMissingWords;
    private boolean createMissingWords;
    private String wordReplacement;
    private URL wordDictionaryFile;
    private URL fillerDictionaryFile;
    private boolean allocated = false;
    private UnitManager unitManager;
    private boolean compound=false;
    private Map <String,java.util.List<Word>> choix;
    private Map wordDictionary;
    private Map fillerDictionary;
    private Timer loadTimer;
    
    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
            throws PropertyException {
        this.name = name;
        registry.register(PROP_DICTIONARY, PropertyType.RESOURCE);
        registry.register(PROP_FILLER_DICTIONARY, PropertyType.RESOURCE);
        registry.register(PROP_ADD_SIL_ENDING_PRONUNCIATION, 
                          PropertyType.BOOLEAN);
        registry.register(PROP_WORD_REPLACEMENT, PropertyType.STRING);
        registry.register(PROP_ALLOW_MISSING_WORDS, PropertyType.BOOLEAN);
        registry.register("compound", PropertyType.BOOLEAN);
        registry.register(PROP_CREATE_MISSING_WORDS, PropertyType.BOOLEAN);
        registry.register(PROP_UNIT_MANAGER, PropertyType.COMPONENT);
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
        wordDictionaryFile = ps.getResource(PROP_DICTIONARY);
        }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#getName()
     */
    public String getName() {
        return name;
    }

    /* (non-Javadoc)
     * @see edu.cmu.sphinx.linguist.dictionary.Dictionary#allocate()
     */
    public void allocate() throws IOException {
        
        if (!allocated) {
            loadTimer = Timer.getTimer("DictionaryLoad");
            loadTimer.start();
            // NOTE: "location" can be null here, in which case the
            // "wordDictionaryFile" and "fillerDictionaryFile" should
            // contain the full path to the Dictionaries.
            logger.info("Loading dictionary from: " + wordDictionaryFile);
	    choix =new HashMap <String,List<Word>>();
            wordDictionary = 
                loadDictionary(wordDictionaryFile.openStream(), false);
	            loadTimer.stop();
            allocated = true;
	    if (logger.isLoggable(Level.FINEST)) dump();// cela peut servir
	    logger.info("end loading dicts : " +wordDictionary.size() + " ch:" + choix.size());
        }
    }

    /* (non-Javadoc)
     * @see edu.cmu.sphinx.linguist.dictionary.Dictionary#deallocate()
     */
    public void deallocate() {
        if (allocated) {
            fillerDictionary = null;
            wordDictionary = null;
            loadTimer = null;
            allocated = false;
	    choix=null;
        }
    }
        
    private void  addChoix(String cle, Word w) {
	List l = choix.get(cle);
	if (l==null) {
	    l=new LinkedList<Word>();
	    choix.put(cle,l);
	}
	l.add(w);
    }
    public java.util.List<Word> getChoix(String nom) {
	return choix.get(nom);
    }
    
    /**
     * Loads the given sphinx3 style simple dictionary from the given
     * InputStream. The InputStream is assumed to contain ASCII data.
     * 
     * @param inputStream
     *                the InputStream of the dictionary
     * @param isFillerDict
     *                true if this is a filler dictionary, false otherwise
     * 
     * @throws java.io.IOException
     *                 if there is an error reading the dictionary
     */
    private Map loadDictionary(InputStream inputStream, boolean isFillerDict)
            throws IOException {
        Map dictionary = new HashMap();
        java.io.BufferedReader buf = new java.io.BufferedReader (new java.io.InputStreamReader(inputStream));
        String word;
	String ligne;
	java.util.regex.Pattern pat= java.util.regex.Pattern.compile("\\s+");
        while ((ligne=buf.readLine()) != null) {
	    word=pat.split(ligne)[0];
            word = removeParensFromWord(word);
            String base = word.toLowerCase(); //paul
	    Word ici= new Word(word,null,false);
	    logger.finer("lire:"+word);
	    dictionary.put(word,ici);
	    addChoix(base,ici);
	}
	inputStream.close();
	buf.close();
	return dictionary;
    }

    /**
     * Returns a new string that is the given word but with the ending
     * parenthesis removed.
     * <p>
     * Example:
     * 
     * <pre>
     *  "LEAD(2)" returns "LEAD"
     *  "LEAD" returns "LEAD"
     *  @param word
     *  the word to be stripped
     *  
     *  @return the given word but with all characters from the first
     *  open parentheses removed
     */
    private String removeParensFromWord(String word) {
        if (word.charAt(word.length() - 1) == ')') {
            int index = word.lastIndexOf('(');
            if (index > 0) {
                word = word.substring(0, index);
            }
        }
        return word;
    }

    /**
     * Returns a Word object based on the spelling and its classification. The
     * behavior of this method is affected by the properties wordReplacement,
     * allowMissingWords, and createMissingWords.
     * 
     * @param text
     *                the spelling of the word of interest.
     * 
     * @return a Word object
     * 
     * @see edu.cmu.sphinx.linguist.dictionary.Word
     */
    public Word getWord(String text) {
        //text = text.toLowerCase();
        Word word = lookupWord(text);
	if (word==null) logger.warning(text  + " is not in dico");
        return word;
    }

    /**
     * Lookups up a word
     * 
     * @param spelling
     *                the spellling of the word
     * 
     * @return the word or null
     */
    private Word lookupWord(String spelling) {
        Word word = (Word) wordDictionary.get(spelling);
        if (word == null) {
            logger.info("ratee :"+spelling); 
        }
        return word;
    }

    /**
     * Returns the sentence start word.
     * 
     * @return the sentence start word
     */
    public Word getSentenceStartWord() {
        return getWord(SENTENCE_START_SPELLING);
    }

    /**
     * Returns the sentence end word.
     * 
     * @return the sentence end word
     */
    public Word getSentenceEndWord() {
        return getWord(SENTENCE_END_SPELLING);
    }

    /**
     * Returns the silence word.
     * 
     * @return the silence word
     */
    public Word getSilenceWord() {
        return getWord(SILENCE_SPELLING);
    }

    /**
     * Returns the set of all possible word classifications for this
     * dictionary.
     * 
     * @return the set of all possible word classifications
     */
    public WordClassification[] getPossibleWordClassifications() {
        return null;
    }

    /**
     * Returns a string representation of this FullDictionary in alphabetical
     * order.
     * 
     * @return a string representation of this FullDictionary
     */
    public String toString() {
        SortedMap sorted = new TreeMap(wordDictionary);
        String result = "";
        sorted.putAll(fillerDictionary);
        for (Iterator i = sorted.keySet().iterator(); i.hasNext();) {
            String text = (String) i.next();
            Word word = getWord(text);
            List<Word> ch = choix.get(text.toLowerCase());
            result += (word + " ");
            if (ch!=null) for (Word w:ch) {
                result += ("   " + w.toString() + " ");
	       
            }result +="\n";
        }
        return result;
    }

    /**
     * Gets the set of all filler words in the dictionary
     * 
     * @return an array (possibly empty) of all filler words
     */
    public Word[] getFillerWords() {
        return null;
    }
    /**
     * Dumps this FullDictionary to System.out.
     */

    public void dump() {
        System.out.println(wordDictionary.size() + " words");


        SortedMap sorted = new TreeMap(wordDictionary);
        for (Iterator i = sorted.keySet().iterator(); i.hasNext();) {
	    String result = "";        
            String text = (String) i.next();
            Word word = getWord(text);
            List<Word> pronunciations = choix.get(text.toLowerCase());
            result += (word);
	    if (word.getClasse() != null) {
		result += " " + word.getClasse();
		result += " ";
	    }
	    else result += " ";
	    if (pronunciations !=null)
		for (Word w: pronunciations) 
		    result += ("   " + w.toString() + " ");
	    System.out.println(result);   
        }
        
    }
}
