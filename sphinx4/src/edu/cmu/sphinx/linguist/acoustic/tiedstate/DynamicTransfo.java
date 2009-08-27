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


package edu.cmu.sphinx.linguist.acoustic.tiedstate;

import java.io.IOException;
import java.util.Scanner;
import java.util.Locale;
import java.io.File;
public class DynamicTransfo {
    private Loader loader;
    private String dumpDir;
    private Boolean isNotLoaded=true;
    private Pool meansTransformationVectorPool;
    private Pool meansTransformationMatrixPool;
    private Pool senonePool;
    private String id;
    private boolean bySent=false;
    private boolean byShow =false;
    private DynamicTransfo() {};
    public DynamicTransfo(Loader loader, String dumpDir) {
	
	this.loader=loader;
	this.dumpDir=dumpDir;
    }
    public DynamicTransfo(Loader loader, String dumpDir, boolean byShow) {
	this.byShow= byShow;
	this.loader=loader;
	this.dumpDir=dumpDir;
    }



    private void load() {
	isNotLoaded=false;
	meansTransformationVectorPool=loader.getMeansTransformationVectorPool();
	meansTransformationMatrixPool=loader.getMeansTransformationMatrixPool();
	senonePool= loader.getSenonePool();
    }
    public void setId(String id) throws IOException {
	if (isNotLoaded) load();
	Scanner s;
	String[] temp=id.split("!");
	
        String[] dec= temp[0].split("-");
        StringBuffer lenom;
	
	new StringBuffer(dec[3]);
        if (byShow) {
	    lenom= new StringBuffer(dec[3]);
	    
	    if (lenom.charAt(1)!='0' && lenom.charAt(1)!='2')
		lenom.setCharAt(1,'0');}
	else {
	    if (temp.length>1)
		lenom= new StringBuffer(temp[1]);
	    else {
		lenom= new StringBuffer(dec[3]);	
		for (int l=4 ; l< dec.length ; l++){
		    lenom.append("-");
		    lenom.append(dec[l]);
		}
	    }
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
		System.err.println(" lire : " + dec[0] +" " + lenom.toString());
	    }

	s.useLocale(Locale.US);
	int nbClasse=s.nextInt();
        if (nbClasse !=meansTransformationVectorPool.size()) 
	    new Error( "nbclasse " + nbClasse+ " " + meansTransformationVectorPool.size());
	int nbFeat=s.nextInt();
	assert(nbFeat==1);
	for (int i =0 ; i<nbClasse ;i++) {
	    float [][] mat =(float [][] ) meansTransformationMatrixPool.get(i);
	    float [] vect =(float[]) meansTransformationVectorPool.get(i);
	    int veclen = s.nextInt();
	    assert(veclen== mat.length);
	    for (int lig=0; lig<veclen; lig++)
		for (int col=0 ; col<veclen; col++)
		    mat[lig][col] = s.nextFloat();
	    	for (int col=0 ; col<veclen; col++)
		    vect[col]=s.nextFloat();
	}
	for (int i=0; i<senonePool.size(); i++)
	    ((Senone)  senonePool.get(i)).resetTransfo();
	s.close();
    }
}
