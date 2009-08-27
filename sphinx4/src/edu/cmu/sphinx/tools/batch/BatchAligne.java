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

import java.net.URL;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.logging.Logger;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import edu.cmu.sphinx.result.Sausage;
import edu.cmu.sphinx.linguist.dictionary.Dictionary;
import edu.cmu.sphinx.util.Utilities;
import edu.cmu.sphinx.util.SplitNameFile;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.util.props.SetId;
import edu.cmu.sphinx.result.Aligner;
/**
 *confucius
 *
 */
public class BatchAligne implements Configurable {
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
    public final static String PROP_LATTICE_DIR = "latDir";
    /**
     * The sphinx property that specifies the feat'extension
     */
    public final static String PROP_LATTICE_DIR_DEFAULT = "latDir";
 /**
     * The sphinx property that specifies the lattice directory
     */
    public final static String PROP_LATTICE_EXT = "latExt";
    /**
     * The sphinx property that specifies the feat'extension
     */
    public final static String PROP_LATTICE_EXT_DEFAULT = "saus";

    public final static String PROP_ALIGNER = "aligner";
    /**
     * The sphinx property that specifies if ctm dum and where
     */
   /**
     * The sphinx property that specifies if ctm dum and where
     */
    public final static String PROP_STM = "stmName";
    public final static String PROP_STM_DEFAULT = null;
    public final static String PROP_SEUIL = "seuil";
    public final static float PROP_SEUIL_DEFAULT = 0.0001f;
    private String name;
    private int beginSkip;
    private int totalCount;
    private Dictionary dictionary;
    private Logger logger;
    private String latDir;
    private String latExt;
    private ConfigurationManager cm;
    private Aligner al;
    private String stmName,ctmName;
    private float seuil,sanseps,aveceps;
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
 //        registry.register(PROP_DICTIONARY, 
//                           PropertyType.COMPONENT);

        registry.register(PROP_ALIGNER, 
                          PropertyType.COMPONENT);
	registry.register(PROP_STM,PropertyType.STRING);
	registry.register("ctmName",PropertyType.STRING);
	registry.register(PROP_LATTICE_DIR, PropertyType.STRING);
	registry.register(PROP_LATTICE_EXT, PropertyType.STRING);
      	registry.register(PROP_SEUIL,PropertyType.FLOAT);
      	registry.register("sanseps",PropertyType.FLOAT);
      	registry.register("aveceps",PropertyType.FLOAT);
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
        if (totalCount <= 0) {
            totalCount = Integer.MAX_VALUE;
        }
        al = (Aligner) ps.getComponent(PROP_ALIGNER,
						  Aligner.class);
	stmName = ps.getString(PROP_STM,PROP_STM_DEFAULT);
	ctmName = ps.getString("ctmName",null);
  //       dictionary =(Dictionary)  ps.getComponent
//             (PROP_DICTIONARY, Dictionary.class);


        latDir = ps.getString(PROP_LATTICE_DIR,PROP_LATTICE_DIR_DEFAULT);
        latExt = ps.getString(PROP_LATTICE_EXT,PROP_LATTICE_EXT_DEFAULT);
	seuil  =ps.getFloat(PROP_SEUIL,PROP_SEUIL_DEFAULT);
	sanseps  =ps.getFloat("sanseps",seuil);
	aveceps  =ps.getFloat("aveceps",seuil);
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
    public void decode(String batchFile) {
	PrintStream stmWriter=null,ctmWriter=System.out;
	
	if (stmName!= null) 
	    try {
		stmWriter =new PrintStream(//new BufferedWriter(new OutputStreamWriter(new FileOutputStream(
					   stmName,
										     "ISO8859_1");
        	
	    } catch (IOException ioe) {
		System.err.println("IO erreur durant stm file" + stmName +" "+ ioe);
	    }
	if (ctmName!= null) 
	    try {
		ctmWriter =new PrintStream(ctmName,"ISO8859_1");
        	
	    } catch (IOException ioe) {
		System.err.println("IO erreur durant stm file" + ctmName +" "+ ioe);
	    }
	int count = 0 ;
	List <String> list = new ArrayList<String> ();
	try {
	BufferedReader reader = new BufferedReader(new FileReader(batchFile));
	
	String line = null;
        while ((beginSkip>0) &&  ((line = reader.readLine()) != null))
	    if (line.length() > 0) beginSkip--;
	while ((line = reader.readLine()) != null && count++ <totalCount ) {
            if (line.length() > 0) {
		list.add(line);
	    }
	}
    
	reader.close();
	  } catch (IOException ioe) {
		System.err.println("IO erreur durant ctl file" + batchFile +" "+ ioe);
	    }
	try {
	    Pattern an=Pattern.compile("(.*)\\s+\\((\\S+?)\\)$");
	    //	    dictionary.allocate();
            logger.info("BatchDecoder: decoding files in "
			+ batchFile + count+ " totalCount " + totalCount);
	    for(String l: list){
		Matcher m=an.matcher(l);
		if(!m.find()) throw new Error(l);
		Sausage saus= new Sausage(SplitNameFile.splitNameFile(latDir,m.group(2),latExt),
					  seuil,aveceps,sanseps);// faudra changer cela
		if (stmWriter!=null)	stmWriter.println(m.group(2) +" "+al.aligner(m.group(1),saus,m.group(2),ctmWriter));
		else al.aligner(m.group(1),saus,m.group(2),ctmWriter);
	    }
	    if (stmWriter !=null)	    stmWriter.close();
	    if (ctmWriter !=null)	    ctmWriter.close();
	}
	catch (IOException io) {
            logger.warning("I/O error during decoding: " + io.getMessage());
	    
	}
    }

    
    
    /**
     * Main method of this BatchDecoder.
     * 
     * @param argv
     *                argv[0] : config.xml argv[1] : a file listing
     *                all the audio files to decode
     */
    public static void main(String[] argv) {
        if (argv.length < 2) {
            System.out.println(
                    "Usage: BatchDecoder propertiesFile batchFile [-shell]");
            System.exit(1);
        }
        String cmFile = argv[0];
        String batchFile = argv[1];
        ConfigurationManager cm;
        BatchAligne bmr = null;
      

        //BatchModeRecognizer recognizer;
        try {
            URL url = new File(cmFile).toURI().toURL();
            cm = new ConfigurationManager(url);
            bmr = (BatchAligne) cm.lookup("aligne");
	   
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
            System.err.println("Can't find batchConfus in " + cmFile);
            return;
        }
	System.gc();
	bmr.decode(batchFile);
        
    }
}
