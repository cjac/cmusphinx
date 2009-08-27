
package edu.cmu.sphinx.util;
import  java.io.File;


public class  SplitNameFile {
public static final  File splitNameFile(String base,String id,String ext) {
	return splitNameFile(base,id,ext,false);
    }
public static    File splitNameFile(String base,String id,String ext,boolean ecrire){
	String[] dec= id.split("-");
        StringBuffer lenom=new StringBuffer(dec[3]);
        for (int l=4 ; l< dec.length ; l++){
            lenom.append("-");
            lenom.append(dec[l]);
        }
        if (ecrire){
	    if ((new File(base)).isDirectory() ){
		File rep=  new File(base,dec[0]);
		if (!rep.isDirectory()) rep.mkdirs();
	    }
	    else throw new Error(base+" n' est pas un repertoire");
	}
	return new File(new File(base,dec[0]),id+ext);
	
    }



}
