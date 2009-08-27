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
public class SausageMakerFast implements ConfidenceScorer, Configurable {
  private int numero=0;
    private class Cluster {
	int id =numero++;
        int idTopo;
	private int beginTime=-100;
	private int endTime=-100;
	private int minTime=-100;
	private int maxTime=-100;
	ArrayList <Node> l;
	HashMap <Word,Float> proba=null;
	Cluster(ArrayList <Node>  l) {
	    this.l=l;
	    this.id =id;
	}
	Cluster (Node n) {
	    l = new ArrayList <Node> (1);
	    l.add(n);
	    minTime=beginTime=n.getBeginTime();
	    maxTime= endTime =n.getEndTime();
       
	}
	void calculWordPost() {
            proba= new HashMap <Word,Float>();
	    for (Node n : l) {
		Word  w=n.getWord();
		if (proba.containsKey(w)) 
		    proba.put(w,logMath.addAsLinear(proba.get(w),(float)n.getPosterior()));
		else
		    proba.put(w,(float)n.getPosterior());
	    }
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

 // pour le min normale c'est deja le cas
	}

	public String toString() {
	    return "id:"+id+" bt:"+ getBeginTime() +" et:"+ getEndTime()+
		" s:"+l.size()+ " latesbeg: " + getLatestBeginTime()+  " earlest:" + getEarliestEndTime();
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
	    for (Node n : l) 
		if (n.getBeginTime()< minTime)
		    minTime=n.getBeginTime();
	    return minTime;
	}

	int getLatestBeginTime() {
	    if (beginTime >=0) return beginTime;
	    for (Node n : l) 
		if (n.getBeginTime()> beginTime)
		    beginTime=n.getBeginTime();
	    return beginTime;
	}
	int getEndTime() {
	    if (maxTime >=0) return maxTime;
	    for (Node n :l)
		if (n.getEndTime() >maxTime)
		    maxTime = n.getEndTime();
	    return maxTime;
	}
	int getEarliestEndTime() {
	    if (endTime >=0) return endTime;
	    endTime = Integer.MAX_VALUE;
	    for (Node n :l)
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
    /**
     * Sphinx property that defines the language model weight.
     */
    public final static String PROP_LANGUAGE_WEIGHT = "languageWeight";

    /**
     * The default value for the PROP_LANGUAGE_WEIGHT property
     */
    public final static float PROP_LANGUAGE_WEIGHT_DEFAULT  = 1.0f;
    
    private String name;
    private float languageWeight;
    
    protected Lattice lattice;
    private  LogMath logMath;

    /**
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
        throws PropertyException {
        this.name = name;
        registry.register(PROP_LANGUAGE_WEIGHT, PropertyType.FLOAT);
    }


    /**
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        languageWeight = ps.getFloat(PROP_LANGUAGE_WEIGHT,
                                     PROP_LANGUAGE_WEIGHT_DEFAULT);
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
    public SausageMakerFast() {
    }
    
    /**
     * Construct a sausage maker
     *
     * @param l the lattice to construct a sausage from
     */
    public SausageMakerFast(Lattice l) {
        lattice = l;
        logMath= lattice.getLogMath();
    }

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
	 while(interWordClusterStep(clusters,false));
         countDump=clusters.size();
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
        } else if (latestBeginTime2 < earliestEndTime1 &&
                   latestBeginTime1 < earliestEndTime2) {
            return true;
        } else {
            return false;
        }
    }
    
