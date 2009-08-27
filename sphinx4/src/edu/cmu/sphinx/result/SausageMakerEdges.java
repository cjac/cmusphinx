/*
 * Copyright 1999-2004 Carnegie Mellon University.
 * Portions Copyright 2004 Sun Microsystems, Inc.
 * Portions Copyright 2004 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 *
 * See the fgile "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 *
 * Created on Aug 10, 2004
 */
package edu.cmu.sphinx.result;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Vector;
import java.util.ArrayList ;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import edu.cmu.sphinx.linguist.dictionary.Pronunciation;
import edu.cmu.sphinx.linguist.dictionary.Word;
import edu.cmu.sphinx.linguist.dictionary.Dictionary;
import edu.cmu.sphinx.linguist.language.ngram.LanguageModel;
import java.util.logging.Logger;
import java.util.logging.Level;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;




/**
 * <p>
 * The SausageMaker takes word lattices as input and turns them into sausages 
 * (Confusion Networks) according to Mangu, Brill and Stolcke, "Finding 
 * Consensus in Speech Recognition: word error minimization and other 
 * applications of confusion networks", Computer Speech and Language, 2000.
 * Note that the <code>getBestHypothesis</code> of the ConfidenceResult
 * object returned by the {@link #score(Result) score} method
 * returns the path where all the words have the highest posterior
 * probability within its corresponding time slot.
 * </p>
 *
 *
 * @author pgorniak
 *
 */
public class SausageMakerEdges implements ConfidenceScorer, Configurable {
  private int numero=0;

    private class ContCluster {
	Cluster c=null;
	ContCluster(Cluster c) {
	    this.c=c;
	}
    }
    private class Cluster {
	int id =numero++;
        int idTopo;
	private int beginTime=-100;
	private int endTime=-100;
	private int minTime=-100;
	private int maxTime=-100;
	ArrayList <NodeEdge> l;
	ContCluster cont=null;
	HashMap <Word,Float> proba=null;
	HashSet <ContCluster> suc=null;
	Cluster(ArrayList <NodeEdge>  l) {
	    this.l=l;
	    this.id =id;
	}
	Cluster (NodeEdge n) {
	    l = new ArrayList <NodeEdge> (1);
	    l.add(n);
	    minTime=beginTime=n.getBeginTime();
	    maxTime= endTime =n.getEndTime();
       
	}
	void calculWordPost() {
            proba= new HashMap <Word,Float>();
	    for (NodeEdge n : l) {
		Word  w=n.getWord();
		if (proba.containsKey(w)) 
		    proba.put(w,logMath.addAsLinear(proba.get(w),(float)n.getPosterior()));
		else
		    proba.put(w,(float)n.getPosterior());
	    }
	}
	double post() {
	    double resu=0.0;
	    for (NodeEdge n:l) {
		resu += logMath.logToLinear((float) n.getPosterior());
	    }
	    return resu;
	}
        void add(NodeEdge n) {
	    if (this.proba!=null) {
		Word w =n.getWord();
		
		if (proba.containsKey(w)) 
		    proba.put(w,logMath.addAsLinear((float)n.getPosterior(),(float)this.proba.get(w)));
		else
		    proba.put(w,(float)n.getPosterior());
		
	    }
	    l.add(n);
	    //id =numero++; au debut ce n'est pas la peine
	    if (beginTime< n.getBeginTime()) 
		beginTime=n.getBeginTime();
	    if (endTime >n.getFirstEndTime())
		endTime=n.getFirstEndTime();
	    maxTime= Math.max(maxTime,n.getEndTime());
	    minTime= Math.min(minTime,n.getBeginTime());


 // pour le min normale c'est deja le cas
	}

	void addAll(Cluster c2) {
	    if (c2.proba!=null) {
		for (Word w : c2.proba.keySet()) {
		    if (proba.containsKey(w)) 
			proba.put(w,logMath.addAsLinear(proba.get(w),(float)c2.proba.get(w)));
		    else
			proba.put(w,c2.proba.get(w));
		}
	    }
	    l.addAll(c2.l);
	    id =numero++;
	    if (beginTime< c2.beginTime) 
		beginTime=c2.beginTime;
	    if (endTime >c2.endTime)
		endTime=c2.endTime;
	    maxTime= Math.max(maxTime,c2.maxTime);
	    if (minTime >c2.minTime) 
		System.err.println( "fusion inv "+ this + "," +  c2);
	    if (this.suc!=null && c2.suc !=null) 
		this.suc.addAll(c2.suc);
		    
 // pour le min normale c'est deja le cas
	}
	ContCluster getCont() {
	    if (cont==null) cont =new ContCluster(this);
	    return cont;
	}
	String lesSucs() {
	    if (suc==null) return "";
	    StringBuffer sb=new StringBuffer();
	    for (ContCluster c: suc) {
		sb.append( " "+ c.c.id);
	    }
	    return "\n sucs:" +sb.toString() + "\n";
	}
	public String toString() {
	    return "id:"+id+" siz:"+ l.size()+" bt:"+ getBeginTime() +" et:"+ getEndTime()+
		" s:"+l.size()+ " latesbeg: " + getLatestBeginTime()+  " earlest:" + getEarliestEndTime()+ " " + l.get(0) +" "+ lesSucs();
	}

	public int hashCode() {
	    return id;}
	public boolean  equals (Object o) {
	    if (o instanceof Cluster)
		return (((Cluster)o).id == this.id);
	    else return false;
	}
	int getBeginTime() {
	    if (minTime >=0) return minTime;
	    minTime=Integer.MAX_VALUE;
	    for (NodeEdge n : l) 
		if (n.getBeginTime()< minTime)
		    minTime=n.getBeginTime();
	    return minTime;
	}

	int getLatestBeginTime() {
	    if (beginTime >=0) return beginTime;
	    for (NodeEdge n : l) 
		if (n.getLastBeginTime()> beginTime)
		    beginTime=n.getLastBeginTime();
	    return beginTime;
	}
	int getEndTime() {
	    if (maxTime >=0) return maxTime;
	    for (NodeEdge n :l)
		if (n.getEndTime() >maxTime)
		    maxTime = n.getEndTime();
	    return maxTime;
	}
	int getEarliestEndTime() {
	    if (endTime >=0) return endTime;
	    endTime = Integer.MAX_VALUE;
	    for (NodeEdge n :l)
		if (n.getEndTime() <endTime)
		    endTime = n.getEndTime();
	    return endTime;
	}

    }




private class ClusterComparator implements Comparator <Cluster> {
    int [][] ecart;


