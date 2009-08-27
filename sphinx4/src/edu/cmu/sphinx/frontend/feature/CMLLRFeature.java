/*
 * Copyright 1999-2002 Carnegie Mellon University.  
 * Portions Copyright 2002 Sun Microsystems, Inc.  
 * Portions Copyright 2002 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 * 
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL 
 * WARRANTIES.
 *
 */
package edu.cmu.sphinx.frontend.feature;

import java.io.IOException;
import java.util.Scanner;
import java.util.Locale;
import java.io.File;
import edu.cmu.sphinx.util.props.SetId;
import edu.cmu.sphinx.frontend.BaseDataProcessor;
import edu.cmu.sphinx.frontend.Data;
import edu.cmu.sphinx.frontend.DataEndSignal;
import edu.cmu.sphinx.frontend.DataProcessingException;
import edu.cmu.sphinx.frontend.DataStartSignal;
import edu.cmu.sphinx.frontend.DoubleData;
import edu.cmu.sphinx.frontend.FloatData;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import java.util.logging.Logger;
/**
 * calulate Cmllr of stream
 *  the current cepstrum.
 * cmllr by bloc 13x13
 */
public class CMLLRFeature extends BaseDataProcessor implements SetId {
    /**
     * The name of the SphinxProperty for the base directory where the 
     * cmllr transfo
     */

	

	
    	public static final String PROP_DUMP_DIR = "dumpDir";
	
	public static final String  PROP_DUMP_DIR_DEFAULT = ".";
	public static final String PROP_BY_SENT = "bySent";
	
	public static final Boolean  PROP_BY_SENT_DEFAULT = true;
    /**
     * The default value of PROP_FEATURE_WINDOW.
     */
    private Logger logger;
    private int featureSize=0;
    private float [][][] a;
    private float [] b;
    private int [] debutBloc;
    private String id;
    private String dumpDir;
    private float [] tempo;
    private int ntrame=0;
    private boolean bySent=true;
    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
            throws PropertyException {
        super.register(name, registry);
        registry.register(PROP_DUMP_DIR, PropertyType.STRING);
	registry.register(PROP_BY_SENT, PropertyType.BOOLEAN);
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        super.newProperties(ps);
        logger = ps.getLogger();
        dumpDir = ps.getString(PROP_DUMP_DIR, PROP_DUMP_DIR_DEFAULT);
        bySent  = ps.getBoolean(PROP_BY_SENT, PROP_BY_SENT_DEFAULT);
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.frontend.DataProcessor#initialize(edu.cmu.sphinx.frontend.CommonConfig)
     */
    public void initialize() {
        super.initialize();
    }

    /**
     * Resets the DeltasFeatureExtractor to be ready to read the next segment
     * of data.
     */
    private void reset() {

	}



    /**
     * Returns the next Data object produced by this DeltasFeatureExtractor.
     * 
     * @return the next available Data object, returns null if no Data is
     *         available
     * 
     * @throws DataProcessingException
     *                 if there is a data processing error
     */
    public Data getData() throws DataProcessingException {
            Data input = getPredecessor().getData();
            if (input != null && a!=null) {
		if (input instanceof FloatData) transfo((FloatData) input);
                if (input instanceof DataStartSignal){
		    logger.info("transfo en cours " +id);
		    ntrame=0;
	      if (false) for (int bloc =0 ;bloc<debutBloc.length; bloc++)
		   for (int ligne=0; ligne<a[bloc].length ;ligne++){
		       for (int col=0; col<a[bloc][ligne].length;col++)
			   System.err.print( a[bloc][ligne][col]+" ");
		       System.err.println(b[ligne+debutBloc[bloc]]);
		   }
		}
		
	    }
	    
	    return (input);
    }

    private void transfo( FloatData data) throws  DataProcessingException  {
	float cumul;
	float []feat=data.getValues();// it is not a copy 
        if (true) {
	if (false && ntrame <200) {
		    
		    System.out.printf("%3d",ntrame);
		    for (float f : feat) 
			System.out.printf(" %9.5f",f);
		    System.out.println();
		}
		
	if (feat.length!= featureSize)
	    throw new  DataProcessingException("mismatch "+ featureSize +"!=" +
					       feat.length );
	for(int bloc = 0; bloc <debutBloc.length; bloc++)
	    for (int ligne=0; ligne<a[bloc].length;ligne++){
		cumul=b[ligne+debutBloc[bloc]];
		for (int col=0;col<a[bloc][ligne].length;col++)
		    cumul += a[bloc][ligne][col]*feat[debutBloc[bloc]+col];
		tempo[ligne+debutBloc[bloc]]=cumul;
	    }
	ntrame++;
	for (int i =0 ; i<feat.length;i++)
	    feat[i]=tempo[i];
	}
    }

    /**
     * load transfo cmllr avec base dir of component
     * id is id of sentence
     * one directory  by speaker
     */
    public void setId(String id) throws IOException {
	setId(dumpDir,id);}
    /**
     *idem except 
     *basedir=dumpdir
     *
     */
    public void setId(String dumpDir,String id)throws IOException {
	Scanner s;
	String []temp= id.split("!");
	String[] dec= temp[0].split("-");
        StringBuffer lenom=new StringBuffer(dec[3]);
        for (int l=4 ; l< dec.length ; l++){
            lenom.append("-");
	    lenom.append(dec[l]);
        }
	if (bySent) {
	this.id=id;
	s= new Scanner(new File(new File(new File(dumpDir,dec[0]),lenom.toString()),id+".regmat"));
	}
	else
	    {
		String locId=dec[0]+"-"+lenom.toString();
		if (locId.equals(this.id)) return;
		this.id=locId;
		s= new Scanner(new File(new File(dumpDir,dec[0]),lenom.toString()+".regmat"));
	    }
	s.useLocale(Locale.US);
	String type =s.next();
	if (!type.equals("pasdiag"))
	    throw new IOException("je sais pas faire "+type);
	int nbBloc = s.nextInt();
	int []taille= new int[nbBloc];
        featureSize=0;
        for (int k=0; k<nbBloc; k++){
	    taille[k]=s.nextInt();
	    featureSize+=taille[k];
	}
	if (a==null) {
	    b= new float[featureSize];
	    tempo=new float[featureSize];
            debutBloc= new int[nbBloc];
	    debutBloc[0]=0;
            a=new float[nbBloc][][];
	    for (int k=0; k<nbBloc; k++)
		 a[k]=new float[taille[k]][taille[k]];
	    for (int k=1; k<nbBloc; k++)
		debutBloc[k]=debutBloc[k-1]+taille[k-1];
	}
	for (int bloc =0 ;bloc<nbBloc; bloc++)
	    for (int ligne=0; ligne<a[bloc].length ;ligne++){
		for (int col=0; col<a[bloc][ligne].length;col++)
		    a[bloc][ligne][col]=s.nextFloat();
		b[ligne+debutBloc[bloc]]=s.nextFloat();
	    }
	
	s.close(); 
    }
}
