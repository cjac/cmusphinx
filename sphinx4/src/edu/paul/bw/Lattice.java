
package edu.paul.bw;
import java.io.InputStreamReader;
import java.io.FileReader;
import java.util.zip.GZIPInputStream;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.io.IOException;
import java.io.File;
import java.io.FileWriter;
import java.io.BufferedReader;
import java.util.HashMap;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Arrays;
import java.util.ArrayList;
import edu.cmu.sphinx.linguist.dictionary.Word;
import edu.cmu.sphinx.linguist.dictionary.Dictionary;
import edu.cmu.sphinx.linguist.WordSequence;
import edu.cmu.sphinx.linguist.language.ngram.LanguageModel;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.linguist.acoustic.LeftRightContext;
import edu.cmu.sphinx.linguist.acoustic.Unit;
import edu.cmu.sphinx.linguist.acoustic.AcousticModel;
import edu.cmu.sphinx.linguist.acoustic.UnitManager;
import edu.cmu.sphinx.linguist.acoustic.HMMPosition;
import edu.cmu.sphinx.linguist.acoustic.HMM;
import edu.cmu.sphinx.linguist.acoustic.HMMState;
import edu.cmu.sphinx.linguist.acoustic.HMMStateArc;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.util.props.Configurable;
import java.util.logging.Logger;
import java.util.logging.Level;