    /**
     * Compares to clusters according to their topological relationship. Relies
     * on strong assumptions about the possible constituents of clusters which
     * will only be valid during the sausage creation process.
     * 
     * @param o1 the first cluster (must be a List)
     * @param o2 the second cluster (must be a List)
     */
    private ClusterComparator(int [][] ecart ) {
	this.ecart=ecart;}



    public int compare( Cluster cluster1 , Cluster cluster2) {
	//      Cluster cluster1 = (Cluster) o1;
	//         Cluster cluster2 = (Cluster) o2
	return  ecart[cluster1.idTopo][cluster2.idTopo];
    
    }
}

  
    class CoupleInt {
	private int i1,i2;
	CoupleInt (int i1, int i2) {
	    this.i1=i1;
            this.i2=i2;
	}
	public int hashCode() {
	    return i1 + 32763*i2;
	}
	public	boolean equals (Object o) {
	    CoupleInt c=(CoupleInt) o; //a netoyer
	    return i1==c.i1 && i2==c.i2;
	}
	public String toString(){
	    return "i1:"+i1+" i2:"+i2;
	}
    }
    HashMap <CoupleInt,Float> distance = new HashMap <CoupleInt,Float>();
    HashSet <CoupleInt> interdit = new HashSet <CoupleInt>();
    
    class Couple {
	private Object  i1,i2;
	Couple (Object i1, Object i2) {
	    this.i1=i1;
            this.i2=i2;
	}
	public int hashCode() {
	    return i1.hashCode() + 37*i2.hashCode();
	}
	public boolean equals (Object o) {
	    if (o instanceof  Couple) {
		Couple c=(Couple) o;
		return i1.equals( c.i1)  && i2.equals(c.i2);
	    }
	    return false;
	}

    }

    static LinkedHashMap <Couple,Float > distanceEdition = new LinkedHashMap <Couple,Float>() {
	    protected boolean  removeEldestEntry(Map.Entry eldest) {
		return size() > 10000;
	    }
	}; 

    public final static String PROP_WORD_INSERTION_PROBABILITY = "wordInsertionProbability";

    /**
     * The default value for PROP_WORD_INSERTION_PROBABILITY
     */
    public final static float PROP_WORD_INSERTION_PROBABILITY_DEFAULT = 1.0f;


 /**
     * Sphinx property that defines the language weight for the search
     */
    public final static String PROP_LANGUAGE_WEIGHT = "languageWeight";

    /**
     * The default value for the PROP_LANGUAGE_WEIGHT property
     */
    public final static float PROP_LANGUAGE_WEIGHT_DEFAULT = 10f;
  /**
     * Sphinx property that defines the language weight for the search
     */
    public final static String PROP_EDGE = "edge";

   public final static boolean  PROP_EDGE_DEFAULT =true;
  /**
     * Sphinx property that defines the language weight for the search
     */
    public final static String PROP_POURCENT_PRUNE = "prune";




    /**
     * The default value for the PROP_LANGUAGE_WEIGHT property
     */
    public final static float PROP_POURCENT_PRUNE_DEFAULT = 1.0f;
    /**
     * Sphinx property that defines the logMath component.
     */
    public final static String PROP_LOG_MATH = "logMath";
   /**
     * The Sphinx Property specifying the dictionary to use
     */
    public final static String PROP_DICTIONARY = "dictionary";
    
   /**
     * A sphinx property for the language model to be used by this grammar
     */
    public final static String PROP_LANGUAGE_MODEL = "languageModel";

 

    private Logger logger;
    private String name;
    private float languageWeight;
    private Dictionary dictionary;
    
    protected Lattice lattice;
    private  LogMath logMath;
    private LanguageModel languageModel;
    private boolean allocLm=true;
    private float prune=10;
    private boolean edgeMode=true;
    private float wip=0;
    private float seuilabs=5.0e-4f;
    private double seuilMot=0.0001;
    private float scale;
    private boolean dumpSausage;
    private boolean dumpLattice;
    private boolean keepAll=false;
/**
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
        throws PropertyException {
        this.name = name;  registry.register(PROP_LOG_MATH, PropertyType.COMPONENT);
        registry.register(PROP_DICTIONARY, PropertyType.COMPONENT);
        registry.register(PROP_LANGUAGE_WEIGHT, PropertyType.FLOAT);
        registry.register(PROP_WORD_INSERTION_PROBABILITY, PropertyType.FLOAT);
	registry.register(PROP_LANGUAGE_MODEL, PropertyType.COMPONENT);
	registry.register(PROP_POURCENT_PRUNE, PropertyType.FLOAT);
	registry.register("seuilAbsolu", PropertyType.FLOAT);
	registry.register("seuilMot", PropertyType.DOUBLE);
	registry.register("scale", PropertyType.FLOAT);
        registry.register("dumpSausage", PropertyType.BOOLEAN);
        registry.register("dumpLattice", PropertyType.BOOLEAN);
        registry.register(PROP_EDGE, PropertyType.BOOLEAN);
	registry.register("keepAll", PropertyType.BOOLEAN);
    }
    
    
    /**
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
	
	logger = ps.getLogger();
        logMath = (LogMath) ps.getComponent(PROP_LOG_MATH, LogMath.class);
        dictionary = (Dictionary) ps.getComponent(PROP_DICTIONARY,
						  Dictionary.class);
	
	languageWeight = ps.getFloat(PROP_LANGUAGE_WEIGHT,
				     PROP_LANGUAGE_WEIGHT_DEFAULT);
	prune = ps.getFloat(PROP_POURCENT_PRUNE,PROP_POURCENT_PRUNE_DEFAULT);
        seuilabs=ps.getFloat("seuilAbsolu",5.0e-4f);
        seuilMot=ps.getDouble("seuilMot",seuilMot);
        scale=ps.getFloat("scale",languageWeight);
	edgeMode = ps.getBoolean(PROP_EDGE,PROP_EDGE_DEFAULT);
	dumpSausage = ps.getBoolean("dumpSausage",false);
	dumpLattice = ps.getBoolean("dumpLattice",false);
	keepAll = ps.getBoolean("keepAll",false);
        wip = ps.getFloat(PROP_WORD_INSERTION_PROBABILITY,
			   PROP_WORD_INSERTION_PROBABILITY_DEFAULT);
 	try {
	    languageModel = (LanguageModel) ps.getComponent(PROP_LANGUAGE_MODEL,
							LanguageModel.class);
	}
        catch (PropertyException e) {
	    logger.info("pas de lm \n");
	    languageModel =null;
	}


 }

    /**
     * @see edu.cmu.sphinx.util.props.Configurable#getName()
     */
    public String getName() {
        return name;
    }

