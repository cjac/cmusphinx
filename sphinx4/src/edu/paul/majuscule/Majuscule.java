package edu.paul.majuscule;
import edu.cmu.sphinx.util.Utilities;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.linguist.WordSequence;
import  edu.cmu.sphinx.linguist.dictionary.Word;
import  edu.cmu.sphinx.linguist.dictionary.Dictionary;
import  edu.cmu.sphinx.linguist.language.ngram.LanguageModel;
import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.logging.Logger;
import edu.cmu.sphinx.util.LogMath;
import java.util.ArrayList;
import java.io.PrintStream;
import java.util.Locale;

class Moi extends ArrayList <Token> {};


public class Majuscule implements Configurable {

  public final static String PROP_LOG_MATH = "logMath";

    /**
     * A sphinx property for the language model to be used by this grammar
     */
    public final static String PROP_LANGUAGE_MODEL = "languageModel";

    /**
     * Property that defines the dictionary to use for this grammar
     */
    public final static String PROP_DICTIONARY = "dictionary";




   private LanguageModel languageModel;
    private LogMath logMath;
    private Dictionary dictionary;
    private Logger logger;
    private String name;
public void register(String name, Registry registry)
            throws PropertyException {
        this.name = name;
        
        registry.register(PROP_LOG_MATH, PropertyType.COMPONENT);
        registry.register(PROP_LANGUAGE_MODEL, PropertyType.COMPONENT);
        registry.register(PROP_DICTIONARY, PropertyType.COMPONENT);
}

    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
	logMath = (LogMath) ps.getComponent(PROP_LOG_MATH, LogMath.class);
        
        languageModel = (LanguageModel) ps.getComponent(PROP_LANGUAGE_MODEL,
                LanguageModel.class);
        dictionary = (Dictionary) ps.getComponent(PROP_DICTIONARY,
                Dictionary.class);
    }

     public String getName() {
	 return name;
     }
    
    HashMap<WordSequence,Token> best;
    Moi  active[];
    public void allocate() throws java.io.IOException {
	dictionary.allocate();
	languageModel.allocate();
	active = new Moi[2];
	for (int i =0 ; i<active.length ;i++)
	    active[i]=new Moi();
	best = new HashMap<WordSequence,Token>();
    }
    
    public void deallocate () {
	dictionary.deallocate();
	languageModel.deallocate();
    }

    public void traiter(ArrayList <String []>  lesLignes , PrintStream ctmWriter) {
	int numero=0;
	int origine=0,cible=1;
	int maxHist=languageModel.getMaxDepth();
	String mot=lesLignes.get(numero)[4];
	if (mot.equals(Dictionary.SENTENCE_START_SPELLING)) {
	    State state = new State(WordSequence.getWordSequence(dictionary.getSentenceStartWord()),dictionary.getSentenceStartWord());
	    active[origine].add(new Token(0.0f,null,numero,state));
	    numero++;
	}
	else {   
	    State state=new State(WordSequence.getWordSequence(dictionary.getSentenceStartWord()),null);
	    active[origine].add(new Token(0.0f,null,-1,state));
	}
	for (;numero<lesLignes.size();numero++,cible=origine, origine=(cible+1)%2){
	    best.clear();
	    //logger.info( String.format(" ori: %d, cible: %d",origine,cible));
	    List<Word> lesChoix=dictionary.getChoix(lesLignes.get(numero)[4]);
	    if (lesChoix!=null) 
		for (Token pred : active[origine]) {
		    for (Word w : lesChoix) {
			WordSequence ns=pred.history.ws.addWord(w,maxHist);
			float nscore=pred.score + languageModel.getProbability(ns);
			Token leBest=best.get(ns);
			if (leBest==null) {
			    Token t= new Token(nscore,pred,numero,new State(ns,w));
			    best.put(ns,t);
			    active[cible].add(t);
			}
			else {
			    if (nscore> leBest.score) {
				leBest.score=nscore;
				leBest.pred=pred;
				assert (leBest.ligne==numero);
				leBest.history=new State(ns,w);
			    }
			}
		    }
		}
	    else {
		for (Token pred : active[origine]) {
		    active[cible].add(new Token(pred.score,pred,numero,new State(pred.history.ws,lesLignes.get(numero)[4])));
		}
		
		
	    }
	    logger.fine("taille "+active[cible].size());
	    active[origine].clear();
	}
	Token fini=null;;
	float max=-Float.MAX_VALUE;
	
	for (Token t:active[origine]) 
	    if (t.score>max) { max=t.score;fini=t;}
	    else logger.fine("---->"+t.score+ " "+max);
	logger.fine("max:" +max+ "ori:"+ origine);
	while (fini !=null && fini.ligne>=0) {
	    logger.fine(fini.history.tete.toString()+ " "+fini.ligne);
	    lesLignes.get(fini.ligne)[4]=fini.history.tete.toString();
	    fini=fini.pred;
	}
	for (String m[] : lesLignes){
	    boolean pasStart=false;
	    for (String s:m) {
		if (pasStart) ctmWriter.print(" ");
		else pasStart=true;
		ctmWriter.print(s);
	    }
	    ctmWriter.println();
	}
	    
    }
}