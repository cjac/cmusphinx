
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
package edu.cmu.sphinx.result;

import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import java.io.PrintWriter;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.LinkedList;
import java.util.logging.Logger;
import java.util.Scanner;
import java.util.Locale;
/**
 * aligne word list with ConfusionSets
 */
public class Aligner implements Configurable {

 /**
     * Sphinx property that specifies whether to dump the sausage.
     */
    public final static String PROP_DUMP_SAUSAGE = "dumpSausage";

    /**
     * The default value of PROP_DUMP_SAUSAGE.
     */
    public final static boolean PROP_DUMP_SAUSAGE_DEFAULT = false;
    public final static String PROP_OUT_SCORE = "outScore";

    /**
     * The default value of PROP_DUMP_SAUSAGE.
     */
    public final static boolean PROP_OUT_SCORE_DEFAULT = false;
    




    private String name;
    private Logger logger;
    private boolean dumpSausage;
    private boolean outScore;
    private boolean timing=false;
    /*
     * (non-Javadoc)
     *
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
        throws PropertyException {
        this.name = name;
	registry.register(PROP_DUMP_SAUSAGE, PropertyType.BOOLEAN);
	registry.register(PROP_OUT_SCORE, PropertyType.BOOLEAN);
	registry.register("timing", PropertyType.BOOLEAN);
	registry.register("sortie", PropertyType.STRING);
	
    }

    
    /*
     * (non-Javadoc)
     *
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
	logger = ps.getLogger();
	dumpSausage = ps.getBoolean(PROP_DUMP_SAUSAGE,
                                    PROP_DUMP_SAUSAGE_DEFAULT);
	outScore = ps.getBoolean(PROP_OUT_SCORE,
                                    PROP_OUT_SCORE_DEFAULT);
	timing  = ps.getBoolean("timing",
                                    timing);

    }

    private class CoupleInt {
	int mot,cs;
	CoupleInt(int l,int c) {
	    mot=l;cs=c;
	}
    }
    /*
     * (non-Javadoc)
     *
     * @see edu.cmu.sphinx.util.props.Configurable#getName()
     */
    public String getName() {
        return name;
    }
    private int  ecartSkip(ConfusionSet cs) {
	for (WordResult wr: cs) {
	    
	    if ("eps".equals(wr.toString()))
		return 0;
	    if( wr.getWord().isFiller()){
		cs.setHasFiller(true);
		return 0;
	    }
	}
	return 1;
    }
    private int ecart(ConfusionSet cs, String mot) {
	if (true){for (WordResult wr: cs) {
	    if (mot.equals(wr.toString()))
		return 0;
	}
	    return 1;} else {
	// best if (cs.getWordResult(mot)!=null) return 1; else return 0;
	    WordResult wr =cs.getWordResult(mot);
	    if (cs!=null) return 1; else return 0;
	}
    }

    int compte =0;
    public String aligner(String ref, ConfidenceResult saus){
	return  aligner(ref, saus,null,System.out);
    }

