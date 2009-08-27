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
package edu.cmu.sphinx.tools.batch;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.io.BufferedReader;
import java.io.FileReader;
import edu.paul.majuscule.Majuscule;
import java.net.URL;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.logging.Logger;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import edu.cmu.sphinx.util.Utilities;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
/**
 *Confucius
 *
 */
public class BatchMajuscule implements Configurable {
     /**
     * The SphinxProperty name for how many files to skip at begin.
     */
    public final static String PROP_BEGIN_SKIP = "beginSkip";

    /**
     * The default value for the property PROP__BEGIN_SKIP.
     */
    public final static int PROP_BEGIN_SKIP_DEFAULT = 0;
  
    /**
     * The SphinxProperty name for how many utterances to process
     */
    public final static String PROP_COUNT = "count";
    /**
     * The default value for the property PROP_COUNT.
     */
    public final static int PROP_COUNT_DEFAULT = 1000000;

    /**
     * The sphinx property that Dictionary
     */
    public final static String PROP_DICTIONARY = "dictionary";
    /**
     * The sphinx property that specifies the lattice directory
     */
    private String name;
    private int beginSkip;
    private int totalCount;
    private Logger logger;
    private ConfigurationManager cm;
    private String ctmIn,ctmOut;
    private Majuscule majuscule;
    /**
 
    // -------------------------------
    // Configuration data
    // --------------------------------
   
    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
            throws PropertyException {
        this.name = name;
        registry.register(PROP_BEGIN_SKIP, PropertyType.INT);
        registry.register(PROP_COUNT, PropertyType.INT);
         registry.register("majuscule", 
                    PropertyType.COMPONENT);
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
     
        cm = ps.getPropertyManager();
        beginSkip = ps.getInt(PROP_BEGIN_SKIP, PROP_BEGIN_SKIP_DEFAULT);
        totalCount = ps.getInt(PROP_COUNT, PROP_COUNT_DEFAULT);
	majuscule=(Majuscule) ps.getComponent("majuscule",Majuscule.class);
        if (totalCount <= 0) {
            totalCount = Integer.MAX_VALUE;
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

    /**
     * Sets the batch file to use for this recogition
     * 
     * @param batchFile
     *                the name of the batch file
     * @throws IOException
     *                 if the file could not be opened or read.
     */

    /**
     * Decodes the batch of audio files
     * 
     * @throws IOException
     *                 if there is an I/O error processing the batch file
     */
    public void decode(String ctmIn,String ctmOut) throws IOException {
	PrintStream ctmWriter=System.out;
	
	majuscule.allocate();
	if (ctmOut!= null) 
	    try {
		ctmWriter =new PrintStream(ctmOut,"ISO8859_1");
        	
	    } catch (IOException ioe) {
		System.err.println("IO erreur durant ctm file" + ctmOut +" "+ ioe);
	    }
	int count = 0 ;
	ArrayList <String[]> list = new ArrayList<String[]> ();
	Pattern m=Pattern.compile("\\s+");
	String fichier="";
	try {
	BufferedReader reader = new BufferedReader(new FileReader(ctmIn));
	
	String line = null;
        while ((beginSkip>0) &&  ((line = reader.readLine()) != null))
	    if (line.length() > 0) beginSkip--;
	while ((line = reader.readLine()) != null && count++ <totalCount ) {
            if (line.length() > 0) {
		String temp[]=m.split(line);
		if (! fichier.equals(temp[0])){
		    if (list.size() >0)
			majuscule.traiter(list,ctmWriter);
		    list.clear();
		    logger.info(fichier + "traite");
		    fichier=temp[0];
		}
		list.add(temp);
	    }
	}
	if (list.size() >0)
	    majuscule.traiter(list,ctmWriter);
	list.clear();
	logger.info(fichier + "traite");
	
    
	reader.close();
	ctmWriter.close();
	majuscule.deallocate();
	} catch (IOException ioe) {
	    System.err.println("IO erreur durant ctl file" + ctmIn +" "+ ioe);
	}

    }

    
    
    /**
     * Main method of this BatchDecoder.
     * 
     * @param argv
     *                argv[0] : config.xml argv[1] : a file listing
     *                all the audio files to decode
     */
    public static void main(String[] argv) throws IOException {
        if (argv.length < 3) {
            System.out.println(
                    "Usage: BatchDecoder propertiesFile ctmIn ctmOut [-shell]");
            System.exit(1);
        }
        String cmFile = argv[0];
        String ctmIn = argv[1];
        String ctmOut = argv[2];
        ConfigurationManager cm;
        BatchMajuscule bmr = null;
      

        //BatchModeRecognizer recognizer;
        try {
            URL url = new File(cmFile).toURI().toURL();
            cm = new ConfigurationManager(url);
            bmr = (BatchMajuscule) cm.lookup("metMajuscule");
	   
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
        System.err.println("sortie de run ");
        if (bmr == null) {
            System.err.println("Can't find metMajuscule" + cmFile);
            return;
        }
	System.gc();
       
	bmr.decode(ctmIn,ctmOut);
        
    }
}
