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
class QuadrigramBuffer extends NGramBuffer {
    
    /**
     * Constructs a TrigramBuffer object with the given byte[].
     *
     * @param trigramsOnDisk the byte[] with trigrams
     * @param numberNGrams the number of trigram follows in the byte[]
     */
    public QuadrigramBuffer(byte[] quadrigramsOnDisk,
			 int numberNGrams, boolean bigEndian, int bytesPerIDField) {
        super(quadrigramsOnDisk, numberNGrams, bigEndian, bytesPerIDField);
    }


    /**
     * Finds the trigram probability ID for the given third word in a trigram.
     *
     * @param thirdWordID the ID of the third word
     *
     * @return the Trigram Probability ID of the given third word
     */
    public int findProbabilityID(int thirdWordID) {

        // System.out.println("Looking for: " + thirdWordID);

        int mid, start = 0, end = getNumberNGrams();
        // TrigramProbability trigram = null;
        //int trigram = -1;

        while ((end - start) > 0) {
            mid = (start + end)/2;
            int midWordID = getWordID(mid);
	    if (midWordID < thirdWordID) {
                start = mid + 1;
            } else// if (midWordID > thirdWordID) {
                end = mid;
          //  } else {
		//trigram = getProbabilityID(mid);
                //break;
	    //}
        }
                if (end!=getNumberNGrams() &&  thirdWordID== getWordID(end) ) 
                	return getProbabilityID(end);
                return -1 ;
            

     
    }


    /**
     * Returns the TrigramProbability of the nth follower.
     *
     * @param nthFollower which follower
     *
     * @return the TrigramProbability of the nth follower
     */
    public final int getProbabilityID(int nthFollower) {
        int nthPosition = nthFollower * LargeQuadrigramModel.ID_FIELDS_PER_QUADRIGRAM * getBytesPerIDField();
        setPosition(nthPosition + getBytesPerIDField()); // to skip the word ID
        return readIDField();
    }
}
