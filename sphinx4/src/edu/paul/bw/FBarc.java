

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

public class FBarc implements Configurable,FB {
    public final static String PROP_ACTIVE_LIST_FACTORY = "activeListFactory";
    private Logger logger;
    String name;
    private LogMath logMath;
    private Loader loader;
    private float beamGaus;
    private float beam;
    private float lw;
    private double [][]globalMean;
    private double [][]globalVar;
    private double [][]globalMixture;
    private boolean aligner;
    private float logLikeAudioMax;
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
	beamGaus=logMath.linearToLog(ps.getDouble("beamGaus",1e-100));
	beam = logMath.linearToLog(ps.getDouble("beam",1e-100));
	lw= ps.getFloat("languageWeight",10.0f);
	aligner= ps.getBoolean("aligner",false);
	logLikeAudioMax=ps.getFloat("logLikeAudioMax",7000.0f)*lw;
    }
    public double getAlphaPrime() { throw new Error(" pas implemete");}
    public String getName() {
	return name;
    }
    ArrayList<HashMap <SentenceHMMState,Float>> alpha= new  ArrayList<HashMap<SentenceHMMState,Float>>(400);
    public SentenceHMMState  doForward(SentenceHMMState initialState,Data []data) {
	// en cas d'aliner 2 word consecutif au debut ,sinon 2 words en end
	HashMap <SentenceHMMState,Float> alphaCur,nextAlpha;
	alphaCur=new  HashMap <SentenceHMMState,Float>(1);
	if (aligner) {
	   	for (SearchStateArc arc: initialState.getSuccessors()) {
		    WordState toNode= (WordState) arc.getToState();
		    alphaCur.put(toNode,arc.getProbability());
		}  
	}
	else

	alphaCur.put(initialState,logMath.getLogOne());

	for (int t=0; t< data.length; t++) {
	    if (alpha.size()<=t ) {
		alpha.add(new HashMap<SentenceHMMState,Float>());
	    }
	    nextAlpha = alpha.get(t);
	    nextAlpha.clear();
	    if (logger.isLoggable(Level.FINE))
		logger.fine("Frame : "+ t +" size :"+alphaCur.size());
	    
	      

		
	    for ( SentenceHMMState state : alphaCur.keySet()){
		if (t> state.getEndFrame()+1 ) continue;
		float prob=alphaCur.get(state);
		for (SearchStateArc arc: state.getSuccessors()) {
		    SentenceHMMState toNode= (SentenceHMMState) arc.getToState();
		   
		    if (toNode.getStartFrame()<=t && toNode.getEndFrame()>=t)
			if (toNode.isEmitting()){
			    if (toNode.isStateZero() && !state.isEmitting()) {
				if (logger.isLoggable(Level.FINER))
				    logger.finer(" debut d'un phone :" + toNode +" trame "+ t + " deb:"+ toNode.getStartFrame() );
				if (toNode.getStartFrame()==t)
				    {nextAlpha.put(toNode,logMath.getLogOne());
					toNode.setAlpha(
							logMath.addAsLinear(toNode.getAlpha(),
									    prob+arc.getProbability()));
				    }
				continue;
			    }
			    if (nextAlpha.containsKey(toNode))
				nextAlpha.put(toNode,logMath.addAsLinear(nextAlpha.get(toNode),
									 prob+arc.getProbability()));
			    else 
				nextAlpha.put(toNode, prob+arc.getProbability());
			}
		}
	    }
	    
	    for (SentenceHMMState state : nextAlpha.keySet()) {
		nextAlpha.put(state, nextAlpha.get(state) +
			      ((HMMStateState)state).getScore(data[t])-logLikeAudioMax);// bizarre mais je ne sais pas faire
		if (logger.isLoggable(Level.FINEST))
		    logger.finest(" " +state + " sc " + nextAlpha.get(state)+
				  " audio:"+ (((HMMStateState)state).getScore(data[t])/lw)+ 
				  "  scmod:"+ ((((HMMStateState)state).getScore(data[t])-logLikeAudioMax)/lw) );
	    }
	    HashMap<SentenceHMMState,Float> aph=nextAlpha;
	    HashMap<SentenceHMMState,Float> nph=new HashMap<SentenceHMMState,Float> ();
	    for (int ordre =0 ;ordre <=1 ;ordre++){ // ne marche pas pour une limite sup a 1
		for (SentenceHMMState state : aph.keySet()){
		    float prob=aph.get(state);
		    if (ordre==1) { // aph contient des ordres 0 donc le final d'un phone donc state is final phone

			//0) on scale y compris les transitions 
			//1) on note le prob dans l'arc direct pour le retour
			//2) on transmets le prob+alphazero dans les suivants
			//3) on corrige le alphacur pour la suite (surtout  pour le sil precedent </s>, on pourrait faire autrement
			// si on a le droit de passer
			if (state.getEndFrame() !=t) continue;
			state.getDirect().setProbability(prob);

			prob/=lw;
			if (logger.isLoggable(Level.FINER))
			    logger.finer ("st :" + state+ " frame" + t + " prob:"
					  + prob+ " alpha :"+(prob+ state.getDirect().getAlpha()));
			prob =prob+ state.getDirect().getAlpha();

			nextAlpha.put(state,prob);
		    }
		    
		    for (SearchStateArc arc: state.getSuccessors()) {
			SentenceHMMState toNode= (SentenceHMMState) arc.getToState();
			if (toNode.getStartFrame()<=t && toNode.getEndFrame()>=t)
			    if (toNode.getOrder()==ordre){
				if (nph.containsKey(toNode))
				    nph.put(toNode,logMath.addAsLinear(nph.get(toNode),
								       prob+arc.getProbability()));
				else {
				    nph.put(toNode, prob+arc.getProbability());
				    
				}
			    }
		    }
		}
		nextAlpha.putAll(nph);
		aph=nph;
		nph=new HashMap<SentenceHMMState,Float> (aph.size());
	    }
	    alphaCur = nextAlpha;
	}
	
	if (aligner) {  for (SentenceHMMState state : alphaCur.keySet()){
		logger.fine("fin du jeu " +state+ " " + alphaCur.get(state));}
	    for (SentenceHMMState state : alphaCur.keySet()){
		logger.fine("fin du jeu " +state+ " " + alphaCur.get(state));
		if ( state.isFinal())
		    return state;
	    }
	    clear(data.length);
	    return null;
	}
	else {// two words consecutif en final
	    boolean vu=false;
	    float prob=LogMath.getLogZero();
	    SentenceHMMState finalState=null;
	    if (false)
		//
		// state est dedans mais ces predecesseurs sont encore dans alphaCur deja dedans
		//donc ce code est a ne pas executer 
		for (SentenceHMMState state : alphaCur.keySet())
		if (state.isFinal()) {
		    logger.fine("</s> is true word");
		    vu=true;
		    finalState=state;
		    prob= alphaCur.get(state);
		    logger.fine(" post de vrai mot" + logMath.logToLinear(prob));
		}
	    for (SentenceHMMState state : alphaCur.keySet())
		for (SearchStateArc arc: state.getSuccessors()) 
		    if (arc.getToState().isFinal()) 
			if (vu) prob=logMath.addAsLinear(prob,alphaCur.get(state)+arc.getProbability());
			else {finalState=(SentenceHMMState)arc.getToState();
			    vu=true;
			    prob=alphaCur.get(state)+arc.getProbability();
			}
	    if (vu) alphaCur.put(finalState,prob);
	    else clear(data.length);
	    return finalState;
	    
	
	}
    }

    public float vrais(SentenceHMMState s, int trame) {
	return alpha.get(trame).get(s);
    }
    private void clear(int t){
	for (int i =0 ;i<t;i++)
	    if (i<alpha.size()) alpha.get(i).clear();
    }





    protected final static String NUM_SENONES = "num_senones";
    protected final static String NUM_GAUSSIANS_PER_STATE = "num_gaussians";
    protected final static String NUM_STREAMS = "num_streams";
    private class MaClasse extends 	HashMap<SentenceHMMState,Float>{
    }

    public boolean doBackward(SentenceHMMState initState, SentenceHMMState finalState, Data data[],String s){
	return doBackward(initState, finalState, data,Float.parseFloat(s));
    }
    public boolean doBackward(SentenceHMMState initState, SentenceHMMState finalState, Data data[]) {
	return doBackward(initState, finalState, data,alpha.get(data.length-1).get(finalState));

    }


    private boolean doBackward(SentenceHMMState initState, SentenceHMMState finalState, Data data[],float normVrais) {
	//aligner meme chose 2 word au debut si vrai sinon en fin
	ArrayList <double []> accumMixture;
	double papSentence;
	ArrayList <double [][]> accumMean;
	ArrayList <double [][]> accumVar;
	HashMap <Integer,Integer> vu= new HashMap<Integer,Integer>();
	MaClasse [] beta= new  MaClasse[2];
	MaClasse [] noEmit= new MaClasse[2];
	for (int i =0; i<beta.length ;i++)
	    beta[i] = new  MaClasse();
	for (int i =0; i<noEmit.length ;i++)
	    noEmit[i] = new  MaClasse();

	HashMap<SentenceHMMState,Float> oldBeta, nextBeta,alphaCur,alphaPred,betaStateZero;
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
	float [] cd= new float[vectLen];
	int select=0;
	nextBeta=beta[select];
	select= (select+1) %2;
	oldBeta=beta[select];
	if (normVrais< alpha.get(data.length-1).get(finalState))
	    normVrais= alpha.get(data.length-1).get(finalState);
	normVrais= -normVrais;
	papSentence =logMath.logToLinear(alpha.get(data.length-1).get(finalState)+normVrais);
	logger.info("papSentence corrigé :"+ papSentence);
	// MMI la somme des occupation/trame =papSentence et
	// et non 1
	// puisqu'il y a un rapport entre num et den
	// il ne faut donc pas normaliser la papSentence a 1.0
	double beamRelatif = papSentence*beam;
	if (aligner)
	    //	noEmit[finalState.getOrder()].put(finalState,-alpha.get(data.length-1).get(finalState));
	    noEmit[finalState.getOrder()].put(finalState,normVrais);
	//mmie
	else {
	    //	    float prob = -alpha.get(data.length-1).get(finalState);
	    float prob = normVrais;
	    alphaCur=alpha.get(data.length-1);
	    for (SearchStateArc arc: finalState.getPredecessors()) {
		SentenceHMMState fromNode= (SentenceHMMState)arc.getFromState();
		if (! alphaCur.containsKey(fromNode)) continue;
		if (fromNode.getOrder() != 1)
		    logger.fine("</s> est phoneme "+ fromNode + " "+ ((HMMStateState)fromNode).getHMMState());
		noEmit[fromNode.getOrder()].put(fromNode,
						prob+arc.getProbability());
	    }
	}
	for (int t=data.length-1;t>=0; t--) {
	    float [] md= ((FloatData) data[t]).getValues();

	    for (int i=0 ; i<md.length;i++)
		cd[i] = md[i]*md[i];
	    alphaCur=alpha.get(t);
	    alphaPred =(t>0) ? alpha.get(t-1):null;
	    //on commence par le noemit
	    // et on augmente le oldBeta;
	    for (int order=1 ;order>=0; order--) {
		for (SentenceHMMState state: noEmit[order].keySet()) { 
		    float prob= noEmit[order].get(state);
		    if (order==0) {
			SentenceHMMState zero= (SentenceHMMState) state.getDirect();
			float probDirect=state.getDirect().getProbability();
			zero.setBeta(prob+probDirect/lw);

		        prob=  -probDirect  +   //sigma post =1 sur le phoneme
			    prob+ zero.getAlpha()+probDirect/lw ;//   proba d'emprunter ce phoneme 
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
					    logMath.addAsLinear(oldBeta.get(fromNode),prob+arc.getProbability()));
		       
			    else
				oldBeta.put(fromNode,prob+arc.getProbability());
			}
			else {
			    if (noEmit[fromNode.getOrder()].containsKey(fromNode)) 
				noEmit[fromNode.getOrder()].put(fromNode,
								logMath.addAsLinear(noEmit[fromNode.getOrder()].get(fromNode),prob+arc.getProbability()));
			
			    else
				noEmit[fromNode.getOrder()].put(fromNode,prob+arc.getProbability());
			}
		    }
		}
		noEmit[order].clear();
	    }
	    logger.fine("trame t:"+t+ "  size:"+ oldBeta.size());
	    double cumul=0.0;
	    float cumulaslog=logMath.getLogZero();
	    for (SentenceHMMState state : oldBeta.keySet()) {
		//on y est j'ai alpha beta et il faut faire les counts
		cumulaslog=logMath.addAsLinear(cumulaslog,oldBeta.get(state)+alphaCur.get(state));
		double post=logMath.logToLinear(oldBeta.get(state)+alphaCur.get(state));
		float proba;
		if (logger.isLoggable(Level.FINEST))
		    logger.finest("s:" + state+ " alpha:" + alphaCur.get(state)+ " beta:" + 
				  oldBeta.get(state) + " post:" + post);  
		cumul+= post;
		if (!state.isEmitting()) throw new Error("pas emmetteur :"+state);
		proba= ((HMMStateState) state).getScore(data[t]);

		if (post<beamRelatif) {
		    //proba= ((HMMStateState) state).getScore(data[t]);
		}
		else {
		    GaussianMixture gm =(GaussianMixture)((SenoneHMMState)((HMMStateState) state).getHMMState()).getSenone();
		    float [] score= gm.calculateComponentScoreWithMemory(data[t]);
		    int id =(int) gm.getID();
		    if (gm != senones.get(id)) throw
			new Error(" ce n'est pas range comme il faut :"+
				  gm + " get(id) :"+ senones.get(id));
		    if ( !vu.containsKey(id))
			{vu.put(id,accumMixture.size());
			    accumMixture.add(new double[nGausByMixture]);
			    accumMean.add(new double[nGausByMixture][vectLen]);
			    accumVar.add(new double[nGausByMixture][vectLen]);
			}
		    // on passe en prob et le cumul
		    float maxi =score[0];
		    for (int i=1; i<score.length ;i++)
			maxi=Math.max(maxi,score[i]);
		    double lemax=logMath.logToLinear(maxi+beamGaus);
		    double cumulLocal=0.0;
		    double [] scoreDouble= new double [score.length];
		    for (int i=0; i<score.length ;i++) {
			scoreDouble[i] = logMath.logToLinear(score[i]);
			cumulLocal += scoreDouble[i];
			if (scoreDouble[i]<lemax) scoreDouble[i]=0.0;
		    } 
		    //proba=logMath.linearToLog(cumulLocal);
		    double poidsLocal= post/cumulLocal;
		    int baseId =id *nGausByMixture;
		    for (int i=0; i< score.length ;i++) {
			if (scoreDouble[i]==0.0) continue;
			scoreDouble[i] *=poidsLocal;
			accumMixture.get(vu.get(id))[i] +=scoreDouble[i];
			for(int l=0; l<vectLen; l++) {
			    accumMean.get(vu.get(id))[i][l]+=md[l]*scoreDouble[i];
			    accumVar.get(vu.get(id))[i][l]+=cd[l]*scoreDouble[i];
			}
		    }
		}
		proba=oldBeta.get(state)+(proba-logLikeAudioMax);
		//on propage 

		for (SearchStateArc arc: state.getPredecessors()) {
		    SentenceHMMState fromNode= (SentenceHMMState)arc.getFromState();
		    if (alphaPred !=null && ! alphaPred.containsKey(fromNode)) continue;
		    if (fromNode.isEmitting()) {
			if (nextBeta.containsKey(fromNode)) 
			    nextBeta.put(fromNode,
					 logMath.addAsLinear(nextBeta.get(fromNode),proba+arc.getProbability()));
		    
			else
			    nextBeta.put(fromNode,proba+arc.getProbability());
		    }
		    else {
			// c'est zero
			float betaZero= state.getBeta();
			if (noEmit[fromNode.getOrder()].containsKey(fromNode)) 
			    noEmit[fromNode.getOrder()].put(fromNode,
							    logMath.addAsLinear(noEmit[fromNode.getOrder()].get(fromNode),betaZero+arc.getProbability()));
		    
			else
			    noEmit[fromNode.getOrder()].put(fromNode,betaZero+arc.getProbability());
		    }
		}
	    }
	    if (Math.abs(cumul-papSentence)/papSentence > 0.001)
		{ logger.warning("probleme trame " + t+ " post:" +cumul + 
				 " papSent:" +papSentence + " log:"+ logMath.logToLinear(cumulaslog));
		    return false;}
	    else
		logger.fine("verif alpha trame " + t+ " post:" +cumul + 
			    " papSent:" +papSentence);
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
	    float prob=LogMath.getLogZero();
	    for (SentenceHMMState state : noEmit[initState.getOrder()].keySet())
		for (SearchStateArc arc: state.getPredecessors()) {
		    SentenceHMMState fromNode= (SentenceHMMState)arc.getFromState();
		    if (fromNode==initState)
			if (vuInitial) prob= logMath.addAsLinear(prob,noEmit[initState.getOrder()].get(state)+arc.getProbability());
			else {
			    vuInitial=true;
			    prob=noEmit[initState.getOrder()].get(state)+arc.getProbability();
			}
		}
	    if (vuInitial) noEmit[initState.getOrder()].put(initState,prob);
	    
	    if (vuInitial && logger.isLoggable(Level.FINE))
		logger.fine("fini aligne:"+ initState+ " beta:"+ noEmit[initState.getOrder()].get(initState));
	}
	clear(data.length);
	if (initState.getOrder()!=noEmit.length-1 && !noEmit[initState.getOrder()].containsKey(initState) &&
	    Math.abs(
		     logMath.logToLinear(noEmit[initState.getOrder()].get(initState.getOrder()))
		     -papSentence)/papSentence>0.001 )
	    { String s= " " + initState.getOrder()+ "  " + 
		    noEmit[initState.getOrder()].containsKey(initState);
		if (noEmit[initState.getOrder()].containsKey(initState)) 
		    s = s + " beta(init) :"+
			logMath.logToLinear(noEmit[initState.getOrder()].get(initState.getOrder())) + " papSEnt:"+ papSentence;
		logger.warning(s) ;
		return false;
	    }
	for (Integer id : vu.keySet()) 
	    ((GaussianMixture)senones.get(id)).clearComponentData();
	logger.fine("allocate global if necessary");
	if (globalMixture== null) 
	    globalMixture=new double[nMixture][nGausByMixture] ;
	if (globalMean == null)
	    globalMean= new double[nGaus][vectLen] ;
	if (globalVar==null)
	    globalVar=new double[nGaus][vectLen] ;
	for (Integer id : vu.keySet()) {
	    int baseId= id*nGausByMixture;
	    for (int imix=0 ; imix< nGausByMixture;imix++) {
		globalMixture[id][imix]+= accumMixture.get(vu.get(id))[imix];
		for (int  v=0; v<vectLen ;v++) {
		    globalVar[baseId+imix][v] += accumVar.get(vu.get(id))[imix][v];
		    globalMean[baseId+imix][v] += accumMean.get(vu.get(id))[imix][v];
		}
	    }
	}
	return true;
    }

    public void save (Saver saver) throws java.io.IOException {
	saver.save(globalMixture);
	saver.save("meanfn",globalMean,globalMixture[0].length);
	saver.save("varfn",globalVar,globalMixture[0].length);
	saver.save(globalMixture,globalMean,globalVar);

    }
}


			