    /**
     * Construct an empty sausage maker
     *
     */
    public SausageMakerEdges() {
    }
    
    /**
     * Construct a sausage maker
     *
     * @param l the lattice to construct a sausage from
     */
     
    public SausageMakerEdges(Lattice l) {
        lattice = l;
        logMath= lattice.getLogMath();
	maxEcart =100;
    }

    private int maxEcart=0;
    private int getMaxEcart() {
	    return maxEcart;
    }
    
    private void setMaxEcart(int m) {
	this.maxEcart=m;}

    /**
     * Perform the inter word clustering stage of the algorithm
     * 
     * @param clusters the current cluster set
     */
    protected void interWordCluster(List <Cluster> clusters) {
	 hit=0;nohit=0;
	 //
	 distance.clear();
	 toMerged.clear();
	 for (Cluster c : clusters)
	     c.calculWordPost();
         countDump=clusters.size();
	 if (logger.isLoggable(Level.FINEST)) {
	     System.err.println("au milieu -------------------------------------------------------------------- " +countDump);
	     printClusters(clusters);}
	 
	 while(interWordClusterStep(clusters,false));
         countDump=clusters.size();
	 System.err.println("au milieu " +countDump);
	 faireSuc( clusters);
	 if (logger.isLoggable(Level.FINER)) printClusters(clusters);
	 while(interWordClusterStep(clusters,true));

    }

    /**
     * Returns true if the two given clusters has time overlaps.
     * Given clusters A and B, they overlap if and only if:
     * the latest begin time of any node in B is before 
     * the earliest end time of any node in A,
     * and the latest begin time of any node in A is before the
     * earliest end time of any node in B.
     *
     * @param cluster1 the first cluster to examine
     * @param cluster2 the second cluster to examine
     *
     * @return true if the clusters has overlap, false if they don't
     */
  
    private boolean hasOverlap(Cluster cluster1, Cluster cluster2) {
        int latestBeginTime1 = cluster1.getLatestBeginTime();
        int earliestEndTime1 = cluster1.getEarliestEndTime();
        int latestBeginTime2 = cluster2.getLatestBeginTime();
        int earliestEndTime2 = cluster2.getEarliestEndTime();

        if (latestBeginTime1 < earliestEndTime2 &&
            latestBeginTime2 < earliestEndTime1) {
            return true;
        } 
// 	else
// 	    if (latestBeginTime2 < earliestEndTime1 &&  // j'aimerais comprendre ce test ....... je croyais que && commute
//                    latestBeginTime1 < earliestEndTime2) {
//             return true;
//         } else {
//             return false;
//         }
	return false;
    }
    
    private float ecartCluster(Cluster cluster1, Cluster cluster2) {
	float res;
       int latestBeginTime1 = cluster1.getLatestBeginTime();
        int earliestEndTime1 = cluster1.getEarliestEndTime();
        int latestBeginTime2 = cluster2.getLatestBeginTime();
        int earliestEndTime2 = cluster2.getEarliestEndTime();
	if  (latestBeginTime1 < earliestEndTime2)
	    res= 1.0f -((float)latestBeginTime2- earliestEndTime1)/(3.0f*getMaxEcart());
	else 
	    res= 1.0f -((float)latestBeginTime1- earliestEndTime2)/(3.0f*getMaxEcart());
	if (res>1f) logger.warning("overlapped cluster in step 2 c1:"+cluster1+" c2:"+cluster2);
	if (res<=0f) return logMath.linearToLog(0.000001); else return logMath.linearToLog(res);
    }

    private void faireSuc(List <Cluster> clusters) {
	logger.fine(String.format("debut de faire suc %d",interdit.size()));
   ListIterator <Cluster> i = clusters.listIterator();
	int ecart=1;
	boolean pasTrouve;
        while (i.hasNext()) {
            Cluster ci = i.next();
	    if (ci.suc==null) ci.suc=new HashSet <ContCluster>(20);
            if (!i.hasNext()) {
                break;
            }
            ListIterator <Cluster> j = clusters.listIterator(i.nextIndex());
            while (j.hasNext()) {
                Cluster cj = j.next(); 
                if (ci.getEndTime()+ ecart  < cj.getBeginTime()) break; 
		CoupleInt couple= new CoupleInt(ci.id,cj.id);
		if (! interdit.contains(couple)) continue;
		pasTrouve =true; 
		Iterator <NodeEdge>  ic = ci.l.iterator();
		while (ic.hasNext() && pasTrouve) {
		    NodeEdge n1 = ic.next();
		    Iterator <NodeEdge> ic2 = cj.l.iterator();
		    while (ic2.hasNext()&& pasTrouve) {
			NodeEdge n2 = ic2.next();
			if (n1.getEndTime()+getMaxEcart() <n2.getBeginTime()|| n1.isAncestorOf(n2)) {
			    ci.suc.add(cj.getCont());pasTrouve=false;
				       logger.finer("suc ( "+ci+","+cj  + "  ," +  n1.isAncestorOf(n2)
						    + " n1:" +n1 +" n2:"+n2);
				       } else if (n2.getEndTime()+getMaxEcart() <n1.getBeginTime()||n2.isAncestorOf(n1)) {
				if (cj.suc==null) cj.suc=new HashSet <ContCluster> (20);
				cj.suc.add(ci.getCont());pasTrouve=false;
				logger.finer("suc( "+cj+","+ci  + "  ," +  n2.isAncestorOf(n1)
					     + " n1:" +n2 +" n2:"+n1);
			    }
			}
		}
		if (pasTrouve) logger.warning("pas trouve l'ordre pour" +ci+ "et :" + cj);

	    }
	}
	closure (i,clusters);
    }
    void closure (ListIterator <Cluster> fini, List <Cluster> clusters) {
	int passe=5;
	boolean modifier=true;
	while (fini.hasPrevious() && passe-->0 && modifier) {
	    modifier=false;
	    ListIterator <Cluster> j = clusters.listIterator(fini.previousIndex());
	    // fini.previous();
	    int card=0;
	    while(j.hasPrevious()) {
		Cluster ci=j.previous();
	        ContCluster [] temp= ci.suc.toArray( new ContCluster[0]);
		for (ContCluster fils: temp) {
		    for (ContCluster contPetitFils :fils.c.suc) {
			Cluster cj=contPetitFils.c;
			if (ci.getEndTime()+ getMaxEcart() +3 > cj.getBeginTime())  // je me suis fait avoir une fois alors .........on n'est pas a 3 pres. 
			    modifier |= ci.suc.add(cj.getCont());
		    }
		}
		card+=ci.suc.size();
	    }
	    System.err.println("une passe de closure " + card);
	}
    }

