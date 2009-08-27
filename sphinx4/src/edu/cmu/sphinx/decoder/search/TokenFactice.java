package edu.cmu.sphinx.decoder.search;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Set;

import edu.cmu.sphinx.decoder.scorer.Scoreable;
import edu.cmu.sphinx.frontend.Data;
import edu.cmu.sphinx.linguist.HMMSearchState;
import edu.cmu.sphinx.linguist.SearchState;
import edu.cmu.sphinx.linguist.WordSearchState;
import edu.cmu.sphinx.linguist.UnitSearchState;
import edu.cmu.sphinx.linguist.acoustic.HMMState;
import edu.cmu.sphinx.linguist.acoustic.Unit;
import edu.cmu.sphinx.linguist.dictionary.Word;

/**
 * Represents a single state in the recognition trellis. Subclasses of
 * a token are used to represent the various emitting state.
 *
 * All scores are maintained in LogMath log base
 *
 */
public class TokenFactice extends Token  { 
 public TokenFactice(float logAcousticScore, float logLanguageScore,
                 Token predecessor) {
     super( logAcousticScore,  logLanguageScore,  predecessor);
    }
    private static DecimalFormat scoreFmt = new DecimalFormat("0.0000000E00");
    private static DecimalFormat numFmt = new DecimalFormat("0000");

    public String toString() {
	return   "factice "  +  ((getPredecessor()!=null) ?  numFmt.format(getPredecessor().getFrameNumber()) + scoreFmt.format(getPredecessor().getScore()) :   "     ") + "-> " +  scoreFmt.format(getAcousticScore())  + " " +  scoreFmt.format(getLanguageScore()) + " ::  ";
    }
} 
