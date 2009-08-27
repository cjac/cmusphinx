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
package edu.paul.question;
import java.util.logging.Logger;
import java.util.Properties;
import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.BufferedWriter;
import java.io.PrintWriter;
import java.util.Scanner;
import java.util.Locale;
import java.net.URL;
import java.util.Locale;
import java.util.Iterator;
import java.util.Iterator;
import java.util.List;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.logging.Logger;
import edu.cmu.sphinx.util.SplitNameFile;
import edu.cmu.sphinx.util.Utilities;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.util.ExtendedStreamTokenizer;





public class Stat implements Configurable {
     /**
     * The Sphinx property that specifies the recognizer to use
     */
    public final static String PROP_ACOUSTIC_MODEL = "acousticModel";

    
    /**
     * The sphinx property that specifies if ctm dum and where
     */
     /**
     * the fsm grammar if saucussice decoder
     */
    // Configuration data
    // --------------------------------
    private String name;
    private Logger logger;
    private ConfigurationManager cm;
    private LogMath logMath;
    private PrintWriter fileidWriter;
    private PetitLoader loader;
    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
            throws PropertyException {
        this.name = name;
 	registry.register("logMath", PropertyType.COMPONENT);
 	registry.register("loader", PropertyType.COMPONENT);


    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
        cm = ps.getPropertyManager();
	logMath = (LogMath) ps.getComponent("logMath", LogMath.class);
	loader = (PetitLoader) ps.getComponent("loader", PetitLoader.class);
   
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#getName()
     */
    public String getName() {
        return name;
    }
    void reverse( float [] a) {
	float temp;
	for (int i =0 ; i<a.length/2 ; i++) {
	    temp=a[i];
	    a[i]=a[a.length-i-1];
	    a[a.length-i-1]=temp;
	}
    }
	    
    void tri(float [][] p) {
	for (float [] a :p){ 
	    Arrays.sort(a);
	    reverse(a);
	}
    }
    void normalise(float [] poids) {
	float s=0.00000000001f;
	for (float a :poids) 
	    s+=a;
	for (int i =0 ; i<poids.length ; i++) 
	    poids[i] /=s;
    }
    float [] somme (float [][] poids) {
	float [] cumul= new float[poids[0].length];
	for ( float []a : poids) 
	    for (int i =0 ; i<a.length ; i++)
		cumul[i] += a[i];
	return cumul;
    }
    void print (float [] a) {
	normalise(a);
	for (float f:a) 
	    System.out.format(Locale.US," %6.4f",10*f);
	System.out.println();
    }
    void normalise(float [][]poids){
	for (float [] a :poids) {
	    normalise(a);
	}
    }
    void stat(float[][] poids) {
	normalise(poids);
	print(somme(poids));
	tri(poids);
	print(somme(poids));
    }

    public void faire(String [] argv) {
	float [][] mixture;
	List <Etat> lesEtats;
	ClassePhoneme classe;
	try {
	    mixture=loader.loadMixturePoids(argv[1]);

	}
	catch (IOException e)
	    {logger.warning(e.toString() + ":loader pas charge");
		return;
	    }
	stat(mixture);
    }
	    
    /**
     * Main method of this BatchBW.
     * 
     * @param argv
     *                argv[0] : config.xml argv[1] : a file listing
     *                all the audio files to decode
     */
    public static void main(String[] argv) {
        if (argv.length < 2) {
            System.out.println(
                    "Usage: BatchDecoder propertiesFile   mixture  ");
            System.exit(1);
        }
        String cmFile = argv[0];
        ConfigurationManager cm;
        Stat bmr = null;
      

        //BatchModeRecognizer recognizer;
        try {
            URL url = new File(cmFile).toURI().toURL();
            cm = new ConfigurationManager(url);
            bmr = (Stat) cm.lookup("batch");
	   
        } catch (IOException ioe) {
            System.err.println("I/O error during initialization: \n   " + ioe);
            return;
        } catch (InstantiationException e) {
            System.err.println("Error during initialization: \n  " + e);
            return;
        } catch (PropertyException e) {
            System.err.println("Error during initialization: \n  " + e);
            return;
        }

	try { 
	    edu.cmu.sphinx.instrumentation.ConfigMonitor config=
		(edu.cmu.sphinx.instrumentation.ConfigMonitor)cm.lookup("configMonitor");
	    config.run();// peut etre faut-il faire un thread
	}
	catch (InstantiationException e) {
            System.err.println("Error during config: \n  " + e);
            //return;
        } catch (PropertyException e) {
            System.err.println("Error during config: \n  " + e);
	    //   return;
        }

        if (bmr == null) {
            System.err.println("Can't find batchModeRecognizer in " + cmFile);
            return;
        }



	System.gc();
	bmr.faire(argv);
        
    }




}