    /**
     * Perform one inter word clustering step of the algorithm
     * 
     * @param clusters the current cluster set
     */
    protected boolean interWordClusterStep(List <Cluster>  clusters,boolean ncont) {
        Cluster toBeMerged1 = null;
        Cluster toBeMerged2 = null;
        float maxSim = Float.NEGATIVE_INFINITY;
        ListIterator <Cluster> i = clusters.listIterator();
	int ecart=(ncont) ? getMaxEcart():1;
	if (clusters.size() <=countDump|| logger.isLoggable(Level.FINE)) {
	    countDump=clusters.size()-100;
	    logger.info("inter :" + clusters.size() + 
			       " ancestor " + ancestors.size()+
			       " distance:"+ distance.size()+ 
			       " int:"+interdit.size()+
			       " distanceEd:"+  distanceEdition.size()
			       +" hit:" +hit + " nohit:"+nohit + "ecart: "+ecart+ "ncont :" +ncont);
	    
	}
        while (i.hasNext()) {
            Cluster c1 = i.next();
            if (!i.hasNext()) {
                break;
            }
            ListIterator <Cluster> j = clusters.listIterator(i.nextIndex());
            while (j.hasNext()) {
                Cluster c2 = j.next(); 
                if (c1.getEndTime()+ ecart  < c2.getBeginTime()) break;
		if (ncont && (c1.suc.contains(c2.getCont()) || c2.suc.contains(c1.getCont()))) continue;
                float sim = interClusterDistance(c1,c2);
		if (ncont) sim+=ecartCluster(c1,c2);
                if (sim > maxSim &&(ncont|| hasOverlap(c1,c2))) {
                    maxSim = sim;
		    if ( toBeMerged2!=null 
			 && toBeMerged2.getEndTime()+100<c1.getBeginTime())
			{toMerged.add(toBeMerged1);toMerged.add(toBeMerged2);}
                    toBeMerged1 = c1;
                    toBeMerged2 = c2;
                }
            }
        }
        if (toBeMerged1 != null) {
	    toMerged.add(toBeMerged1);toMerged.add(toBeMerged2);
	    for (int clus=0; clus<toMerged.size();){
		toBeMerged1=toMerged.get(clus++);
		toBeMerged2=toMerged.get(clus++);
                if (logger.isLoggable(Level.FINE)){
	     	    logger.fine("merge : " +toBeMerged1+ "----avec--  " +toBeMerged2);
		    //printCluster(toBeMerged1);
		    // printCluster(toBeMerged2);
		}
		
		
		clusters.remove(toBeMerged2);
		toBeMerged1.addAll(toBeMerged2);
		if (logger.isLoggable(Level.FINE)){
		    logger.fine("resul : " +toBeMerged1);
		}
	    }
	    logger.fine(" fin d'une passe" );
            toMerged.clear();
            return true;
        }
        return false;
    }
    
    /**
     * Find the string edit distance between to lists of objects.
     * Objects are compared using .equals()
     * TODO: could be moved to a general utility class
     * 
     * @param p1 the first list 
     * @param p2 the second list
     * @return the string edit distance between the two lists
     */
    protected int stringEditDistance(List p1, List p2) {
        if (p1.size() == 0) {
            return p2.size();
        }
        if (p2.size() == 0) {
            return p1.size();
        }
        int [][] distances = new int[p1.size()][p2.size()];
        distances[0][0] = 0;
        for (int i=0;i<p1.size();i++) {
            distances[i][0] = i;
        }
        for (int j=0;j<p2.size();j++) {
            distances[0][j] = j;
        }
        for (int i=1;i<p1.size();i++) {
            for (int j=1;j<p2.size();j++) {
                int min = Math.min(distances[i-1][j-1]
				   + (p1.get(i).equals(p2.get(j)) ? 0 : 1),
                                   distances[i-1][j] + 1);
                min = Math.min(min,distances[i][j-1] + 1);
            }            
        }
        return distances[p1.size()-1][p2.size()-1];
    }
    
    /**
     * Compute the phonetic similarity of two lattice nodes, based on the string
     * edit distance between their most likely pronunciations.
     * TODO: maybe move to Node.java?
     * 
     * @param n1 the first node
     * @param n2 the second node
     * @return the phonetic similarity, between 0 and 1
     */
    protected float computePhoneticSimilarity(Word w1, Word w2) {
        Pronunciation p1 = w1.getMostLikelyPronunciation();
        Pronunciation p2 = w2.getMostLikelyPronunciation();
        Couple c ;
        if (w1.hashCode() <w2.hashCode()) c=new Couple(w1,w2);
	else c=new Couple(w2,w1);
        if  (distanceEdition.containsKey(c))
	    return distanceEdition.get(c);
        float sim = stringEditDistance(Arrays.asList(p1.getUnits()),
					Arrays.asList(p2.getUnits()));
        sim /= (double)(p1.getUnits().length + p2.getUnits().length);
        distanceEdition.put(c,sim);
        return 1-sim;
    }
    
    /**
     * Return the total probability mass of the subcluster of nodes of the given
     * cluster that all have the given word as their word.
     * 
     * @param cluster the cluster to subcluster from
     * @param word the word to subcluster by
     * @return the log probability mass of the subcluster formed by the word
     */
    protected float wordSubClusterProbability(Cluster cluster, String word) {
        return clusterProbability(makeWordSubCluster(cluster,word));
    }
    
    /**
     * Calculate the sum of posteriors in this cluster.
     * 
     * @param cluster the cluster to sum over
     * @return the probability sum
     */
    protected float clusterProbability(List <NodeEdge> cluster) {
        float p = LogMath.getLogZero();
        
        for (NodeEdge n : cluster)
            p = lattice.getLogMath().addAsLinear(p,(float)(n.getPosterior()));
       
        return p;
    }
    
