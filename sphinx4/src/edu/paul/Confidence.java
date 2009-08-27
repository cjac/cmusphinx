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
package edu.paul;

import edu.cmu.sphinx.result.ConfidenceResult;
import edu.cmu.sphinx.result.ConfidenceScorer;
import edu.cmu.sphinx.result.Lattice;
import edu.cmu.sphinx.result.LatticeOptimizer;
import edu.cmu.sphinx.result.MAPConfidenceScorer;
import edu.cmu.sphinx.result.Path;
import edu.cmu.sphinx.result.Result;
import edu.cmu.sphinx.result.WordResult;


import java.io.File;
import java.io.IOException;


import java.text.DecimalFormat;


public class Confidence {

    private static DecimalFormat format = new DecimalFormat("#.###########");

    /**
     * Main method for running the Confidence demo.
     */
    public static  void  confucius(ConfidenceScorer cs, Result result)  {

	ConfidenceResult cr = cs.score(result);
	Path best;
	if (cr==null) {
	    System.err.println("j'ai rate la sauccise \n");
            return;
	}
	 best = cr.getBestHypothesis();

	/* confidence of the best path */
	System.out.println(best.getTranscription());
	System.out.format(
			  "(confidence: %.4g ) ",
			  best.getLogMath().logToLinear
			  ((float)best.getConfidence()));
	   
	System.out.println();
                        
	/*
	 * print out confidence of individual words 
	 * in the best path
	 */
	WordResult[] words = best.getWords();
	for (int i = 0; i < words.length; i++) {
	    WordResult wr = (WordResult) words[i];
	    printWordConfidence(wr);
	}
	System.out.println();
    }

      
    /**
     * Prints out the word and its confidence score.
     *
     * @param wr the WordResult to print
     */
    private static void printWordConfidence(WordResult wr) {
        String word = wr.getWord().getSpelling();

        System.out.print(word);

        /* pad spaces between the word and its score */
        int entirePadLength = 10;
        if (word.length() < entirePadLength) {
            for (int i = word.length(); i < entirePadLength; i++) {
                System.out.print(" ");
            }
        }

        System.out.println
            (" (confidence: " +
             format.format
             (wr.getLogMath().logToLinear((float)wr.getConfidence())) + ")");
    }
}
