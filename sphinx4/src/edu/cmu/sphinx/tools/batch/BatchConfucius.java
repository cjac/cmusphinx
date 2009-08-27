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
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.BufferedWriter;
import java.io.PrintWriter;
import java.net.URL;
import java.util.Iterator;
import java.util.List;
import java.util.logging.Logger;
import edu.paul.Confidence;
import edu.cmu.sphinx.result.ConfidenceScorer;
import edu.cmu.sphinx.result.Sausage;
import edu.cmu.sphinx.result.SausageMakerFast;
import edu.cmu.sphinx.result.SausageMakerEdges;
import edu.cmu.sphinx.result.Lattice;
import edu.cmu.sphinx.linguist.dictionary.Dictionary;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.util.BatchItem;
import edu.cmu.sphinx.util.BatchManager;
import edu.cmu.sphinx.util.PooledBatchManager;
import edu.cmu.sphinx.util.SimpleBatchManager;
import edu.cmu.sphinx.util.Utilities;
import edu.cmu.sphinx.util.SplitNameFile;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.util.props.SetId;
/**
 *confucius
 *
 */
public class BatchConfucius implements Configurable {

    /**
     * The SphinxProperty name for how many files to skip for every decode.
     */
    public final static String PROP_SKIP = "skip";

    /**
     * The default value for the property PROP_SKIP.
     */
    public final static int PROP_SKIP_DEFAULT = 0;
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
     * The SphinxProperty that specified which batch job is to be run.
     *  
     */
    public final static String PROP_WHICH_BATCH = "whichBatch";

    /**
     * The default value for the property PROP_WHICH_BATCH.
     */
    public final static int PROP_WHICH_BATCH_DEFAULT = 0;
    
    /**
     * The SphinxProperty for the total number of batch jobs the decoding run
     * is being divided into.
     * 
     * The BatchDecoder supports running a subset of a batch. This allows a
     * test to be distributed among several machines.
     *  
     */
    public final static String PROP_TOTAL_BATCHES = "totalBatches";

    /**
     * The default value for the property PROP_TOTAL_BATCHES.
     */
    public final static int PROP_TOTAL_BATCHES_DEFAULT = 1;

    /**
     * The SphinxProperty that defines whether or not the decoder should use
     * the pooled batch manager
     */
    public final static String PROP_USE_POOLED_BATCH_MANAGER = "usePooledBatchManager";

    /**
     * The default value for the property PROP_USE_POOLED_BATCH_MANAGER.
     */
    public final static boolean PROP_USE_POOLED_BATCH_MANAGER_DEFAULT = false;

    /**
     * The Sphinx property that specifies logmath
     */
    public final static String PROP_LOGMATH = "logMath";

    /**
     * The sphinx property that Dictionary
     */
    public final static String PROP_DICTIONARY = "dictionary";
    /**
     * The sphinx property that specifies the lattice directory
     */
    public final static String PROP_LATTICE_DIR = "latDir";
    public final static String PROP_LATTICE_DIR_DEFAULT = "latdir";

    /**
     * The sphinx property that specifies the FSM directory
     */
    public final static String PROP_FSM_DIR = "fsmDir";
    /**
     * The sphinx property that default fsm dir
     */
    public final static String PROP_FSM_DIR_DEFAULT = "fsmdir";