    /**
     * Returns the latest begin time of all nodes in the given cluster.
     *
     * @param cluster the cluster to examine
     *
     * @return the latest begin time
     */
   
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
	int ecart=(ncont) ? 200:1;
	if (clusters.size() <=countDump) {
	    countDump=clusters.size()-100;
	    System.err.println("inter :" + clusters.size() + 
			       " ancestor " + ancestors.size()+
			       " distance:"+ distance.size()+ 
			       " int:"+interdit.size()+
			       " distanceEd:"+  distanceEdition.size()
			       +" hit:" +hit + " nohit:"+nohit);
	    
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
                float sim = interClusterDistance(c1,c2);
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
                if (false){
	     	    System.err.println("merge : " +toBeMerged1+ " " +toBeMerged2);
		    printCluster(toBeMerged1);
		    printCluster(toBeMerged2);
		}


            clusters.remove(toBeMerged2);
            toBeMerged1.addAll(toBeMerged2);
	    }
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
    protected float clusterProbability(List <Node> cluster) {
        float p = LogMath.getLogZero();
        
        for (Node n : cluster)
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
        ArrayList <Node> sub = new ArrayList <Node>();
	for ( Node n : cluster.l) 
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
    protected boolean hasAncestralRelationship(Node n1, Node n2) {
	Couple ci;
	boolean b;
        if (n1.getBeginTime()==n2.getBeginTime()) return false;
	if (n1.getBeginTime()<n2.getBeginTime()){
	    if (n1.getFirstEndTime()>n2.getBeginTime()) return false;
	    if (n1.getEndTime()+100 <n2.getBeginTime()) return true;
	    ci=new Couple(n1,n2);
	    if (ancestors.containsKey(ci)) return ancestors.get(ci);
	    b= n1.isAncestorOf(n2);
	}
	else {
	    if (n2.getFirstEndTime()>n1.getBeginTime()) return false;
	    if (n2.getEndTime()+100 <n1.getBeginTime()) return true;
	    ci =new Couple(n2,n1);
	    if (ancestors.containsKey(ci)) return ancestors.get(ci);
	    b= n2.isAncestorOf(n1);
	}
        ancestors.put(ci,b);
	return b;
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
    protected boolean areClustersInRelation(Cluster cluster1, Cluster cluster2) {
        Iterator<Node>  i = cluster1.l.iterator();
        while (i.hasNext()) {
            Iterator <Node>  j = cluster2.l.iterator();
            Node n1 = i.next();
            while (j.hasNext()) {
                if (hasAncestralRelationship(n1,j.next())) {
                    return true;
                }
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
        Iterator <Node>i1 = cluster1.l.iterator();
        while (i1.hasNext()) {
            Node node1 = i1.next();
            Iterator <Node>  i2 = cluster2.l.iterator();
            while (i2.hasNext()) {
                Node node2 = i2.next();
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
        for (Node n : cluster.l) {
            System.err.print(" " + n);                
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
        System.err.println("----");
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
	    System.err.println("intra :" + clusters.size() + " ancestor " + ancestors.size()+
			       " distance:"+ distance.size()+ " int:"+interdit.size()
			       + " hit:"+hit + " nohit :" +nohit);
                               
	}
        while (i.hasNext()) {
            Cluster c1 = i.next();
            if (!i.hasNext()) break ;
// 	    if( toBeMerged2!=null 
// 		&& toBeMerged2.getEndTime()+100<c1.getBeginTime()) {
//                 System.err.println("sortie a " + c1.getBeginTime());
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
		    System.err.println("merge :	"+toBeMerged1.id +
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
    
    /**
     * Turn the lattice contained in this sausage maker into a sausage object.
     * 
     * @return the sausage producing by collapsing the lattice.
     */
    public Sausage makeSausage() {

        numero=0;
        ArrayList <Cluster>  clusters = new ArrayList <Cluster> (lattice.getNodes().size());
	System.err.println(" cluster debut :" +clusters.size());
        ArrayList  <Node> nodes = new ArrayList <Node>(lattice.nodes.values());
	Collections.sort(nodes, new Comparator<Node>() { 
			     public int compare  (Node  t1, Node t2) {
				 return Double.compare(t2.getPosterior(), t1.getPosterior());}
			 });
        Iterator i = nodes.iterator();
	System.err.println(" cluster avant prune :"+ nodes.size());
        int nb=0;
	while(i.hasNext()&& nb< nodes.size()/10) {
            Node n=(Node) i.next();    
	    nb++;
            if (true || n.getIsNotPruned()){
		
		Cluster  bucket = new Cluster (n);
		clusters.add(bucket);
	    }
	}
	System.err.println(" cluster avant tri :"+ clusters.size());
        Collections.sort(clusters, new Comparator<Cluster>() { 
			     public int compare  (Cluster  t1, Cluster t2) {
				 return t1.getBeginTime() - t2.getBeginTime();}
			 });
        System.err.println(" cluster :"+ clusters.size());
	if (false) 
	    for (Cluster c: clusters)
		if (c.getBeginTime()<=2) 
		    System.err.println(c.id+" "+c.l.get(0)+" "+
				       c.l.get(0).getWord().getSpelling());
		else break;
	System.err.println();
        intraWordCluster(clusters);
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
	System.err.println(" clusterapres intra :" +clusters.size());
        interWordCluster(clusters);
	System.err.println(" clusterapres inter :" +clusters.size());
     
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
	interdit.clear();
	distance.clear();
	ancestors.clear();
        Sausage sausage = new Sausage(clusters.size());
        ListIterator<Cluster>  c1 = clusters.listIterator();
        while (c1.hasNext()) {
            int index = c1.nextIndex();
            final Cluster cluster =  c1.next();
	    for (Word word :cluster.proba.keySet() ) {
                SimpleWordResult swr = new SimpleWordResult
                    (word.getSpelling(),
                     cluster.proba.get(word),
                     lattice.getLogMath());
                sausage.addWordHypothesis(index,swr);
            }
        }
        sausage.fillInBlanks(lattice.getLogMath());// sort thes ConfusSet
        return sausage;
    }
    public ConfidenceResult score ( Lattice l) {
	lattice=l;
	lattice.computeNodePosteriors(languageWeight);
        return makeSausage();
    }
    /**
     * @see edu.cmu.sphinx.result.ConfidenceScorer#score(edu.cmu.sphinx.result.Result)
     */
    public ConfidenceResult score(Result result) {
        lattice = new Lattice(result);
        LatticeOptimizer lop = new LatticeOptimizer(lattice);
        lop.optimize();
        lattice.computeNodePosteriors(languageWeight);
        return makeSausage();
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
      for (int i=0 ;i< sorted.size();i++) {
	  distance[i][i]=0;
	  //System.err.format("%"+(3*(i+1))+"d",0);
	  Cluster ci=sorted.get(i);
	  for (int j=i+1 ;j<sorted.size();j++){
	      Cluster cj= sorted.get(j);//100 or 200
              if (ci.getEndTime()+ 200  < cj.getBeginTime()) distance[i][j]=-1;
              else { 

		  distance[i][j]=0;
		  CoupleInt couple= new CoupleInt(ci.id,cj.id);
		  if (! interdit.contains(couple)) throw  new Error( ci +"  cj:"+cj);
		  Iterator <Node>  ic = ci.l.iterator();
		  while (ic.hasNext() && distance[i][j]==0) {
		      Node n1 = ic.next();
		      Iterator <Node> ic2 = cj.l.iterator();
		      while (ic2.hasNext()&& distance[i][j] ==0) {
			  Node n2 = ic2.next();
			  if (n1.isAncestorOf(n2)) {
			      distance[i][j]=-1;
			      System.err.println("top( "+i+","+j
						 + " n1:" +n1 +" n2:"+n2);
			  } else if (n2.isAncestorOf(n1)) {
			      distance[i][j]= 1; 
			      System.err.println("top( "+j+","+i
						 + " n1:" +n2 +" n2:"+n1);
			  }
		      }
		  }
	      }
		if (distance[i][j]==0)  throw  new Error("distance nulle" +ci+ "  cj:"+cj);
		distance[j][i]= -distance[i][j];
		//System.err.format("%3d",distance[i][j]);
	  }
	  // System.err.println();
      }
	Comparator comparator = new SausageMakerFast.ClusterComparator(distance);
	Collections.sort(sorted,comparator);
        return sorted;
    }
}