    /**
     * Form a subcluster by extracting all nodes corresponding to a given word.
     * 
     * @param cluster the parent cluster
     * @param word the word to cluster by
     * @return the subcluster.
     */
    protected List makeWordSubCluster(Cluster cluster, String word) {
        ArrayList <NodeEdge> sub = new ArrayList <NodeEdge>();
	for ( NodeEdge n : cluster.l) 
	    if (n.getWord().getSpelling().equals(word)) 
                sub.add(n);
            
        
        return sub;
    }
    
    /**
     * Calculate the distance between two clusters
     * 
     * @param cluster1 the first cluster
     * @param cluster2 the second cluster
     * @return the inter cluster similarity, or Float.NEGATIVE_INFINITY if 
     *         these clusters should never be clustered together.
     */
    protected float interClusterDistance(Cluster cluster1, Cluster  cluster2) {
	CoupleInt ci = new CoupleInt(cluster1.id,cluster2.id);
	if (interdit.contains(ci)){hit++; return Float.NEGATIVE_INFINITY;}
	if (distance.containsKey(ci)){
	    hit++;
	    return distance.get(ci);}
	nohit++;
 
        if (areClustersInRelation(cluster1,cluster2)) {
	    interdit.add(ci);
            return Float.NEGATIVE_INFINITY;
        }
        float totalSim = LogMath.getLogZero();
        float wordPairCount = (float)0.0;
        Iterator <Word>  i1 = cluster1.proba.keySet().iterator();
        while (i1.hasNext()) {
            Word node1 = i1.next();
            Iterator <Word> i2 = cluster2.proba.keySet().iterator();
            while (i2.hasNext()) {
                Word node2 = i2.next();
                float sim = (float)computePhoneticSimilarity(node1,node2);
                sim = lattice.getLogMath().linearToLog(sim);
                sim += cluster1.proba.get(node1);
                sim += cluster2.proba.get(node2);
                totalSim = lattice.getLogMath().addAsLinear(totalSim,sim);
                wordPairCount++;
            }
        }      
	float res=            totalSim - lattice.getLogMath().linearToLog(wordPairCount);

	distance.put(ci,res);     
        return res;
    }
    HashMap <Couple,Boolean> ancestors=new HashMap <Couple,Boolean> ();
    protected boolean hasAncestralRelationship(NodeEdge n1, NodeEdge n2) {
	Couple ci;
	boolean b=false;
	if (!modeDebug) {
        // if (n1.getBeginTime()==n2.getBeginTime()) return false;
	    if (n1.getBeginTime()<n2.getLastBeginTime() &&
		n1.getFirstEndTime()<=n2.getLastBeginTime()) {
		if (n1.getEndTime()+getMaxEcart() <n2.getBeginTime()) return true;
		ci=new Couple(n1,n2);
		if (ancestors.containsKey(ci)) b=ancestors.get(ci);
		else{
		    b= n1.isAncestorOf(n2);
		    ancestors.put(ci,b);
		}
	    }
	    if (!b && n2.getBeginTime()<n1.getLastBeginTime()) {
		if (n2.getFirstEndTime()>n1.getBeginTime()) return false; // idem mais dans l'autre sens
		if (n2.getEndTime()+getMaxEcart() <n1.getBeginTime()) return true;
		ci =new Couple(n2,n1);
		if (ancestors.containsKey(ci)) return ancestors.get(ci);
		b= n2.isAncestorOf(n1);
		ancestors.put(ci,b);}
	    return b;}
	else  {
	    System.err.println (n1+" --" + n1.getLastBeginTime()+ " ----------la------- "+n2+ ":" +n2.getLastBeginTime());
	    if (n1.getBeginTime()<n2.getLastBeginTime() && 
		n1.getFirstEndTime()<=n2.getLastBeginTime()) {
		System.err.println ("ici");// rajouter pour cecilia sarkozy !!!!!le 26 novembre 2008
		if (n1.getEndTime()+getMaxEcart() <n2.getBeginTime()) return true;
		ci=new Couple(n1,n2);
		if (ancestors.containsKey(ci)) b=ancestors.get(ci);
		else{System.err.println ("la ");
		    b= n1.isAncestorOf(n2);
		    System.err.println ("la2 "+b);
		    ancestors.put(ci,b);
		}
	    }
	    
	    if (!b && n2.getBeginTime()<n1.getLastBeginTime()) {
		System.err.println ("la3 "+b);
		if (n2.getFirstEndTime()>n1.getBeginTime()) return false; // idem mais dans l'autre sens
		System.err.println ("la4 "+b);
		if (n2.getEndTime()+getMaxEcart() <n1.getBeginTime()) return true;
		System.err.println ("la5 "+b);
		ci =new Couple(n2,n1);
		if (ancestors.containsKey(ci)) return ancestors.get(ci);
		b= n2.isAncestorOf(n1);
		System.err.println ("la6 "+b);
		ancestors.put(ci,b);}
	    return b;}
    }
    /**
     * Check whether these to clusters stand in a relation to each other.
     * Two clusters are related if a member of one is an ancestor of a member
     * of the other cluster.
     * 
     * @param cluster1 the first cluster
     * @param cluster2 the second cluster
     * @return true if the clusters are related
     */
    private  final boolean  modeDebug=false;
    protected boolean areClustersInRelation(Cluster cluster1, Cluster cluster2) {
	//	if (cluster1.id==35 && cluster2.id==32) 
// 	{
// 		System.err.println (cluster1 + "------------------"+cluster2);
// 		printCluster(cluster1);
// 		printCluster(cluster2);
// 		modeDebug=true;
// 	    }
	    //	else modeDebug=false;
        Iterator<NodeEdge>  i = cluster1.l.iterator();
	for (NodeEdge n1 : cluster1.l )
	    for (NodeEdge n2 : cluster2.l){
                if (hasAncestralRelationship(n1,n2)) {
		   return true;
                }
	    }
	return false;
    }
    
    /**
     * Calculate the distance between two clusters, forcing them to have the same
     * words in them, and to not be related to each other.
     * 
     * @param cluster1 the first cluster
     * @param cluster2 the second cluster
     * @return The intra cluster distance, or Float.NEGATIVE_INFINITY if the clusters
     *         should never be clustered together.
     */