    public final static String PROP_SAUSSICE = "saussice";
    //public final static String PROP_PROPORTION_EDGES = "edge";
    /*
     * The sphinx property that specifies the feat'extension
     */
    // public final int PROP_PROPORTION_EDGES_DEFAULT = 10;
    /*
     * The sphinx property that specifies if ctm dum and where
     */
    // -------------------------------
    // Configuration data
    // --------------------------------
    private String name;
    private int skip;
    private int beginSkip;
    private int totalCount;
    private int whichBatch;
    private int totalBatches;
    private boolean usePooledBatchManager;
    private BatchManager batchManager;
    private LogMath logMath;
    private Dictionary dictionary;
    private Logger logger;
    private String latDir;
    private BatchItem curBatchItem;
    private ConfigurationManager cm;
    private ConfidenceScorer sm;
    private String fsmDir;
    //private float pedge;
    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String,
     *      edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry)
            throws PropertyException {
        this.name = name;
        registry.register(PROP_SKIP, PropertyType.INT);
        registry.register(PROP_BEGIN_SKIP, PropertyType.INT);
        registry.register(PROP_COUNT, PropertyType.INT);
        registry.register(PROP_WHICH_BATCH, PropertyType.INT);
        registry.register(PROP_TOTAL_BATCHES, PropertyType.INT);
        registry.register(PROP_USE_POOLED_BATCH_MANAGER, PropertyType.BOOLEAN);
        registry.register(PROP_LOGMATH, PropertyType.COMPONENT);
	registry.register(PROP_SAUSSICE, PropertyType.COMPONENT);
        registry.register(PROP_DICTIONARY, 
                          PropertyType.COMPONENT);

	registry.register(PROP_LATTICE_DIR, PropertyType.STRING);
	registry.register(PROP_FSM_DIR, PropertyType.STRING);
	//	registry.register(PROP_PROPORTION_EDGES,PropertyType.FLOAT);
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
     
        cm = ps.getPropertyManager();
        skip = ps.getInt(PROP_SKIP, PROP_SKIP_DEFAULT);
        beginSkip = ps.getInt(PROP_BEGIN_SKIP, PROP_BEGIN_SKIP_DEFAULT);
        totalCount = ps.getInt(PROP_COUNT, PROP_COUNT_DEFAULT);
        if (totalCount <= 0) {
            totalCount = Integer.MAX_VALUE;
        }
        whichBatch = ps.getInt(PROP_WHICH_BATCH, PROP_WHICH_BATCH_DEFAULT);
        totalBatches = ps
	    .getInt(PROP_TOTAL_BATCHES, PROP_TOTAL_BATCHES_DEFAULT);
        usePooledBatchManager = ps.getBoolean(PROP_USE_POOLED_BATCH_MANAGER,
					      PROP_USE_POOLED_BATCH_MANAGER_DEFAULT);
        logMath = (LogMath) ps.getComponent(PROP_LOGMATH,
						  LogMath.class);
        sm = (ConfidenceScorer) ps.getComponent(PROP_SAUSSICE,
						  ConfidenceScorer.class);

        dictionary =(Dictionary)  ps.getComponent
            (PROP_DICTIONARY, Dictionary.class);
        latDir = ps.getString(PROP_LATTICE_DIR,PROP_LATTICE_DIR_DEFAULT);
        fsmDir = ps.getString(PROP_FSM_DIR,PROP_FSM_DIR_DEFAULT);
	//	pedge  =ps.getFloat(PROP_PROPORTION_EDGES,PROP_PROPORTION_EDGES_DEFAULT);
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
    public void setBatchFile(String batchFile) throws IOException {
        if (usePooledBatchManager) {
            batchManager = new PooledBatchManager(batchFile, skip,true);
        } else {
            batchManager = new SimpleBatchManager(batchFile, skip, whichBatch,
						  totalBatches,true,beginSkip);
        }
    }

    /**
     * Decodes the batch of audio files
     * 
     * @throws IOException
     *                 if there is an I/O error processing the batch file
     */
    public void decode(String batchFile) {
        BatchItem batchItem;

        int count = 0;
  //       if (ctmName!= null) 
	//         	try {
	//         		ctmWriter =new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(ctmName),
	//         				"ISO8859_1")));
        	
//         	} catch (IOException ioe) {
//         		System.err.println("IO erreur durant ctm file" + ctmName +" "+ ioe);
//         		return;
//         	}

    
        try {
	    dictionary.allocate();
	    setBatchFile(batchFile);
	    
            batchManager.start();
            logger.info("BatchDecoder: decoding files in "
			+ batchManager.getFilename() + count+ " totalCount " + totalCount);
	    
            while (count < totalCount && 
		   (batchItem = batchManager.getNextItem()) != null) {
                Lattice lat=new Lattice(logMath,dictionary,SplitNameFile.splitNameFile(latDir,batchItem.getId(),".lat.gz"));
                //lat.computeNodePosteriors(10.0f,false,pedge);// false with lm
		//		SausageMakerFast sm = new SausageMakerFast(lat);
		//SausageMakerEdges sm = new SausageMakerEdges();
		Sausage s = (Sausage) sm.score(lat);
		System.err.println(s.getBestHypothesisString());
		//s.dumpAISee("mapSausage.gdl"+count, "MAP Sausage");
		s.dumpFSM(SplitNameFile.splitNameFile(fsmDir,batchItem.getId(),"saus",true),
			  logMath.linearToLog(1e-4),
			  logMath.linearToLog(1e-4));

		logger.info("File  : " + batchItem.getId());
                count++;
	    }
	    System.gc();
	
	    batchManager.stop();
	} catch (IOException io) {
            logger.severe("I/O error during decoding: " + io.getMessage());
	    
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
        BatchConfucius bmr = null;
      

        //BatchModeRecognizer recognizer;
        try {
            URL url = new File(cmFile).toURI().toURL();
            cm = new ConfigurationManager(url);
            bmr = (BatchConfucius) cm.lookup("confucius");
	   
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
