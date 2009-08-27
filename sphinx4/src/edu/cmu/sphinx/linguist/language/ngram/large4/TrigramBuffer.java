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

package edu.cmu.sphinx.linguist.language.ngram.large4;



/**
 * Implements a buffer for trigrams read from disk.
 */
class TrigramBuffer extends NGramBuffer {
    
    /**
     * Constructs a TrigramBuffer object with the given byte[].
     *
     * @param trigramsOnDisk the byte[] with trigrams
     * @param numberNGrams the number of trigram follows in the byte[]
     */
    public TrigramBuffer(byte[] trigramsOnDisk,
			 int numberNGrams, boolean bigEndian, int bytesPerIDField) {
        super(trigramsOnDisk, numberNGrams, bigEndian, bytesPerIDField);
    }


    /**
     * Finds the trigram probability ID for the given third word in a trigram.
     *
     * @param thirdWordID the ID of the third word
     *
     * @return the Trigram Probability ID of the given third word
     */
    public TrigramProbability findTrigram(int thirdWordID) {

        // System.out.println("Looking for: " + thirdWordID);

        int mid, start = 0, end = getNumberNGrams()-1;
       // TrigramProbability trigram = null;
        //int trigram = -1;
        while ((end - start) > 0) {
            mid = (start + end)/2;
            int midWordID = getWordID(mid);
	    if (midWordID < thirdWordID) {
                start = mid + 1;
            } else // if (midWordID > thirdWordID) {
                end = mid;
//            } else {
//		bigram = getTrigramProbability(mid);
//                break;
//	    }
        }
        if (end!=getNumberNGrams()-1 &&  thirdWordID== getWordID(end) ) 
        	return getTrigramProbability(end);
        return null;
    }


       
    /**
     * Returns the TrigramProbability of the nth follower.
     *
     * @param nthFollower which follower
     *
     * @return the TrigramProbability of the nth follower
     */
    public final TrigramProbability getTrigramProbability(int nthFollower) {
        int nthPosition = nthFollower * LargeQuadrigramModel.ID_FIELDS_PER_TRIGRAM * getBytesPerIDField();
        setPosition(nthPosition);
        int wordID = readIDField();
        int probID = readIDField();
        int backoffID = readIDField();
        int firstQuadrigram = readIDField();

        return (new TrigramProbability
                (nthFollower, wordID, probID, backoffID, firstQuadrigram));
    }
}