    protected float intraClusterDistance(Cluster cluster1, Cluster cluster2) {
        float maxSim = Float.NEGATIVE_INFINITY;
        if (!cluster1.l.get(0).getWord().getSpelling().equals(cluster2.l.get(0).getWord().getSpelling())) return maxSim;
	CoupleInt ci = new CoupleInt(cluster1.id,cluster2.id);
	if (interdit.contains(ci)){hit++; return maxSim;}
	if (distance.containsKey(ci)) {
	    hit++;
	    return distance.get(ci);
	}
	nohit++;
        Iterator <NodeEdge>i1 = cluster1.l.iterator();
        while (i1.hasNext()) {
            NodeEdge node1 = i1.next();
            Iterator <NodeEdge>  i2 = cluster2.l.iterator();
            while (i2.hasNext()) {
                NodeEdge node2 = i2.next();
		//               if (!node1.getWord().getSpelling().equals(node2.getWord().getSpelling())) {
		//  return Float.NEGATIVE_INFINITY;
                //}
             
                if (hasAncestralRelationship(node1,node2)) {
		    interdit.add(ci);
                    return  Float.NEGATIVE_INFINITY;
                }
                float overlap = 0.0f;
                if (node1.getBeginTime() <= node2.getBeginTime() &&
		    node1.getEndTime() >= node2.getBeginTime()) {
                    overlap = node1.getEndTime() - node2.getBeginTime();
                    if (node1.getEndTime() > node2.getEndTime()) {
                        overlap -= node2.getEndTime() - node1.getEndTime(); 
                    }
                } else if(node2.getBeginTime() <= node1.getBeginTime() &&
			  node2.getEndTime() >= node1.getBeginTime()) {
                    overlap = node2.getEndTime() - node1.getBeginTime();                    
                    if (node2.getEndTime() > node1.getEndTime()) {
                        overlap -= node1.getEndTime() - node2.getEndTime(); 
                    }
                }
                if (overlap > 0.0) {
                    overlap = lattice.getLogMath().linearToLog(overlap);
                    overlap += node1.getPosterior() + node2.getPosterior();
                    if (overlap > maxSim) {
                        maxSim = overlap;
                    }
                }
            }
        }  
	distance.put(ci,maxSim);      
        return maxSim;
    }
    
    /**
     * print out a cluster for debugging
     * 
     * @param cluster
     */
    protected void printCluster(Cluster cluster) {
	System.err.println( "id : "+cluster.id + cluster.lesSucs());
	for (NodeEdge n : cluster.l) {
            System.err.print(" ("+n.getLastBeginTime()+"," + n+")");                
        }
	System.err.println();
    }
    
    /**
     * print out a list of clusters for debugging
     * 
     * @param clusters
     */
    protected void printClusters(List <Cluster>  clusters) {
        ListIterator<Cluster> i = clusters.listIterator();
        while (i.hasNext()) {
            System.err.print("----cluster " + i.nextIndex() + " : ");
            printCluster(i.next());
        }
        logger.info("----");
    }
    private ArrayList<Cluster>  toMerged;  
    /**
     * Perform the intra word clustering stage of the algorithm
     * 
     * @param clusters the current list of clusters
     */
    
    protected void intraWordCluster(List <Cluster> clusters) {
	toMerged=new ArrayList<Cluster>(); 
	countDump=clusters.size();
        while (intraWordClusterStep(clusters));
       
    }
    