public class Lattice  implements Configurable{
    private class Couple {
	Couple(Unit u, int duree) {
	    this.u=u;
	    this.duree=duree;
	}
	Unit u;
	int duree;
    }
    public enum Mode {normal,fbArc,phone,phoneLinear}
    private Logger logger;
    String name;
    private LanguageModel lm;
    private AcousticModel am;
    private LogMath logMath;
    private UnitManager um;
    private Dictionary dictionary;
    private float wip;
    private float lw;
    private float insertionFiller;
    private boolean allocated=false;
    private boolean rescoreArc =true;
    private Mode mode=Mode.normal;
    private boolean addAlignement;
    private boolean dump=false;
    private boolean linear=false;
    public void register(String name, Registry registry)
            throws PropertyException {
	this.name = name;
	registry.register("unitManager", PropertyType.COMPONENT);
	registry.register("acousticModel", PropertyType.COMPONENT);
	registry.register("languageModel", PropertyType.COMPONENT);
	registry.register("logMath", PropertyType.COMPONENT);
	registry.register("wip",PropertyType.FLOAT);
	registry.register("lw",PropertyType.FLOAT);
	registry.register("insertionFiller",PropertyType.FLOAT);
	registry.register("dictionary",PropertyType.COMPONENT);
	registry.register("rescoreArc",PropertyType.BOOLEAN);
	registry.register("dump",PropertyType.BOOLEAN);
	registry.register("addAlignement",PropertyType.BOOLEAN);
	registry.register("mode",PropertyType.STRING);

    }
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
	logMath = (LogMath) ps.getComponent("logMath", LogMath.class);
	am = (AcousticModel) ps.getComponent("acousticModel", AcousticModel.class);
	lm = (LanguageModel) ps.getComponent("languageModel", LanguageModel.class);
        dictionary = (Dictionary) ps.getComponent("dictionary",
						  Dictionary.class);
	um =(UnitManager) ps.getComponent("unitManager", UnitManager.class);
	wip= logMath.linearToLog(ps.getFloat("wip",0.7f));
	lw= logMath.linearToLog(ps.getFloat("lw",10f));
	insertionFiller=logMath.linearToLog( ps.getFloat("insertionFiller",0.2f));
	rescoreArc=ps.getBoolean("rescoreArc",rescoreArc);
	dump=ps.getBoolean("dump",false);
	addAlignement=ps.getBoolean("addAlignement",true);
	mode=Enum.valueOf(Mode.class,ps.getString("mode","normal"));
	if (mode==Mode.phoneLinear) linear=true; else linear=false;
    }
    
    public String getName() {
        return name;
    }
    
    public void allocate() throws  java.io.IOException {
	if (!allocated)  {
	    dictionary.allocate();
	    am.allocate();
	    lm.allocate();
	    allocated=true;
	}
	
    }
    private HashMap<Integer,SentenceHMMState> nodes=new HashMap<Integer,SentenceHMMState>();
    int countPho=0;
    public ArrayList<Couple> loadTimeStamp (File file) throws IOException {
	countPho=0;
	    logger.info("Loading from " + file+ " mode " +mode );
	    BufferedReader in = new BufferedReader(new InputStreamReader(new GZIPInputStream(new java.io.FileInputStream( file))));
	    Pattern blanc=Pattern.compile("\\s+");
	    String line;
	    line=in.readLine();
	    String [] l= blanc.split(line);
	    if (!l[0].equals("initialNode:"))
		throw new Error("initialNode:"+ line);
	    int keyInit=SentenceHMMState.getKey(0,Integer.parseInt(l[1]));
	    line=in.readLine();
	    l= blanc.split(line);
	    if (!l[0].equals("terminalNode:"))
		throw new Error("terminalNode:"+ line);
	    int keyFinal=SentenceHMMState.getKey(0,Integer.parseInt(l[1]));
	    line=in.readLine();
	    l= blanc.split(line);
	    if (!l[0].equals("logBase:"))
		throw new Error("logBase:"+ line);
	    ArrayList <Couple> timeStamp = new ArrayList<Couple> (1000);
	    while ((line=in.readLine())!=null){
		l=blanc.split(line);
		if (l[0].equals("node:"))
		    doNode(l,timeStamp,1);
	    }
	    in.close();
	    if (logger.isLoggable(Level.FINE)){
		Unit u=timeStamp.get(0).u;
		System.err.print(u+": 0");
		for (int i=1;i<timeStamp.size() ;i++){
		    if (u.equals(timeStamp.get(i).u))
			System.err.print(" "+i);
		    else {System.err.println();
			u=timeStamp.get(i).u;
			System.err.print(u+": "+ i);
		    }
		}
		System.err.println();
	    }
	    logger.info("coun:"+ countPho);
	    return timeStamp;
	    
	    
    }
	
    
    
    public SentenceHMMState load(File file) {
	return load(file,null);
    }
    public SentenceHMMState load(File file,File stamp){
	return load(file,stamp,1);
    }
    
    public SentenceHMMState load(File file,File stamp,int base) {
	try {
	    logger.info("Loading from " + file+ " mode " +mode+ " base" +base);
	    BufferedReader in = new BufferedReader(new InputStreamReader(new GZIPInputStream(new java.io.FileInputStream( file))));
	    Pattern blanc=Pattern.compile("\\s+");
	    String line;
	    line=in.readLine();
	    String [] l= blanc.split(line);
	    if (!l[0].equals("initialNode:"))
		throw new Error("initialNode:"+ line);
	    int keyInit=SentenceHMMState.getKey(0,Integer.parseInt(l[1])*base);
	    line=in.readLine();
	    l= blanc.split(line);
	    if (!l[0].equals("terminalNode:"))
		throw new Error("terminalNode:"+ line);
	    int keyFinal=SentenceHMMState.getKey(0,Integer.parseInt(l[1])*base);
	    if (Integer.parseInt(l[1])!=0 && base!=1)
		{logger.warning(line + "je sais pas faire base negative et initial non nulle"+ base + file);
		return null;
		}
	    line=in.readLine();
	    l= blanc.split(line);
	    if (!l[0].equals("logBase:"))
		throw new Error("logBase:"+ line);
	    
	    while ((line=in.readLine())!=null){
		l=blanc.split(line);
		if (l[0].equals("node:"))
		    doNode(l,null,base);
		else
		    doEdge(l,base);
	    }
	    in.close();
	    nodes.get(keyFinal).setFinalState(true);

	    if (stamp==null && base>0 ) rescoreArc(); // equilibrage entre monde et sentence 10 fois la meme sequence c'est la meme	  
	    if (stamp!=null) {
		if (mode!=Mode.phone&& mode!= Mode.phoneLinear){
		    logger.severe("stamp avec mode:" + mode);
		    return null;
		}
		SentenceHMMState debut;
		if (addAlignement && (debut=load(stamp,null, -1)) !=null)
		    { //on raccorde

		      SentenceHMMState lfin = nodes.get(SentenceHMMState.getKey(0,Integer.MIN_VALUE));
		      logger.fine("on raccorde "+debut+ ": " +lfin);
		      SearchStateArc  arc= debut.getSuccessors()[0];
		      SentenceHMMState fphone=(SentenceHMMState) arc.getToState().getSuccessors()[0].getToState();
		      SentenceHMMStateArc a =new SentenceHMMStateArc(nodes.get(keyInit),fphone,arc.getProbability());
		      nodes.get(keyInit).connect(a);
		      fphone.connectPred(a);
		      arc=lfin.getPredecessors()[0];
		      lfin= (SentenceHMMState)arc.getFromState();
		      a=new SentenceHMMStateArc(lfin, nodes.get(keyFinal),arc.getProbability());
		      lfin.connect(a);
		      nodes.get(keyFinal).connectPred(a);
	  }
		tag(nodes.values(),loadTimeStamp(stamp));
	    }
	    SentenceHMMState resultat=resultat=nodes.get(keyInit);
	    if (base>0) {
		if (dump) dumpAISee("aisee"+file.getName()+".dot");
		nodes.clear();}
	    logger.fine("res:"+resultat);
	    return resultat;
	}
	catch (IOException e) {
	    nodes.clear();
	    e.printStackTrace();
	    return null;
	}
    }

    public    int getCountPho(){
	return countPho;
    }
    private void rescoreArc() {
	if(!rescoreArc) return;
	for (SentenceHMMState state : nodes.values()) 
	    if(! state.isEmitting() && state instanceof HMMStateState ) {
		SearchStateArc []arcs= state.getSuccessors();
		if (arcs.length==0) continue;
		float incr=logMath.linearToLog(1.0/arcs.length);
		for (SearchStateArc a:arcs)
		    ((SentenceHMMStateArc)a).increment(incr);
	    }
    }

    private void doEdge(String [] l,int base) {
	if (!(l.length ==3 && l[0].equals("edge:")))
	    throw new Error(" pas edge" + Arrays.toString(l));
        int fin=Integer.parseInt(l[2])*base;
	if (fin==0 && base !=1) fin=Integer.MIN_VALUE;
	SentenceHMMState fromNode = nodes.get(SentenceHMMState.getKey(0,Integer.parseInt(l[1])*base));
	SentenceHMMState toNode =nodes.get(SentenceHMMState.getKey(0,fin));
	if (fromNode instanceof HMMStateState) {
	    fromNode= nodes.get(SentenceHMMState.getKey(((HMMStateState)fromNode).getHMMState().getHMM().getOrder(),Integer.parseInt(l[1])*base));
	}
	float logProb=LogMath.getLogOne();
	if (toNode instanceof WordState) {
	    Word w = ((WordState ) toNode).getWord();
	    if (w.isFiller() ) logProb=insertionFiller;
	    else logProb= lm.getProbability (WordSequence.getWordSequence(w));
	    logProb += wip/lw;
	}
	if (logger.isLoggable(Level.FINER))
	    logger.finer("arc:"+fromNode+ " to:"+toNode +" prob:"+ logMath.logToLinear(logProb));
	if (linear) logProb=(float)logMath.logToLinear(logProb);//je sais il y a une perte .....
	SentenceHMMStateArc a=  new SentenceHMMStateArc(fromNode,toNode,logProb);
	fromNode.connect(a);
	toNode.connectPred(a);
    }



    Pattern p=Pattern.compile("HMM\\((\\S+)\\[(\\S+),(\\S+)]\\):(\\S)"); //bizarre bizarre 2 versions mais bof...
    
    private void doNode(String []l, ArrayList<Couple> timeStamp,int baseId) {
	int nodeId=Integer.parseInt(l[1])*baseId;
	int startFrame=Integer.parseInt(l[2])-1;
	// in decoder frame decale de 1 the first token emit has frameNumber of one
	//if (startFrame==1) startFrame=0; //pb avec <s> qui est en 0 et non en -1
	// startFrame is false for non-emetting
	int endFrame=Integer.parseInt(l[3])-1;
	if (l.length<=5&& timeStamp==null) {
	    //that, is a word
	    if (startFrame > endFrame) {
		startFrame=endFrame;
	    }
	    Word w= dictionary.getWord(l[4]);
	    if (nodeId ==0 && baseId<0){ nodeId=Integer.MIN_VALUE;
	    logger.fine(l[4] + "passe au mini");}
	    if (!nodes.containsKey(SentenceHMMState.getKey(0,nodeId)))
		nodes.put(SentenceHMMState.getKey(0,nodeId),new WordState(w,nodeId,startFrame,endFrame));
		else
		logger.info(l[4]+ " " + nodeId + "deja vu");
	    return;
	}
	if (l.length<=5&& timeStamp!=null) return;
	//that is a HMM
	if (!"HMM".equals(l[4]))
	    throw new Error("not hmm"+ l[0] +" "+baseId+l[1]+" "+l[4]);
	Matcher ml=p.matcher(l[6]);
	Boolean isFiller=Boolean.parseBoolean(l[5]);
	if (!ml.find())
	    throw new Error("not hmm:"+l[6] + "! p:"+p+"!");
	HMMPosition position= HMMPosition.lookup(ml.group(4));
	Unit base=um.getUnit(ml.group(1),isFiller);
	if (timeStamp!=null) {
	    Couple cu= new Couple(base,endFrame-startFrame+1);
	    countPho++;
	    while (timeStamp.size() <endFrame+1) timeStamp.add(null);
	    for (int i =startFrame; i<=endFrame;i++)
		timeStamp.set(i,cu);
	    return;
	}
	Unit cd;
	if ("-".equals(ml.group(2)))
	    cd=base;
	else {
	    Unit cog,cod;
	    if ("SIL".equals(ml.group(2)))
		cog= um.SILENCE;
	    else
		cog=um.getUnit(ml.group(2));
	    if ("SIL".equals(ml.group(3)))
		cod= um.SILENCE;
	    else
		cod=um.getUnit(ml.group(3));
	    Unit []ag,ad;
	    ag=new Unit[1];
	    ad=new Unit[1];
	    ag[0]=cog;
	    ad[0]=cod;
	    cd= um.getUnit(ml.group(1),isFiller,LeftRightContext.get(ag,ad));
	}
	HMM hmm= am.lookupNearestHMM(cd,position,true);
	if (hmm==null) {
	    hmm=am.lookupNearestHMM(cd,position,false);
	    if (hmm==null)
	  throw   new Error( "ratee " + cd +" p:"+ position);
	    else 
		logger.warning("pas vu le phone :"  + cd +" p:"+ position+
			       " pris:"+hmm);
	}
	int nState=hmm.getOrder()+1;
	//c'est le bordel between getOrder et numState getOrder= number of emittings states 
	//System.err.println("-------------- "+hmm+ "  order:"+nState + " mod" +
	//		   (nState%2!=0)	   );
	if (nState%2 != 0) 
	    throw   new Error( "ratee  order "+nState);
	HMMStateState[] lesEtats= new HMMStateState[nState];
	switch  (mode) {
	case normal: lesEtats[0]= new  HMMStateState(hmm.getState(0),nodeId,startFrame,endFrame);break;
	case fbArc: lesEtats[0]= new StateZero(hmm.getState(0),nodeId,startFrame,endFrame);break;
	case phone: lesEtats[0]= new StatePhone(hmm.getState(0),nodeId,startFrame,endFrame);break;
	case phoneLinear :lesEtats[0]=	new StatePhoneDouble(hmm.getState(0),nodeId,startFrame,endFrame); break;
	}
	if (logger.isLoggable(Level.FINEST))
	    logger.finest("e:" +lesEtats[0] + " classe:"+ lesEtats[0].getClass()); 
	for (int i=1 ; i<nState; i++) {
	    lesEtats[i]= new HMMStateState(hmm.getState(i),nodeId,startFrame,endFrame);
	    lesEtats[i].direct=lesEtats[0];
	}
	lesEtats[0].direct=lesEtats[nState-1];
	nodes.put(lesEtats[0].getKey(),lesEtats[0]);
	nodes.put(lesEtats[nState-1].getKey(),lesEtats[nState-1]);
	for (int i=0 ; i<nState ;i++)
	    for (HMMStateArc arc: hmm.getState(i).getSuccessors()){
		SentenceHMMStateArc a= new SentenceHMMStateArc(lesEtats[i],lesEtats[arc.getHMMState().getState()],
							       (linear)? (float) logMath.logToLinear(arc.getLogProbability()) :
							       arc.getLogProbability() );
		logger.finest(a.toString());
		lesEtats[i].connect(a);
		lesEtats[arc.getHMMState().getState()].connectPred(a);
	    }
    }
		 
	
    private void tag(Collection <SentenceHMMState> states,ArrayList<Couple> timeStamp) {
	for (SentenceHMMState s: states) {
	    if (! s.isStateZero() ) continue;
	    HMMStateState  ph =(HMMStateState ) s; 
	    Unit u= ph.getHMMState().getHMM().getBaseUnit();
	    // les fillers sont problematique dans cette histoire mais  bon ....
	    HashMap<Couple,Integer> count= new HashMap<Couple,Integer>(3) ;
		// entre unit et couple ce n'est pas la meme chose c'est un choix
		// dans le cas ou un phoneme se repete dans l'alignement ...
	    for (int i=ph.getStartFrame() ; i<= ph.getEndFrame() ; i++) {
		if (i>=timeStamp.size()) {
		    logger.warning ("on deborde --"+u+"---"+ph.getStartFrame() + " "+ph.getEndFrame()+"tt"+ph+" i:"+i);}
		else
		    if (count.containsKey(timeStamp.get(i)))
			count.put(timeStamp.get(i), count.get(timeStamp.get(i))+1);
		    else
			count.put(timeStamp.get(i),1);
	    }
	    float valeur=-1.0f;
	    
	    for (Couple c: count.keySet()){ float x;
		if (c.u.equals(u))
		    x= -1.0f + 2.0f*count.get(c).floatValue()/c.duree;
		else x=-1.0f + count.get(c).floatValue()/c.duree;
		if (u.isFiller() && x<0) x=0.0f;
		if (x> valeur) valeur=x;
	    }
	    ph.setPhoneAcc(valeur);
	    if (logger.isLoggable(Level.FINE)) logger.fine("acc" +s+" st:"+ph+ " u:" +u +" deb:" + ph.getStartFrame() 
							     +" fin:" +      ph.getEndFrame()         + " v:"+ valeur+
							   " "+ ph.getPhoneAcc());


	}
    }
	
    void dumpAISee(String fileName) {
	String title=fileName;
	try {
            System.err.println("Dumping " + title + " to " + fileName);
            FileWriter f = new FileWriter(fileName);
            f.write("graph: {\n");
            f.write("title: \"" + title + "\"\n");
            f.write("display_edge_labels: yes\n");
	    for (SentenceHMMState s: nodes.values()) {
		if (!s.isEmitting() )s.dumpAISee(f);
	    }
	    for (SentenceHMMState s: nodes.values()) {
		if  (!s.isEmitting() ) {
		    for (SearchStateArc  arc: s.getSuccessors()) {
			SentenceHMMState toNode = (SentenceHMMState)arc.getToState();
			if (toNode.isEmitting()) toNode =toNode.getDirect();
			f.write( "edge: { sourcename: \"" + s.getName()
				 + "\" targetname: \"" + toNode.getName()
				 + "\" label: \"" + arc.getProbability()
				 + "\" }\n" );
		    }		
		    
		}
	    }
	    f.write("}\n");
	    f.close();
	}
	catch (IOException e) {
	    e.printStackTrace();
	}
    }
    
}

