/*
 * Copyright 1999-2004 Carnegie Mellon University.
 * Portions Copyright 2004 Sun Microsystems, Inc.
 * Portions Copyright 2004 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 *
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */
package edu.cmu.sphinx.result;

import java.util.Collections;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Comparator;

/**
 * A confusion set is a set of words with their associated posteriors.
 * In Java terms it's a SortedMap from posteriors to sets of WordResults, but
 * the class is called a set because that's what this beast is known as
 * in the literature.
 * 
 * @author pgorniak
 */ 
public class ConfusionSet extends ArrayList<WordResult> {
    private boolean tri=false;
    private boolean hasFiller=false;
    private int beginTime;
    private int endTime;
    public void setEndTime(int t){
	endTime=t;
    }
    public int getEndTime() {
	return endTime;
    }
 public void setBeginTime(int t){
	beginTime=t;
    }
    public int getBeginTime() {
	return beginTime;
    }




    public void setHasFiller(boolean b) {
	this.hasFiller=b;
    }
    public boolean hasFiller() {
	return hasFiller;
    }
    /**
     * Add a word hypothesis to this confusion set.
     * 
     * @param word the hypothesis to add
     */
    static Comparator<WordResult> comp= new Comparator<WordResult> (){ 
	public int compare (WordResult wr1,WordResult wr2){
	    return Double.compare(wr1.getConfidence(),wr2.getConfidence());}
    };

    public void addWordHypothesis(WordResult word) {
         add(word);
    }
    public void sort(){
	Collections.sort(this,comp);
    }
    
    /**
     * Return the set of best hypothesis. This will usually contain one
     * hypothesis, but might contain more case there are some that have
     * exactly equal confidences.
     * 
     * @return a set of best hypotheses
     * 
     */
    //    public WordResult get(int n){
    //	if (!tri) {
    //	    tri=true; /// this order is fondamental sort call get
    //    Collections.sort(this,comp);
    //}
    //return super.get(n);
    //}
    public WordResult getBestHypothesis() {
	return get(size()-1); 
    }
    
    /**
     * Get the highest posterior (confidence) stored in this set.
     *
     * @return the highes posterior
     */
    public double getBestPosterior() {
        return get(size()-1).getConfidence();
    }

    /**
     * Returns the WordResult in this ConfusionSet for the given word.
     *
     * @param word the word to look for
     *
     * @return the WordResult for the given word, or null if no WordResult
     *     for the given word is found
     */
    public WordResult getWordResult(String word) {
        for (Iterator<WordResult> i = this.iterator(); i.hasNext(); ) {
             WordResult wordResult =  i.next();
	     String resultSpelling 
		 = wordResult.getWord().getSpelling();
	     if (resultSpelling.equals(word)) {
		 return wordResult;
	     }
	}
        
        return null;
    }

    /**
     * Dumps out the contents of this ConfusionSet.
     *
     * @param name the name of the confusion set
     */
    public void dump(String name) {
        System.out.print(name + " :");
        for (Iterator <WordResult> i = this.iterator(); i.hasNext(); ) {
	    WordResult wordResult = i.next();
	    System.out.print
		(" " + 
		 wordResult.getPronunciation().getWord().getSpelling());
	}
        
        System.out.println();
    }
    public String toString() {
	StringBuffer s=new StringBuffer();
        for ( WordResult wr: this){   
	    s.append(wr.toString());
	    s.append(" ");
	    s.append(wr.getConfidence());
	    s.append(",");
	}
	return s.toString();
    }

}