    /**
     * Perform a step of the intra word clustering stage
     * 
     * @param clusters the current list of clusters
     * @return did two clusters get merged?
     */
    private int hit=0,nohit=0,countDump;
protected boolean intraWordClusterStep(List<Cluster> clusters) {
        Cluster toBeMerged1 = null;
        Cluster toBeMerged2 = null;
        int count=0;
        float maxSim = Float.NEGATIVE_INFINITY;
        ListIterator <Cluster> i = clusters.listIterator();
	if ( clusters.size() <=countDump) {countDump =clusters.size() -100;
	    logger.info("intra :" + clusters.size() + " ancestor " + ancestors.size()+
			       " distance:"+ distance.size()+ " int:"+interdit.size()
			       + " hit:"+hit + " nohit :" +nohit);
                               
	}
        while (i.hasNext()) {
            Cluster c1 = i.next();
            if (!i.hasNext()) break ;
// 	    if( toBeMerged2!=null 
// 		&& toBeMerged2.getEndTime()+100<c1.getBeginTime()) {
//                 logger.info("sortie a " + c1.getBeginTime());
//                 break;
//             } 
            count++;
            ListIterator <Cluster>  j = clusters.listIterator(i.nextIndex());
            while (j.hasNext()) {
                Cluster  c2 = j.next();
		if  (c1.getEndTime()+ 1  < c2.getBeginTime()) break;
                float sim = intraClusterDistance(c1,c2);
                if (sim > maxSim) {
                    maxSim = sim;
		    // on est intra mot 0,5 s est peut etre suffisant
                    if ( toBeMerged2!=null 
			 && toBeMerged2.getEndTime()+50<c1.getBeginTime())
			{toMerged.add(toBeMerged1);toMerged.add(toBeMerged2);}
                    toBeMerged1 = c1;
                    toBeMerged2 = c2;
                }
            }
        }
        if (toBeMerged1 != null) {
	    toMerged.add(toBeMerged1);toMerged.add(toBeMerged2);
	    for (int clus=0; clus<toMerged.size();){
		toBeMerged1=toMerged.get(clus++);
		toBeMerged2=toMerged.get(clus++);
		if (false) {
		    logger.info("merge :	"+toBeMerged1.id +
				       " "+ toBeMerged2.id);
		printCluster(	toBeMerged1);
		printCluster(	toBeMerged2);
		}
            clusters.remove(toBeMerged2);
            toBeMerged1.addAll(toBeMerged2);
 	    }
            toMerged.clear();
            return true;
        }
        return false;
    }
    public Sausage makeSausage() {
	return makeSausage(this.lattice,false);
    }
    /**
     * Turn the lattice contained in this sausage maker into a sausage object.
     * 
     * @return the sausage producing by collapsing the lattice.
     */
    public Sausage makeSausage(Lattice l,boolean useAcoustic) {
	lattice=l;
	if (allocLm && languageModel !=null ){
	    try {
	    languageModel.allocate();
	    allocLm=false;}
	    catch (Exception e){
		
		logger.warning(" lm a rate");
		languageModel=null;
	    }
	}
	if (logMath != lattice.getLogMath())
	    new Error("logMath mismatch");
        logger.info("lm "+ languageWeight + " wip:"+ wip);
        l.computeNodePosteriors(languageWeight,useAcoustic,prune,seuilabs,wip,languageModel,scale);
	LatticeOptimizerBis op = new LatticeOptimizerBis(l);
	op.optimize();
	// si wip>0 lineartolog else wiplocal in latttice
	maxEcart =100;
	int ecart=0;
        numero=0;
        ArrayList <Cluster>  clusters;
	if (edgeMode)
	{clusters = new ArrayList <Cluster> (lattice.getNodes().size());
	logger.info(" cluster debut :" +clusters.size());
        ArrayList  <NodeEdge> nodes = new ArrayList <NodeEdge>(lattice.getEdges());
	Collections.sort(nodes, new Comparator<NodeEdge>() { 
			     public int compare  (NodeEdge  t1, NodeEdge t2) {
				int ret= Double.compare(t1.getEndTime(), t2.getEndTime());
				if (ret==0) return t1.getWord().getSpelling().compareTo(
								   t2.getWord().getSpelling());
				else return ret;

			     }
			 });
        Iterator i = nodes.iterator();
	logger.info(" cluster avant prune :"+ nodes.size());
        int nb=0;
        Cluster al=null;
	while(i.hasNext()) {
            NodeEdge n=(NodeEdge) i.next();    
	    nb++;
            if (n.getEndTime()-n.getBeginTime() >ecart)
		ecart= n.getEndTime()-n.getBeginTime();
            if ( n.getIsNotPruned()){
		if (al!=null &&al.l.get(0).getEndTime()== n.getEndTime() &&al.l.get(0).getWord().equals(n.getWord()))
		    al.add(n);
		else {
               
		Cluster  bucket = new Cluster (n);
		clusters.add(bucket);
		al=bucket;
		}
	    }
	}}
	else
	{clusters = new ArrayList <Cluster> (lattice.getNodes().size());
	logger.info(" cluster debut :" +clusters.size());
        ArrayList  <NodeEdge> nodes = new ArrayList <NodeEdge>(lattice.nodes.values());
	Collections.sort(nodes, new Comparator<NodeEdge>() { 
			     public int compare  (NodeEdge  t1, NodeEdge t2) {
				 return Double.compare(t2.getPosterior(), t1.getPosterior());}
			 });
        Iterator i = nodes.iterator();
	logger.info(" cluster avant prune :"+ nodes.size());
        int nb=0;
	while(i.hasNext()&& nb< nodes.size()/10) {
            NodeEdge n=(NodeEdge) i.next();    
	    nb++;
            if (true || n.getIsNotPruned()){
		 if (n.getEndTime()-n.getBeginTime() >ecart)
		ecart= n.getEndTime()-n.getBeginTime();
		Cluster  bucket = new Cluster (n);
		clusters.add(bucket);
	    }
	}}
	setMaxEcart(ecart+20);
	logger.info(" cluster avant tri :"+ clusters.size());
        Collections.sort(clusters, new Comparator<Cluster>() { 
			     public int compare  (Cluster  t1, Cluster t2) {
				 return t1.getBeginTime() - t2.getBeginTime();}
			 });
        logger.info(" cluster :"+ clusters.size());
	if (false) {
	    for (Cluster c: clusters)
		if (c.getBeginTime()<=2) 
		    System.err.print(c.id+" "+c.l.get(0)+" "+
				       c.l.get(0).getWord().getSpelling());
		else break;
	System.err.println();}
        intraWordCluster(clusters);
	logger.info(" clusteravfiltre intra :" +clusters.size());
	ArrayList <Cluster> aDetruire= new ArrayList <Cluster> (100);
	for (Cluster c :clusters) 
	    if (c.post() <= seuilMot) aDetruire.add(c);
	for (Cluster c: aDetruire){
	    // System.err.format(" je vire :%s : %f\n",c.l.get(0).getWord(),c.post());
	    clusters.remove(c);
	}
	aDetruire.clear();
	
        if (false) {
	    ListIterator <Cluster> ic = clusters.listIterator();
	    int count=0;
	    while (ic.hasNext()) {
		Cluster c1 = ic.next();count++;
		//System.err.print((count++) + " " +c1.l.size() +"\n      ");
		if (c1.l.size()<=1) continue;
		System.err.print((count) + " " +c1 +"\n      ");
		printCluster(c1);
		System.err.print("     ");
		ListIterator <Cluster>  j = clusters.listIterator(ic.nextIndex());
		while (j.hasNext()) {
		    Cluster  c2 = j.next();
		    CoupleInt ci=new CoupleInt(c1.id,c2.id);
		    if (interdit.contains(ci))
			System.err.print(c2.id+",");
		}
		System.err.println();
	    }
	}
	logger.info(" clusterfiltre intra :" +clusters.size());
        interWordCluster(clusters);
	logger.info(" clusterapres inter :" +clusters.size());
     
	int count =0;
// 	ListIterator <Cluster> ic = clusters.listIterator();
// 	while (ic.hasNext()) {
//             Cluster c1 = ic.next();
// 	    System.err.print((count++) + " " +c1 +"\n      ");
//             printCluster(c1);
//             System.err.print("     ");
// 	    ListIterator <Cluster>  j = clusters.listIterator(ic.nextIndex());
// 	    while (j.hasNext()) {
//                 Cluster  c2 = j.next();
// 		CoupleInt ci=new CoupleInt(c1.id,c2.id);
// 		if (interdit.contains(ci))
// 		    System.err.print(c2.id+",");
// 	    }
// 	    System.err.println();
//	}
	clusters = topologicalSort(clusters);

        Sausage sausage = new Sausage(clusters.size()+ (edgeMode?1 :0));
        ListIterator<Cluster>  c1 = clusters.listIterator();
        while (c1.hasNext()) {
            int index = c1.nextIndex();
            final Cluster cluster =  c1.next();
            ConfusionSet cs= sausage.getConfusionSet(index);
	    cs.setBeginTime(cluster.getBeginTime());
	    cs.setEndTime(cluster.getEndTime());
	    	 if (logger.isLoggable(Level.FINER)){ //debug
		System.err.println(cluster.idTopo +"clus :"+ cluster);
		    for (NodeEdge e: cluster.l) 
			System.err.print( "  (" +e.getLastBeginTime() +","+  e +"),");
		System.err.println();
	    }



	    for (Word word :cluster.proba.keySet() ) {
                SimpleWordResult swr = new SimpleWordResult
                    (word.getSpelling(),
                     cluster.proba.get(word),
                     lattice.getLogMath());
                sausage.addWordHypothesis(index,swr);
            }
        }
        if (edgeMode) {
	SimpleWordResult swr= new SimpleWordResult("</s>",0.0,lattice.getLogMath());
	sausage.addWordHypothesis(clusters.size(),swr);
	}
        sausage.fillInBlanks(lattice.getLogMath());// sort thes ConfusSet
        distance.clear();
	interdit.clear();
	ancestors.clear();
	if (dumpLattice)
	    l.dumpAISee("lat"+(compteSaus)+".gdl","lattice");
	if (dumpSausage) 
	    sausage.dumpAISee("saus"+(compteSaus++)+".gdl","saussice");
        return sausage;
    }
    private int compteSaus=0;
    /**
     * @see edu.cmu.sphinx.result.ConfidenceScorer#score(edu.cmu.sphinx.result.Result)
     */
    public ConfidenceResult score(Lattice l) {
	return makeSausage(l,false);
    }
    public ConfidenceResult score(Result result) {
        lattice = new Lattice(result,keepAll);
        LatticeOptimizer lop = new LatticeOptimizer(lattice);
        //lop.optimize();
	// lattice.computeNodePosteriors(languageWeight);
        return makeSausage(lattice,true);// the acoustic has all scores
    }

