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
import java.util.Locale;
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

public class FBPhoneDouble implements Configurable,FB {
    public final static String PROP_ACTIVE_LIST_FACTORY = "activeListFactory";
    private Logger logger;
    String name;
    private LogMath logMath;
    private Loader loader;
    private double beamGaus;
    private double beam;
    private float lw;
    private double [][]globalMean;
    private double [][]globalVar;
    private double [][]globalMixture;
 
    private double [][]globalMondeMean;
    private double [][]globalMondeVar;
    private double [][]globalMondeMixture;
    private double [] logMax;
    private double alphaPrimeFinal;
    private boolean aligner;
    private double logLikeAudioMax;
    // aligner =true lattice from aligner word before state
    // and <s> first word   ...... last Phone </s>
    // = false lattice from lextree word after state
    //and <s> first's phones....   words finals </s>
    public void register(String name, Registry registry)
	throws PropertyException {
	this.name = name;
	registry.register("loader",PropertyType.COMPONENT);
	registry.register("logMath", PropertyType.COMPONENT);
	registry.register("beamGaus", PropertyType.DOUBLE);
	registry.register("beam", PropertyType.DOUBLE);
	registry.register("languageWeight", PropertyType.FLOAT);
	registry.register("aligner", PropertyType.BOOLEAN);
	registry.register("logLikeAudioMax",PropertyType.FLOAT);
    }
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
	logMath = (LogMath) ps.getComponent("logMath", LogMath.class);
	loader = (Loader) ps.getComponent("loader",Loader.class);
	beamGaus=(ps.getDouble("beamGaus",1e-100));
	beam = (ps.getDouble("beam",1e-100));
	lw= ps.getFloat("languageWeight",10.0f);
	aligner= ps.getBoolean("aligner",false);
	logLikeAudioMax=ps.getFloat("logLikeAudioMax",7000.0f)*lw;
    }

    public String getName() {
	return name;
    }
    ArrayList<HashMap <SentenceHMMState,Double>> alpha= new  ArrayList<HashMap<SentenceHMMState,Double>>(400);
    public SentenceHMMState  doForward(SentenceHMMState initialState,Data []data) {
	// en cas d'aliner 2 word consecutif au debut ,sinon 2 words en end
	HashMap <SentenceHMMState,Double> alphaCur,nextAlpha;
	alphaCur=new  HashMap <SentenceHMMState,Double>(1);
	alphaPrimeFinal=0.0;
	logMax= new double[data.length];
	if (aligner) {
	   	for (SearchStateArc arc: initialState.getSuccessors()) {
		    WordState toNode= (WordState) arc.getToState();
		    alphaCur.put(toNode,(double) arc.getProbability());
		    toNode.setAlphaPrime(0.0); //in linear
		}  
	}
	else
	    initialState.setAlphaPrime(0.);
	    alphaCur.put(initialState,1.0);

	for (int t=0; t< data.length; t++) {
	    if (alpha.size()<=t ) {
		alpha.add(new HashMap<SentenceHMMState,Double>());
	    }
	    nextAlpha = alpha.get(t);
	    nextAlpha.clear();
	    if (logger.isLoggable(Level.FINE))
		logger.fine("Frame : "+ t +" size :"+alphaCur.size());
	    
	      

		
	    for ( SentenceHMMState state : alphaCur.keySet()){
		if (t> state.getEndFrame()+1 ) continue;
		double prob=alphaCur.get(state);
		for (SearchStateArc arc: state.getSuccessors()) {
		    SentenceHMMState toNode= (SentenceHMMState) arc.getToState();
		   
		    if (toNode.getStartFrame()<=t && toNode.getEndFrame()>=t)
			if (toNode.isEmitting()){
			    if (toNode.isStateZero() && !state.isEmitting()) {
				if (logger.isLoggable(Level.FINER))
				    logger.finer(" debut d'un phone :" + toNode +" trame "+ t + " deb:"+ toNode.getStartFrame() );
				if (toNode.getStartFrame()==t)
				    {nextAlpha.put(toNode,1.0);
					double resul=  prob*arc.getProbability();
					toNode.setAlpha(toNode.getAlphaDouble()+resul);
				     
					toNode.setAlphaPrime(toNode.getAlphaPrime()+ resul*
							     state.getDirect().getAlphaPrime());
				    }
				continue;
			    }
			    if (nextAlpha.containsKey(toNode))
				nextAlpha.put(toNode,nextAlpha.get(toNode)+ prob*arc.getProbability());
			    else 
				nextAlpha.put(toNode, prob*arc.getProbability());
			}
		}
	    }
	    double logMaxTrame=0.0;
	    double logMaxAlpha=0.0;
	    for (SentenceHMMState state : nextAlpha.keySet()){
		double temp= ((HMMStateState)state).getScoreDouble(data[t]);
		if (temp>logMaxTrame) logMaxTrame=temp;
		double temp2= Math.pow(state.isStateZero()?state.getAlphaDouble() :
				       state.getDirect().getAlphaDouble(),lw)*nextAlpha.get(state);
		if (temp2*temp > logMaxAlpha) logMaxAlpha=temp2*temp;
	    }
	    if (logger.isLoggable(Level.FINER))
		logger.finer(String.format(Locale.US," trame %4d ac: %8g tt: %8g %8g %8g",t,logMaxTrame,logMaxAlpha,
					   logMath.linearToLog(logMaxTrame ),  logMath.linearToLog(logMaxAlpha )   ));
	    logMaxTrame = logMaxAlpha;
	    logMax[t] = logMaxTrame;
	    double toto;
	    logMaxAlpha=0;
	    for (SentenceHMMState state : nextAlpha.keySet()) {
		nextAlpha.put(state,toto= nextAlpha.get(state) *
			      ((HMMStateState)state).getScoreDouble(data[t])/logMaxTrame);// bizarre mais je ne sais pas faire
		if (toto> logMaxAlpha) logMaxAlpha=toto;
		if (logger.isLoggable(Level.FINEST))
		    logger.finest(" " +state + " sc " + nextAlpha.get(state)+ " lsc "+ logMath.linearToLog(nextAlpha.get(state))+
				  " audio:"+ (((HMMStateState)state).getScoreDouble(data[t]))+ 
				  "  scmod:"+ ((((HMMStateState)state).getScoreDouble(data[t])/logMax[t]))+
				  " toto " + logMath.linearToLog(toto));
	    }
	    if (logger.isLoggable(Level.FINER))
		logger.finer(String.format(Locale.US," trame %4d max apres norme: %8g l %8g ", t,logMaxAlpha,
					   logMath.linearToLog( logMaxAlpha)));
	    HashMap<SentenceHMMState,Double> aph=nextAlpha;
	    HashMap<SentenceHMMState,Double> nph=new HashMap<SentenceHMMState,Double> ();
	    for (int ordre =0 ;ordre <=1 ;ordre++){ // ne marche pas pour une limite sup a 1
		for (SentenceHMMState state : aph.keySet()){
		    double prob=aph.get(state);
		    double alphaPrime =0.0;
		    if (ordre==1) { // aph contient des ordres 0 donc le final d'un phone donc state is final phone

			//0) on scale y compris les transitions 
			//1) on note le prob dans l'arc direct pour le retour
			//1 bis on normalize le alphaprime de direct (cela aurait put etre fait il y a longtemps mais ....
			//          et on ajoute phoneAcc 
			//2) on transmets le prob+alphazero dans les suivants
			//3) on corrige le alphacur pour la suite (surtout  pour le sil precedent </s>, on pourrait faire autrement
			// si on a le droit de passer
			if (state.getEndFrame() !=t) continue;
			SentenceHMMState zero= state.getDirect();
			state.getDirect().setProbability(prob);
			if (logger.isLoggable(Level.FINER)) {
			    logger.finer(String.format(Locale.US,"fin de phone zer:%s alpha' %7g acc %7g alpha %7g ",
						       zero,zero.getAlphaPrime(), zero.getPhoneAcc(), zero.getAlphaDouble()));}
 
			double post = (zero.getAlphaDouble());
			alphaPrime= (post >10e-200) ? zero.getAlphaPrime()/post+zero.getPhoneAcc()  :0;
			zero.setAlphaPrime(alphaPrime);//= (zero.getAlphaPrime()/ logMath.logToLinear(zero.getAlpha()))+
			                              //zero.getPhoneAcc());
			prob=Math.pow(prob,1.0/lw);
			
			if (logger.isLoggable(Level.FINER))
			    logger.finer ("st :" + state+ " frame" + t + " prob:"
					  + prob+ " alpha :"+(prob* state.getDirect().getAlphaDouble()) +" a':" +
					  zero.getAlphaPrime()+ " " + alphaPrime);
			prob =prob*state.getDirect().getAlphaDouble();
			
			nextAlpha.put(state,prob);
		    }
		    if (prob >beam)
		    for (SearchStateArc arc: state.getSuccessors()) {
			SentenceHMMState toNode= (SentenceHMMState) arc.getToState();
			if (toNode.getStartFrame()<=t && toNode.getEndFrame()>=t)
			    if (toNode.getOrder()==ordre){
				

				double probtemp=    prob*arc.getProbability();
				if (ordre ==1){
				    if (logger.isLoggable(Level.FINER)) {
					logger.finer("word:"+toNode +":" + toNode.getAlphaPrime() + " "+prob+ " " + probtemp+ " :"+alphaPrime  );} 
				    toNode.setAlphaPrime(toNode.getAlphaPrime()+alphaPrime*(probtemp));}
				if (nph.containsKey(toNode))
				    nph.put(toNode,(nph.get(toNode)+ probtemp));
				else {
				    nph.put(toNode, probtemp);
				    
				}
			    }
		    }
		}
		if (ordre==1)  //normalize les alphaprimes des words
		    for (SentenceHMMState w: nph.keySet()) {
			double alphaLin=(nph.get(w));
			
			w.setAlphaPrime(alphaLin>1e-200  ? w.getAlphaPrime()/alphaLin : 0 );
			if (logger.isLoggable(Level.FINER)) {
			    logger.finer("word:"+w +":"  +w.getAlphaPrime() + " "+nph.get(w)  );} 
		    }
		nextAlpha.putAll(nph);
		aph=nph;
		nph=new HashMap<SentenceHMMState,Double> (aph.size());
	    }
	    alphaCur = nextAlpha;
	}
	
	if (aligner) {  for (SentenceHMMState state : alphaCur.keySet()){
		logger.finer("fin du jeu " +state+ " " + alphaCur.get(state));}
	    for (SentenceHMMState state : alphaCur.keySet()){
		logger.finer("fin du jeu " +state+ " " + alphaCur.get(state));
		if ( state.isFinal()){
		    alphaPrimeFinal= state.getAlphaPrime();
		    return state;
		}
	    }
	    clear(data.length);
	    return null;
	}
	else {// two words consecutif en final
	    boolean vu=false;
	    double prob=0.0;
	    double phoneAcc=0.0;
	    SentenceHMMState finalState=null;
// 		// state est dedans mais ces predecesseurs sont encore dans alphaCur deja dedans
// 		//donc ce code est a ne pas executer et je le supprime
// 		for (SentenceHMMState state : alphaCur.keySet())
// 		if (state.isFinal()) {
// 		    logger.fine("</s> is true word");
// 		    vu=true;
// 		    finalState=state;
// 		    prob= alphaCur.get(state);
// 		    logger.fine(" post de vrai mot" + logMath.logToLinear(prob));
// 		}
	   
	    for (SentenceHMMState state : alphaCur.keySet()){
		
		for (SearchStateArc arc: state.getSuccessors()) 
		    if (arc.getToState().isFinal()) {
			phoneAcc +=state.getDirect().getAlphaPrime()*
			    ( alphaCur.get(state)*arc.getProbability());
			if (logger.isLoggable(Level.FINER)){
			    logger.fine( "fin " + arc+ " p:"+alphaCur.get(state));
			}
			if (vu) prob=(prob+alphaCur.get(state)*arc.getProbability());
			else {finalState=(SentenceHMMState)arc.getToState();
			    vu=true;
			    prob=alphaCur.get(state)*arc.getProbability();
			    
			}
		    }
		}
	    if (vu) {
		alphaCur.put(finalState,prob);
		finalState.setAlphaPrime(phoneAcc/(prob));
	    }
	    else clear(data.length);
	    if (finalState!=null) alphaPrimeFinal= finalState.getAlphaPrime();
	    else alphaPrimeFinal=0.0;
	    return finalState;
	    
	
	}
    }
    double lamasse;
    public double getAlphaPrime() {
	return alphaPrimeFinal;
    }
    public float vrais(SentenceHMMState s, int trame) {
	if (!s.isStateZero()) logger.info(" a' of final :"+ s.getDirect().getAlphaPrime());
	return logMath.linearToLog(alpha.get(trame).get(s));
    }
    private void clear(int t){
	for (int i =0 ;i<t;i++)
	    if (i<alpha.size()) alpha.get(i).clear();
    }





    protected final static String NUM_SENONES = "num_senones";
    protected final static String NUM_GAUSSIANS_PER_STATE = "num_gaussians";
     protected final static String NUM_STREAMS = "num_streams";
    private class MaClasse extends 	HashMap<SentenceHMMState,Double>{
    }

    public boolean doBackward(SentenceHMMState initState, SentenceHMMState finalState, Data data[],String s){
	logger.severe("usage anormal de doBackward de FBPhone avec vraisemblance : " +s);
	return doBackward(initState, finalState, data,Float.parseFloat(s));
    }
    public boolean doBackward(SentenceHMMState initState, SentenceHMMState finalState, Data data[]) {
	return doBackward(initState, finalState, data,logMath.linearToLog(alpha.get(data.length-1).get(finalState)));
	//for back compatibility
    }


    private boolean doBackward(SentenceHMMState initState, SentenceHMMState finalState, Data data[],float logNormVrais) {
	//aligner meme chose 2 word au debut si vrai sinon en fin
	double papSentence;
	double totalMpePlus=0.0,totalMpeMoins=0.0;
	ArrayList <double []> accumMixture;
	ArrayList <double [][]> accumMean;
	ArrayList <double [][]> accumVar;
	ArrayList <double []> mondeMixture;
	ArrayList <double [][]> mondeMean;
	ArrayList <double [][]> mondeVar;
	double [] scoreDouble=null;


	double normVrais= logMath.logToLinear(logNormVrais);
	HashMap <Integer,Integer> vu= new HashMap<Integer,Integer>();
	MaClasse [] beta= new  MaClasse[2];
	MaClasse [] noEmit= new MaClasse[2];
	for (int i =0; i<beta.length ;i++)
	    beta[i] = new  MaClasse();
	for (int i =0; i<noEmit.length ;i++)
	    noEmit[i] = new  MaClasse();

	HashMap<SentenceHMMState,Double> oldBeta, nextBeta,alphaCur,alphaPred,betaStateZero;
	Pool senones = loader.getSenonePool();
	Pool means= loader.getMeansPool();
	Pool mixtureWeights= loader.getMixtureWeightPool();
	int nMixture= mixtureWeights.size() ;
	int nGaus=means.size();
	int nGausByMixture=mixtureWeights.getFeature(NUM_GAUSSIANS_PER_STATE, 0);
	int vectLen= ((FloatData) data[0]).getValues().length;
	if (nGausByMixture*nMixture!=nGaus || nMixture != mixtureWeights.getFeature(NUM_SENONES, 0))
	    throw new Error("incoherence "+ (nGausByMixture*nMixture) + " "+
			    nGaus + " " +  nMixture + " "+ mixtureWeights.getFeature(NUM_SENONES, 0));
	accumMixture= new ArrayList<double[]>();
	accumMean=new ArrayList <double [][]>();
	accumVar=new ArrayList < double [][]>();
	mondeMixture= new ArrayList<double[]>();
	mondeMean=new ArrayList <double [][]>();
	mondeVar=new ArrayList < double [][]>();
	float [] cd= new float[vectLen];
	int select=0;
	nextBeta=beta[select];
	select= (select+1) %2;
	oldBeta=beta[select];
	if (normVrais< alpha.get(data.length-1).get(finalState))
	    normVrais= alpha.get(data.length-1).get(finalState);
	normVrais= 1.0/normVrais;
	papSentence =(alpha.get(data.length-1).get(finalState)*normVrais);
	logger.info("papSentence corrigé :"+ papSentence);
	double cAvg= finalState.getAlphaPrime();
	if (aligner) cAvg -=1;
	// MMI la somme des occupation/trame =papSentence et
	// et non 1
	// puisqu'il y a un rapport entre num et den
	// il ne faut donc pas normaliser la papSentence a 1.0
	double beamRelatif = papSentence*beam;
	if (aligner)
	    //	noEmit[finalState.getOrder()].put(finalState,-alpha.get(data.length-1).get(finalState));
	    {    noEmit[finalState.getOrder()].put(finalState,normVrais);
	
		finalState.setBetaPrime(0.0);
	    }
	//mmie
	else {	finalState.setBetaPrime(0.0);
	    //	    float prob = -alpha.get(data.length-1).get(finalState);
	    double prob = 1.0 ;//normVrais;
	    alphaCur=alpha.get(data.length-1);
	    for (SearchStateArc arc: finalState.getPredecessors()) {
		SentenceHMMState fromNode= (SentenceHMMState)arc.getFromState();
		if (! alphaCur.containsKey(fromNode)) continue;
		if (fromNode.getOrder() != 1)
		    logger.fine("</s> est phoneme "+ arc+ " "+ ((HMMStateState)fromNode).getHMMState()+
				" p:" + prob + " al " +alphaCur.get(fromNode));
		else 
		    logger.fine(" arc " +arc+ " prob:" +prob+ " al " +alphaCur.get(fromNode));
		assert( !noEmit[fromNode.getOrder()].containsKey(fromNode));
		noEmit[fromNode.getOrder()].put(fromNode,
						prob*arc.getProbability());
		fromNode.getDirect().setBetaPrime(0.0);
	    }
	}
	if (true) {double masse=0.0;  

	       for (int order=1 ;order>=0; order--)
		   for (SentenceHMMState state: noEmit[order].keySet()) { 
		       masse+=(noEmit[order].get(state)*alpha.get(data.length-1).get(state));
			if (logger.isLoggable(Level.FINER))
			    logger.finer(String.format(Locale.US,"%s logmas %14.8e masse %14.8e noemit %14.8e alpha %14.8e",
						       state.toString(),logMath.linearToLog(masse),
						       masse,noEmit[order].get(state), alpha.get(data.length-1).get(state) ));
		   }
	       double borne;;
 	       borne = 1.0/(masse);
	       logger.fine(String.format(Locale.US,"jjj m:%8g b:%8g v:%8g",masse,borne,normVrais));
	       for (int order=1 ;order>=0; order--)
		   for (SentenceHMMState state: noEmit[order].keySet()) { 
		       noEmit[order].put(state,noEmit[order].get(state)*borne);
		   }
	}
	if (logger.isLoggable(Level.FINE)) {double masse=0.0;
	       for (int order=1 ;order>=0; order--)
		   for (SentenceHMMState state: noEmit[order].keySet()) { 
		       masse+=(noEmit[order].get(state)*alpha.get(data.length-1).get(state));
		   }
	       logger.fine(String.format(Locale.US,"masav %8g",masse));
	}


	for (int t=data.length-1;t>=0; t--) {
	    float [] md= ((FloatData) data[t]).getValues();

	    for (int i=0 ; i<md.length;i++)
		cd[i] = md[i]*md[i];
	    alphaCur=alpha.get(t);
	    alphaPred =(t>0) ? alpha.get(t-1):null;
	    //on commence par le noemit
	    // et on augmente le oldBeta;
	    double tempPost=0;
	    double tempPrime=0;
	    /// j'en suis ici
	    for (int order=1 ;order>=0; order--) {
		for (SentenceHMMState state: noEmit[order].keySet()) { 
		    double prob= noEmit[order].get(state);
		    double betaPrime, betaLinear= (prob);
		    betaPrime= (betaLinear>1e-200) ? state.getDirect().getBetaPrime()/betaLinear :0;
		    // la probleme avec zerostate
		    state.getDirect().setBetaPrime(betaPrime) ; //=state.getDirect().getBetaPrime()/logMath.logToLinear(prob));
		    if (order==0) {
			SentenceHMMState zero= (SentenceHMMState) state.getDirect();
			double probDirect=state.getDirect().getProbabilityDouble();
			double probDirectLw=Math.pow(probDirect,1.0/lw);
			zero.setBeta(prob*probDirectLw);
			zero.setGammaMpe(zero.getAlphaPrime()+zero.getBetaPrime()-cAvg);
			if (t==data.length-1) {
			    double toto=(zero.getBetaDouble()*zero.getAlphaDouble());
			    tempPost+=toto;
			    tempPrime+= zero.getAlphaPrime()*toto;
			    if (logger.isLoggable(Level.FINE))
				logger.fine(String.format(Locale.US,"trame fin %s  b:%.5f aprim: %.5f  bprim: %5g",
							  zero,
							  toto,
							  zero.getAlphaPrime(),
							  zero.getBetaPrime()));
			}
		        
			prob=   //sigma post =1 sur le phoneme
			    (prob*zero.getAlphaDouble()*probDirectLw)/probDirect ;//   proba d'emprunter ce phoneme / post=1 sur le pho 
		    } 
		    //System.err.println("------" +state+"   pr:" +prob);
		    for (SearchStateArc arc: state.getPredecessors()) {
			SentenceHMMState fromNode= (SentenceHMMState)arc.getFromState();
			if (logger.isLoggable(Level.FINEST))
			    logger.finest("from:" +fromNode + " alpha:" +alphaCur.containsKey(fromNode));
			if (! alphaCur.containsKey(fromNode)) continue;
			if (fromNode.isEmitting()) {
			    if (oldBeta.containsKey(fromNode)) 
				oldBeta.put(fromNode,
					    (oldBeta.get(fromNode)+prob*arc.getProbability()));
		       
			    else
				oldBeta.put(fromNode,prob*arc.getProbability());
			}
			else {
			    fromNode.getDirect().setBetaPrime(fromNode.getDirect().getBetaPrime()+
							      betaPrime*(prob*arc.getProbability()));
			    if (noEmit[fromNode.getOrder()].containsKey(fromNode)) 
				noEmit[fromNode.getOrder()].put(fromNode,
								(noEmit[fromNode.getOrder()].get(fromNode)+prob*arc.getProbability()));
			
			    else
				noEmit[fromNode.getOrder()].put(fromNode,prob*arc.getProbability());
			}
		    }
		}
		noEmit[order].clear();
	    }
	    logger.fine("trame t:"+t+ "  size:"+ oldBeta.size()+" tp: " + tempPost+ "tmpe" +tempPrime);
	    double cumul=0.0;
	    double cumulMpePlus=0.0;
	    double cumulMpeMoins=0.0;
	    //	    double cumulasLinear=0.0;
	    for (SentenceHMMState state : oldBeta.keySet()) {
		//on y est j'ai alpha beta et il faut faire les counts
		//cumulasLinear=cumulasLinear+ oldBeta.get(state)*alphaCur.get(state));
		double post=(oldBeta.get(state)*alphaCur.get(state));
		double  proba;
		if (logger.isLoggable(Level.FINEST))
		    logger.finest("s:" + state+ " alpha:" + alphaCur.get(state)+ " beta:" + 
				  oldBeta.get(state) + " post:" + post);  
		cumul+= post;
		if (!state.isEmitting()) throw new Error("pas emmetteur :"+state);
		//proba= ((HMMStateState) state).getScoreDouble(data[t]);

		if (post<beamRelatif) {
		    proba= ((HMMStateState) state).getScoreDouble(data[t]);
		}
		else {
		    GaussianMixture gm =(GaussianMixture)((SenoneHMMState)((HMMStateState) state).getHMMState()).getSenone();
		    double [] scoreDoubleTemp= gm.calculateComponentScoreWithMemoryDouble(data[t]);
		    if (scoreDouble==null || scoreDouble.length != scoreDoubleTemp.length)
			scoreDouble= new double[scoreDoubleTemp.length];
		    int id =(int) gm.getID();
		    if (id>24 && id <200) {
			new Error("monophone " + gm +  " get(id) :"+ senones.get(id)+ " s:"+
				  state + " ss:" + ((HMMStateState) state).getHMMState());
		    }
		
		    if (gm != senones.get(id)) throw
			new Error(" ce n'est pas range comme il faut :"+
				  gm + " get(id) :"+ senones.get(id));
		    if ( !vu.containsKey(id))
			{vu.put(id,accumMixture.size());
			    accumMixture.add(new double[nGausByMixture]);
			    accumMean.add(new double[nGausByMixture][vectLen]);
			    accumVar.add(new double[nGausByMixture][vectLen]);
			    
			    mondeMixture.add(new double[nGausByMixture]);
			    mondeMean.add(new double[nGausByMixture][vectLen]);
			    mondeVar.add(new double[nGausByMixture][vectLen]);
			}
		    // on passe en prob et le cumul
		    double maxi =scoreDoubleTemp[0];
		    for (int i=1; i<scoreDoubleTemp.length ;i++)
			maxi=Math.max(maxi,scoreDoubleTemp[i]);
		    double lemax=(maxi*beamGaus);
		    double cumulLocal=0.0;
		    for (int i=0; i<scoreDoubleTemp.length ;i++) {
			scoreDouble[i]=scoreDoubleTemp[i];
			cumulLocal += scoreDouble[i];
			if (scoreDouble[i]<lemax) scoreDouble[i]=0.0;
		    } 
		    proba=cumulLocal;
		    //proba=logMath.linearToLog(cumulLocal);
		    double poidsLocal= post/cumulLocal;
		    if (proba <1.0e-300 && post>1.0e-20){ 
			logger.warning(String.format(Locale.US,"probafaible %3d id %4d sc %5e max:%5e post:%5e pl:%5e\n",t,id,proba,maxi,post,poidsLocal));
			clear(data.length);
			for (Integer idTemp : vu.keySet()) 
		       ((GaussianMixture)senones.get(idTemp)).clearComponentData();
			return false; }
		    int baseId =id *nGausByMixture;
		    double gammaMpe= state.isStateZero() ? state.getGammaMpe() : state.getDirect().getGammaMpe();

		    if (gammaMpe >0){
			cumulMpePlus +=gammaMpe*post;
			poidsLocal*=gammaMpe;
			for (int i=0; i< scoreDouble.length ;i++) {
			    if (scoreDouble[i]==0.0) continue;
			    if (false && id==22) 
				System.err.format(Locale.US," %2d %4g %4g,",i, scoreDouble[i],scoreDouble[i]*poidsLocal);
			    scoreDouble[i] *=poidsLocal;
			    accumMixture.get(vu.get(id))[i] +=scoreDouble[i];
			    
			    for(int l=0; l<vectLen; l++) {
				accumMean.get(vu.get(id))[i][l]+=md[l]*scoreDouble[i];
				accumVar.get(vu.get(id))[i][l]+=cd[l]*scoreDouble[i];
			    }
			}
		    }
		    else
			{ 
			    cumulMpeMoins -=gammaMpe*post;
			    poidsLocal *=-gammaMpe;
			    for (int i=0; i< scoreDouble.length ;i++) {
				if (scoreDouble[i]==0.0) continue;
				scoreDouble[i] *=poidsLocal;
				if (scoreDouble[i] > 1e+5) {
				    logger.warning(
						   String.format(Locale.US,
								 "score haut gau: %d,mix %d, s:%7g post:%7g mpe: %7g poidsloc %7g proba %7g",
								 id,i,scoreDouble[i],post,gammaMpe,poidsLocal,proba));
				System.err.format("proba:%7g ", ((HMMStateState) state).getScoreDouble(data[t]));
				if (false) 
				    {for(int tt =0 ; tt< scoreDouble.length; tt++)
				    System.err.format(" (%d,%7g)",i,scoreDoubleTemp[i]);
				    System.err.println();}
				}

				
				mondeMixture.get(vu.get(id))[i] +=scoreDouble[i];
				
				if (mondeMixture.get(vu.get(id))[i]>=1.0e+10) {
				     logger.warning(
						   String.format(Locale.US,
								 "monde haut m:%7g %7g",mondeMixture.get(vu.get(id))[i],scoreDouble[i]));
						   }
				for(int l=0; l<vectLen; l++) {
				    mondeMean.get(vu.get(id))[i][l]+=md[l]*scoreDouble[i];
				    mondeVar.get(vu.get(id))[i][l]+=cd[l]*scoreDouble[i];
				}
			    }
			
			}
		}
	      
		proba=oldBeta.get(state)*(proba/logMax[t]);
		//on propage 

		for (SearchStateArc arc: state.getPredecessors()) {
		    SentenceHMMState fromNode= (SentenceHMMState)arc.getFromState();
		    if (alphaPred !=null && ! alphaPred.containsKey(fromNode)) continue;
		    if (fromNode.isEmitting()) {
			if (nextBeta.containsKey(fromNode)) 
			    nextBeta.put(fromNode,
					 (nextBeta.get(fromNode)+proba*arc.getProbability()));
		    
			else
			    nextBeta.put(fromNode,proba*arc.getProbability());
		    }
		    else {
			// c'est zero
			double betaZero= state.getBetaDouble();
			fromNode.getDirect().setBetaPrime(fromNode.getDirect().getBetaPrime()+
							  (state.getBetaPrime()+ state.getPhoneAcc())*
							  (betaZero*arc.getProbability()));

			
			if (noEmit[fromNode.getOrder()].containsKey(fromNode)) 
			    noEmit[fromNode.getOrder()].put(fromNode,
							    (noEmit[fromNode.getOrder()].get(fromNode)+betaZero*arc.getProbability()));
		    
			else
			    noEmit[fromNode.getOrder()].put(fromNode,betaZero*arc.getProbability());
		    }
		}
	    }
	    totalMpePlus+=cumulMpePlus;
            totalMpeMoins+=cumulMpeMoins;
	    if (Math.abs(cumul-papSentence)/papSentence > 0.001|| (!aligner && Math.abs(cumulMpePlus-cumulMpeMoins)/(cAvg>2?cAvg:2) >0.01 ) )
		{ logger.warning("probleme trame " + t+ " post:" +cumul + 
				 " papSent:" +papSentence +
			    
				 " mpeplus:" + cumulMpePlus + " mpesomme:" +((cumulMpePlus- cumulMpeMoins)) + " cavg" +cAvg);
		   clear(data.length);
		   for (Integer id : vu.keySet()) 
		       ((GaussianMixture)senones.get(id)).clearComponentData();
		   return false;}
	    else
		logger.fine(String.format(Locale.US,"verif trame %4d post:%.4f mpeplus:%7g mpemoin:%7g mpetotal:%7g papsent:%.4f",
					  t,cumul,cumulMpePlus,cumulMpeMoins,
					  cumulMpePlus-cumulMpeMoins,
					  papSentence));
	    //on doit permutter
	    oldBeta.clear();
	    nextBeta=beta[select];
	    select= (select+1) %2;
	    oldBeta=beta[select];
	}
	//logiquement 
	//on devrait recuperer l'initial quelque part  avec un beta de 0
	// sauver les locales
	//
	if (aligner) {// le <s> est juste avant le first word
	    boolean vuInitial=false;
	    double prob=0.0;
	    double gammaMpe=0.0;
	    for (SentenceHMMState state : noEmit[initState.getOrder()].keySet())
		for (SearchStateArc arc: state.getPredecessors()) {
		    SentenceHMMState fromNode= (SentenceHMMState)arc.getFromState();
		    if (fromNode==initState){
			System.err.println("prob:"+ state +" beta:"+noEmit[initState.getOrder()].get(state)+
					   " betaprime:"+ state.getBetaPrime()/( noEmit[initState.getOrder()].get(state)));
			gammaMpe +=state.getBetaPrime()*
			    (arc.getProbability()) ; // on aurait pu mettre state.getDirect().getBeatPrime();
			if (vuInitial) prob= (prob+noEmit[initState.getOrder()].get(state)*arc.getProbability());
			else {
			    vuInitial=true;
			    prob=noEmit[initState.getOrder()].get(state)*arc.getProbability();
			}
		    }
		}
	    if (vuInitial) {
		noEmit[initState.getOrder()].put(initState,prob);
		initState.setBetaPrime(gammaMpe/(prob));
	    }
	    
	    if (vuInitial && logger.isLoggable(Level.FINE))
		logger.fine("fini aligne:"+ initState+ " beta:"+ noEmit[initState.getOrder()].get(initState)+ " betaPrime: " + initState.getBetaPrime());
	}
	
	clear(data.length);
	logMax=null;
	if (initState.getOrder()!=noEmit.length-1 && !noEmit[initState.getOrder()].containsKey(initState) &&
	    Math.abs(
		     (noEmit[initState.getOrder()].get(initState.getOrder()))
		     -papSentence)/papSentence>0.001      || Math.abs(initState.getBetaPrime() - finalState.getAlphaPrime())>0.01*initState.getBetaPrime())
	    { String s= " " + initState.getOrder()+ "  " + 
		    noEmit[initState.getOrder()].containsKey(initState);
		//logger.warning(s+"bbii");
		if (noEmit[initState.getOrder()].containsKey(initState)){
		    s = s + " beta(init) :"+  
			
			(noEmit[initState.getOrder()].get(initState));
		   
		    s = s+ String.format(" rvrais %7g ", Math.abs( (noEmit[initState.getOrder()].get(initState))
								       -papSentence)/papSentence);
		}
		s=s+ " papSEnt:"+ papSentence;
		s= s + " beta prime init : " + initState.getBetaPrime() + " alphaprime final : " +  finalState.getAlphaPrime();
		s= s+ String.format("rapport %7g %7g",Math.abs(initState.getBetaPrime() - finalState.getAlphaPrime()),0.01*initState.getBetaPrime());
		logger.warning(s) ;
		return false;
	    } else
	    logger.info(String.format(Locale.US,"fini back: %s beta:%10g beta':%7g totMpeplus %7g somme : %10g",
				      initState,
				      (noEmit[initState.getOrder()].get(initState)),
				      initState.getBetaPrime(),
				      totalMpePlus,totalMpePlus-totalMpeMoins));
        cumulPlus += totalMpePlus;
	cumulMoins += totalMpeMoins;
	for (Integer id : vu.keySet()) 
	    ((GaussianMixture)senones.get(id)).clearComponentData();
	logger.fine("allocate global if necessary");
	if (true && ( aligner || totalMpePlus> 10* Math.abs(totalMpePlus-totalMpeMoins))) { // j'ai pas la place sur le portable 
	    if (globalMixture== null) 
		globalMixture=new double[nMixture][nGausByMixture] ;
	    if (globalMean == null)
	    globalMean= new double[nGaus][vectLen] ;
	    if (globalVar==null)
	    globalVar=new double[nGaus][vectLen] ;
	    
	    if (globalMondeMixture== null) 
		globalMondeMixture=new double[nMixture][nGausByMixture] ;
	    if (globalMondeMean == null)
		globalMondeMean= new double[nGaus][vectLen] ;
	    if (globalMondeVar==null)
		globalMondeVar=new double[nGaus][vectLen] ;
	    for (Integer id : vu.keySet()) {
		if (id>24 && id <200) 
		    new Error("monophone in accu  get(id) :"+ senones.get(id));
	    int baseId= id*nGausByMixture;
	    //System.err.format("%4d",id);
	    for (int imix=0 ; imix< nGausByMixture;imix++) {
		globalMixture[id][imix]+= accumMixture.get(vu.get(id))[imix];
		//System.err.format(Locale.US," %7g",globalMixture[id][imix]);
		for (int  v=0; v<vectLen ;v++) {
		    globalVar[baseId+imix][v] += accumVar.get(vu.get(id))[imix][v];
		    globalMean[baseId+imix][v] += accumMean.get(vu.get(id))[imix][v];
		}
	
	    }
	    //System.err.println();
	    assert( 8>=nGausByMixture || mondeMixture.get(vu.get(id))[8]<1.0e+10);
	    for (int imix=0 ; imix< nGausByMixture;imix++) {
	       
		globalMondeMixture[id][imix]+= mondeMixture.get(vu.get(id))[imix];
		for (int  v=0; v<vectLen ;v++) {
		    globalMondeVar[baseId+imix][v] += mondeVar.get(vu.get(id))[imix][v];
		    globalMondeMean[baseId+imix][v] += mondeMean.get(vu.get(id))[imix][v];
		}
	    }
	    
	    
	    }
	}
	if (false)
	    for (int testId=25 ; testId<100 ; testId++){
		double testCumul=0.0;
	    
		for (int imix=0 ; imix< nGausByMixture;imix++) testCumul +=globalMixture[testId][imix];
		if (testCumul !=0.0)
		    System.err.format(Locale.US,"monophone vu %4d\n",testId);
		    }

	return true;
    }
    double cumulPlus=0.0,cumulMoins=0.0;
    public void save (Saver saver) throws java.io.IOException {
        logger.info(String.format(Locale.US," fini p %7g  m %7g",cumulPlus,cumulMoins));
	if (globalMixture!=null) {
	saver.save(globalMixture,"P");
	saver.save("meanfn",globalMean,globalMixture[0].length,"P");
	saver.save("varfn",globalVar,globalMixture[0].length,"P");
	saver.save(globalMixture,globalMean,globalVar,"P");
	saver.save(globalMondeMixture,"M");
	saver.save("meanfn",globalMondeMean,globalMondeMixture[0].length,"M");
	saver.save("varfn",globalMondeVar,globalMondeMixture[0].length,"M");
	saver.save(globalMondeMixture,globalMondeMean,globalMondeVar,"M");
	}


    }
}


			
