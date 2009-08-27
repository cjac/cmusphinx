

package edu.paul.bw;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.LogMath;
import java.util.logging.Logger;
import java.util.logging.Level;
import java.util.HashMap;
import java.util.HashSet;
import java.util.ArrayList;
import java.util.Set;
import edu.cmu.sphinx.linguist.acoustic.HMM;
import edu.cmu.sphinx.linguist.acoustic.HMMState;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.Loader;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.SenoneHMMState;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.GaussianMixture;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.Pool;
import edu.cmu.sphinx.frontend.Data;
import edu.cmu.sphinx.frontend.FloatData;


import java.util.ArrayList;
/**
 * forward backward for mmie : not normalization of sentence'pap to one
 * aligner =true word before phone and two words begun the sentence
 *aligner = false word after phone and two consecutif word at end
 */

public interface  FB{
    public SentenceHMMState  doForward(SentenceHMMState initialState,Data []data);


    public float vrais(SentenceHMMState s, int trame);
    public boolean doBackward(SentenceHMMState initState, SentenceHMMState finalState, Data data[],String s);
    public boolean doBackward(SentenceHMMState initState, SentenceHMMState finalState, Data data[]);
    public double getAlphaPrime();
    public void save (Saver saver) throws java.io.IOException ;
}


			