    /**
     * Topologically sort the clusters. Note that this is a brunt force
     * sort by removing the min cluster from the list of clusters,
     * since Collections.sort() does not work in all cases.
     *
     * @param clusters the list of clusters to be topologically sorted
     *
     * @return a topologically sorted list of clusters
     */
    private ArrayList <Cluster> topologicalSort(ArrayList <Cluster> clusters) {

        ArrayList <Cluster>  sorted = new ArrayList<Cluster>( clusters);
        {int count=0;  
      for (Cluster c : sorted) 
	  c.idTopo=count++;
	}
      int [][] distance = new int[sorted.size()][sorted.size()];
      System.err.format("%d\n",getMaxEcart());
      for (int i=0 ;i< sorted.size();i++) {
	  distance[i][i]=0;
	  //System.err.format("%"+(3*(i+1))+"d",0);

	  Cluster ci=sorted.get(i);
	  for (int j=i+1 ;j<sorted.size();j++){
	      Cluster cj= sorted.get(j);//100 or 200
	      distance[i][j]=0;
              if (ci.getEndTime()+ getMaxEcart()  < cj.getBeginTime()) distance[i][j]=-1;
              if (ci.suc.contains(cj.getCont())) distance[i][j]=-1;
              if (cj.suc.contains(ci.getCont())) distance[i][j]=1;

		  { 

		
		  
		  CoupleInt couple= new CoupleInt(ci.id,cj.id);
		  if (distance[i][j]==0 && ! interdit.contains(couple)) throw  new Error( ci +"  cj:"+cj);
		  
		  Iterator <NodeEdge>  ic = ci.l.iterator();
		  while (ic.hasNext() && distance[i][j]==0) {
		      NodeEdge n1 = ic.next();
		      Iterator <NodeEdge> ic2 = cj.l.iterator();
		      while (ic2.hasNext()&& distance[i][j] ==0) {
			  NodeEdge n2 = ic2.next();
			  if (n1.getEndTime()+getMaxEcart() <n2.getBeginTime()|| n1.isAncestorOf(n2)) {
			      distance[i][j]=-1;
			     logger.finer("top( "+i+","+j  + "  ," +  n1.isAncestorOf(n2)
			      		 + " n1:" +n1 +" n2:"+n2);
			  } else if (n2.getEndTime()+getMaxEcart() <n1.getBeginTime()||n2.isAncestorOf(n1)) {
			      distance[i][j]= 1; 
			       logger.finer("top( "+j+","+i  + "  ," +  n2.isAncestorOf(n1)
			      			 + " n1:" +n2 +" n2:"+n1);
			  }
		      }
		  }
	      }
	      if (distance[i][j]==0) { 
                   Iterator <NodeEdge>  ic = ci.l.iterator();
		  while (ic.hasNext() && distance[i][j]==0) {
		      NodeEdge n1 = ic.next();
		      System.err.print ("ooo \n"+n1+"\n           "); //debug

		      Iterator <NodeEdge> ic2 = cj.l.iterator();
		      while (ic2.hasNext()&& distance[i][j] ==0) {
			  NodeEdge n2 = ic2.next(); 
			  System.err.print(n2+","); //debug
			  if (n1.getEndTime()+getMaxEcart() <n2.getBeginTime()|| n1.isAncestorOf(n2)) {
			      distance[i][j]=-1;
			        logger.finer("top( "+i+","+j + "  ," +  n1.isAncestorOf(n2)
			      		 + " n1:" +n1 +" n2:"+n2);
			  } else if ( n2.getEndTime()+getMaxEcart() <n1.getBeginTime() || n2.isAncestorOf(n1)) {
			      distance[i][j]= 1; 
			        logger.finer("top( "+j+","+i + "  ," +  n2.isAncestorOf(n1)
			      			 + " n1:" +n2 +" n2:"+n1);
			  }
			  else
			      if (hasAncestralRelationship(n1,n2)||hasAncestralRelationship(n2,n1) ) {
				  System.err.println("\n\n\n very bizarreeeeeeee " +n1+ " n2      "+n2 + "  " +hasAncestralRelationship(n1,n2) );
			      }
		      }
		  }
		  boolean btoto=areClustersInRelation(ci,cj);
		  System.err.println("distance :" + interClusterDistance(ci,cj)+ " " + btoto);
		      throw  new Error("distance nulle max ecart:" + getMaxEcart()+ "ci :"  +ci+ " cj : "+cj);
	      }
		distance[j][i]= -distance[i][j];
		//System.err.format("%3d",distance[i][j]);
	  }
	  // System.err.println();
      }
	Comparator comparator = new SausageMakerEdges.ClusterComparator(distance);
	Collections.sort(sorted,comparator);
	if (false) {
	    for (Cluster c:sorted) 
		System.err.println("clus ordonne :"+ c.idTopo);
	}
        return sorted;
    }
}
