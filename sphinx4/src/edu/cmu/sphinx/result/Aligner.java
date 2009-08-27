
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





    private String name;
    private Logger logger;
    private boolean dumpSausage;

    
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

    private int ecart(ConfusionSet cs, String mot) {
	for (WordResult wr: cs) {
	    if (mot.equals(wr.toString()))
		return 0;
	}
	return 1;
    }
    int compte =0;
    public String aligner(String ref,ConfidenceResult saus) {
        if (dumpSausage && saus instanceof Sausage) {
	    ((Sausage) saus).dumpAISee("lu.gdl"+compte, "lu Sausage");
        }
	String [] mots = ref.split("\\s+");
	int [][]distance= new int[mots.length+1][saus.size()+1];
	CoupleInt [][]trace= new CoupleInt[mots.length+1][saus.size()+1];
	for (int i=0 ;i<distance.length;i++)
	    for (int j=0 ;j<distance[i].length;j++)
		distance[i][j]=Integer.MAX_VALUE;
	distance[0][0]=0;
	for (int col=1; col<distance[0].length;col++)
	    distance[0][col] =distance[0][col-1]+ecart(saus.getConfusionSet(col-1),"eps");
	for (int ligne =0;ligne<distance.length;ligne++) {
	    distance[ligne][0]=ligne;
	}
       	for (int mot=1 ;mot<distance.length;mot++)
	    for (int cs=1 ;cs<distance[mot].length;cs++){
		int inf;
		CoupleInt ci;
		int inscs=distance[mot][cs-1]+ecart(saus.getConfusionSet(cs-1),"eps");
		int egal=distance[mot-1][cs-1]+ecart(saus.getConfusionSet(cs-1),mots[mot-1]);
		int insmot=distance[mot-1][cs]+1;
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
	boolean first=false;
	StringBuffer sb=new StringBuffer();
	while (mot !=0 && cs!=0) {
	CoupleInt c=trace[mot][cs];
	if (c.mot== mot-1 && c.cs==cs-1) {
	    if (first) first=false;
	    else sb.insert(0," "); 


	    if (distance[mot][cs]==distance[mot-1][cs-1]) 
		sb.insert(0,mots[mot-1]);
	    else
		sb.insert(0,saus.getConfusionSet(cs-1).getBestHypothesis().toString());
	    if (dumpSausage && saus instanceof Sausage) {
		((Sausage)saus).addWordHypothesis(cs-1,mots[mot-1],-1,null);
	    }
	}
	else 
	    if (c.mot==mot-1 && c.cs==cs) {
		//J'AI MANGE LE MOT 
		if (dumpSausage && saus instanceof Sausage) {
		((Sausage)saus).addWordHypothesis(cs-1,mots[mot-1],-3,null);
		}
	    }
	    else if (c.mot==mot && c.cs==cs-1)
		{ if (distance[mot][cs]!=distance[mot-1][cs-1])
			{  if (first) first=false;
			    else sb.insert(0," ");
			    sb.insert(0,saus.getConfusionSet(cs-1).getBestHypothesis().toString());}
		}
	    else throw new Error("cela ne devrait pas faire "+ mot+ " "+ cs);
	mot=c.mot;
	cs=c.cs;
	}
	if (dumpSausage && saus instanceof Sausage) {
	    ((Sausage) saus).dumpAISee("mod.gdl"+(compte++), "lu Sausage");
        }

	return sb.toString();
    }
}