    public String aligner(String ref,ConfidenceResult saus,String id,java.io.PrintStream sortie ) {
        if (dumpSausage && saus instanceof Sausage) {
	    ((Sausage) saus).dumpAISee("lu"+id+".gdl", "lu Sausage");
        }
	float [] tempo=null;
	String[] temp=null;
	if (timing) { temp=ref.split(";");
	    ref=temp[0];
	}
       	saus.getConfusionSet(0).addWordHypothesis(new SimpleWordResult("eps",0,null));
       	saus.getConfusionSet(saus.size()-1).addWordHypothesis(new SimpleWordResult("eps",0,null));
	String [] mots = ref.split("\\s+");
	if (timing) {tempo= new float[mots.length];
	    Scanner s = new Scanner(temp[1]).useDelimiter("\\s+");
	    s.useLocale(Locale.US);
	    int i;
	    tempo[0]=-10.f;
	    for (i=1; i<tempo.length && s.hasNextFloat();i++ ){
		tempo[i]=s.nextFloat();
		//	System.err.format(Locale.US,"prob %d: %f\n",i,tempo[i])	;
	    }
	    if (i!=tempo.length || s.hasNextFloat()){
		
		System.err.format ("prob : %d:%d  :%s : %s:%s:%s\n",i,tempo.length, temp[0],temp[1],
				   mots[0],mots[mots.length-1]);
		tempo=null;}
	}
	int [][]distance= new int[mots.length+1][saus.size()+1];
	CoupleInt [][]trace= new CoupleInt[mots.length+1][saus.size()+1];
	for (int i=0 ;i<distance.length;i++)
	    for (int j=0 ;j<distance[i].length;j++)
		distance[i][j]=Integer.MAX_VALUE;
	distance[0][0]=0;
	for (int col=1; col<distance[0].length;col++)
	    distance[0][col] =distance[0][col-1]+ecartSkip(saus.getConfusionSet(col-1));
	for (int ligne =0;ligne<distance.length;ligne++) {
	    distance[ligne][0]=ligne;
	}
       	for (int mot=1 ;mot<distance.length;mot++)
	    for (int cs=1 ;cs<distance[mot].length;cs++){
		int inf;
		CoupleInt ci;
		int inscs=distance[mot][cs-1]+ecartSkip(saus.getConfusionSet(cs-1));
		int egal=distance[mot-1][cs-1]+ecart(saus.getConfusionSet(cs-1),mots[mot-1]);
		int insmot=distance[mot-1][cs]+2;
		if (inscs<egal) {
		    if (inscs<=insmot) {inf=inscs; ci=new CoupleInt(mot,cs-1);}
		    else {inf=insmot;ci=new CoupleInt(mot-1,cs);}
		} else {
		    if (insmot<egal) {inf=insmot;ci=new CoupleInt(mot-1,cs);}
		    else {inf=egal;ci=new CoupleInt(mot-1,cs-1);}
		}
		distance[mot][cs]=inf;
		trace[mot][cs] = ci;
	    }
	int mot =trace.length-1;int cs=trace[mot].length-1;
        int val=distance[mot][cs];
	boolean first=false;
	StringBuffer sb=new StringBuffer();
	while (mot !=0 && cs!=0) {
	CoupleInt c=trace[mot][cs];
	if (c.mot== mot-1 && c.cs==cs-1) {
	    if (first) first=false;
	    else sb.insert(0," "); 


	    if (distance[mot][cs]==distance[c.mot][c.cs]) {
		if (dumpSausage && saus instanceof Sausage) {
		    ((Sausage)saus).addWordHypothesis(cs-1,mots[mot-1],-1,null);
		}
		if (outScore) {
		    WordResult st =saus.getConfusionSet(cs-1).getWordResult(mots[mot-1]);
		    sortie.print("wr: ");
		    if (id!=null) sortie.print(id + " ");
		    if (tempo!=null) sortie.print (String.format(Locale.US,"%.2f ",tempo[mot-1]));
		    sortie.print(mots[mot-1] +" "+st.getConfidence() );
		    saus.getConfusionSet(cs-1).sort();
		    st=saus.getConfusionSet(cs-1).getBestHypothesis();
                    if (st!=null) sortie.println(" Best: "+ st +" "+st.getConfidence());
		    else sortie.println(" 0.0");
		}
		
		sb.insert(0,mots[mot-1]);}
	    
	    else
		{
		    sb.insert(0,saus.getConfusionSet(cs-1).getBestHypothesis().toString());
		    if (dumpSausage && saus instanceof Sausage) {
			((Sausage)saus).addWordHypothesis(cs-1,mots[mot-1],-2,null);
		    }
		}
	}
	else 
	    if (c.mot==mot-1 && c.cs==cs) {
		//J'AI MANGE LE MOT 
		if (dumpSausage && saus instanceof Sausage) {
		((Sausage)saus).addWordHypothesis(cs-1,mots[mot-1],-3,null);
		
		}
		if (outScore) { sortie.print("sup: ");
		    if (id!=null) sortie.print(id + " ");
		    if (tempo!=null) sortie.format(Locale.US,"%.2f ",tempo[mot-1]);
		    sortie.println( mots[mot-1] + " -1");
		}
	    }
	    else if (c.mot==mot && c.cs==cs-1)
		{ if (distance[mot][cs]!=distance[c.mot][c.cs])
			{  if (first) first=false;
			    else sb.insert(0," ");
			    sb.insert(0,saus.getConfusionSet(cs-1).getBestHypothesis().toString());}
		    else 
			if (outScore && !saus.getConfusionSet(cs-1).hasFiller()) {
			    saus.getConfusionSet(cs-1).sort();
			    WordResult st =saus.getConfusionSet(cs-1).getWordResult("eps");
			    WordResult stbest =saus.getConfusionSet(cs-1).getBestHypothesis();
			    if (st==null ||(st!=stbest && st.getConfidence()<=stbest.getConfidence())) {
			    sortie.print("ins: ");
			    if (id!=null) sortie.print(id + " ");
			    sortie.print(" Best: "+ stbest +" "+stbest.getConfidence());
			    if (st!=null && st.getConfidence()!=0) sortie.println(" eps "+st.getConfidence()); //.toString() );
			    else sortie.println("eps -1");}
		    // 0 => <s> or </s>
			}
		}
	    else throw new Error("cela ne devrait pas faire "+ mot+ " "+ cs);
	mot=c.mot;
	cs=c.cs;
	}
	if (dumpSausage && saus instanceof Sausage) {
	    ((Sausage) saus).dumpAISee("mod"+id+".gdl", "lu Sausage");
	     compte++;
        }

	return val   + " " + sb.toString();
    }
}
