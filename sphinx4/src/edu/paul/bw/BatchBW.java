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
package edu.paul.bw;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.BufferedWriter;
import java.io.PrintWriter;
import java.net.URL;
import java.util.Locale;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.logging.Logger;
import edu.cmu.sphinx.util.SplitNameFile;
import edu.cmu.sphinx.frontend.DataProcessor;
import edu.cmu.sphinx.frontend.Data;
import edu.cmu.sphinx.frontend.FloatData;
import edu.cmu.sphinx.frontend.DataEndSignal;
import edu.cmu.sphinx.frontend.DataProcessingException;
import edu.cmu.sphinx.frontend.DataStartSignal;
import edu.cmu.sphinx.frontend.FrontEnd;
import edu.cmu.sphinx.frontend.util.StreamCepstrumSource;
import edu.cmu.sphinx.frontend.util.StreamDataSource;
import edu.cmu.sphinx.util.BatchItem;
import edu.cmu.sphinx.util.BatchManager;
import edu.cmu.sphinx.util.PooledBatchManager;
import edu.cmu.sphinx.util.SimpleBatchManager;
import edu.cmu.sphinx.util.Utilities;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.util.props.SetId;
import edu.cmu.sphinx.util.SplitNameFile;

//import edu.cmu.sphinx.result.Lattice;
/**
 * Decodes a ctl file containing a list of files(format)
 * sphinx3  to decode. The files can be
 * either audio files or cepstral files, but defaults to audio files.
 * The audio data should be 16-bit, 16kHz, PCM-linear data.
 * Since this classes makes use of Java Sound, it supports all the audio file 
 * formats that are supported by Java Sound. If the audio file does not
 * correspond to a format supported by Java Sound, it is treated as a raw
 * audio file (i.e., one without a header). Audio file formats differ
 * in the endian order of the audio data. Therefore, it is important to
 * specify it correctly in the configuration of the
 * <a href="../../frontend/util/StreamDataSource.html">StreamDataSource</a>.
 * Note that in the ideal situation, the audio format of the data should
 * be passed into the StreamDataSource, so that no extra configuration is
 * needed. This will be fixed in future releases.
 * <p>
 * To run this BatchModeRecognizer:
 * <pre>
 * java BatchCtlModeRecognizer &lt;xmlConfigFile&gt; &lt;batchFile&gt;
 * </pre>
 * where <code>xmlConfigFile</code> is an XML-based configuration file and
 * <code>batchFile</code> is a file listing all the files to decode and
 * transcript of those files. For information about the configuration
 * file, refer to the document 
 * <a href="../../util/props/doc-files/ConfigurationManagement.html">
 * Sphinx-4 Configuration Management</a>. For information about the
 * batch file, refer to the <a href="../../../../../../index.html#batch_files">
 * batch file description</a>.
 * 
 * This class will send recognition results to the logger if the log level is
 * set to INFO.
 *
 */
public class BatchBW implements Configurable {

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
     * The Sphinx property that specifies the recognizer to use
     */
    public final static String PROP_ACOUSTIC_MODEL = "acousticModel";

    /**
     * The sphinx property that specifies the input source
     */
    public final static String PROP_INPUT_DATA_PROCESSORS = "inputDataProcessors";
    /**
     * The sphinx property that specifies the feat'directory
     */
    public final static String PROP_SIGNAL_DIR = "featDir";
    /**
     * The sphinx property that specifies the feat'extension
     */
    public final static String PROP_SIGNAL_DIR_DEFAULT = "feat_bl";
    public final static String PROP_SIGNAL_EXT = "featExt";
    public final static String PROP_SIGNAL_EXT_DEFAULT = "feat_bl";
    public final static String PROP_LAT_DIR = "latDir";
    public final static String PROP_PHRASE_DIR = "phraseDir";
    
    /**
     * The sphinx property that specifies if ctm dum and where
     */
     /**
     * the fsm grammar if saucussice decoder
     */
    public final static String PROP_LATTICE = "lattice";
    public final static String PROP_FILEID = "fileid";
    public final static String PROP_FB = "FB";
    public final static String PROP_SAVER = "saver";
    public final static String PROP_FRONTEND = "frontEnd";
    // -------------------------------
    // Configuration data
    // --------------------------------
    private String name;
    private List inputDataProcessors;
    private int skip;
    private int beginSkip;
    private int totalCount;
    private int whichBatch;
    private int totalBatches;
    private boolean usePooledBatchManager;
    private BatchManager batchManager;
    private Logger logger;
    private String feat_dir;
    private String feat_ext;
    private BatchItem curBatchItem;
    private ConfigurationManager cm;
    private Lattice lattice;
    private FB fb;
    private Saver saver;
    private String latDir;
    private String phraseDir;
    private FrontEnd frontEnd;
    private SentenceHMMState initState,finalState;
    private LogMath logMath;
    private SetId cmllr;
    private String fileid;
    private PrintWriter fileidWriter;
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
	registry.register(PROP_INPUT_DATA_PROCESSORS, 
                          PropertyType.COMPONENT_LIST);
	registry.register(PROP_FILEID, PropertyType.STRING);
	registry.register(PROP_LAT_DIR, PropertyType.STRING);
	registry.register(PROP_PHRASE_DIR, PropertyType.STRING);
	registry.register(PROP_SIGNAL_DIR, PropertyType.STRING);
	registry.register(PROP_SIGNAL_EXT, PropertyType.STRING);
        registry.register(PROP_LATTICE, PropertyType.COMPONENT);
        registry.register(PROP_FB, PropertyType.COMPONENT);
        registry.register(PROP_SAVER, PropertyType.COMPONENT);
	registry.register(PROP_FRONTEND, PropertyType.COMPONENT);
	registry.register("logMath", PropertyType.COMPONENT);
	registry.register("cmllr", PropertyType.COMPONENT);
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
        
        inputDataProcessors = (List) ps.getComponentList
            (PROP_INPUT_DATA_PROCESSORS, DataProcessor.class);
        latDir = ps.getString(PROP_LAT_DIR,"latDir");
        phraseDir = ps.getString(PROP_PHRASE_DIR,null);
        fileid = ps.getString(PROP_FILEID,null);
        feat_dir = ps.getString(PROP_SIGNAL_DIR,PROP_SIGNAL_DIR_DEFAULT);
        feat_ext = ps.getString(PROP_SIGNAL_EXT,PROP_SIGNAL_EXT_DEFAULT);
	lattice  = (Lattice) ps.getComponent(PROP_LATTICE,Lattice.class);
	fb  = (FB) ps.getComponent(PROP_FB,FB.class);
	saver  = (Saver) ps.getComponent(PROP_SAVER,Saver.class);
	frontEnd = (FrontEnd) ps.getComponent(PROP_FRONTEND, FrontEnd.class);
	try {
	    cmllr = (SetId) ps.getComponent("cmllr", SetId.class);}
	catch (Exception e) 
	    { cmllr=null;logger.warning(e.toString());}
	
	logMath = (LogMath) ps.getComponent("logMath", LogMath.class);
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
	double totalAlphaPrime=0.0; int totalCountPho=0;
        try {
            lattice.allocate();
            setBatchFile(batchFile);
            batchManager.start();
            logger.info("BatchDecoder: decoding files in "
			+ batchManager.getFilename());
	} catch (IOException io) {
	    io.printStackTrace();
            logger.severe("I/O error during decoding3: " + io.getMessage());
        };
	if (fileid!=null) 
	    try { fileidWriter = new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(fileid),
											   "ISO8859_1")));
	    }
	    catch (IOException ioe) {
        		logger.severe("IO erreur durant fileid file" + fileid +" "+ ioe);
        		return;
	    }

	try {
	    while (count < totalCount && 
		   (batchItem = batchManager.getNextItem()) != null) {
		try {
		    count++;
		    logger.info("File  : " + batchItem.getId());
		    if (cmllr !=null) cmllr.setId(batchItem.getId());
		    setInputStream(batchItem.getFilename(),batchItem.getStartSentence(),batchItem.getEndSentence());
		    if (phraseDir!=null)
			initState=lattice.load(SplitNameFile.splitNameFile(latDir,batchItem.getId(),".lat.gz"),
					       SplitNameFile.splitNameFile(phraseDir,batchItem.getId(),".lat.gz"));
		    //mpe with FPPone and bwaccumdirM et bwaccumdirP 
		    else
			initState=lattice.load(SplitNameFile.splitNameFile(latDir,batchItem.getId(),".lat.gz"));
		    //mmie 
		    if (initState==null) {
			logger.warning("non loader "+ SplitNameFile.splitNameFile(latDir,batchItem.getId(),".lat.gz"));
			continue;
		    }
		    Data []data = loadData();
		    if (data==null) {
			logger.warning("pb data load"+  batchItem.getId());
			continue;}
		    finalState=fb.doForward(initState,data);
		    
		    if (finalState==null){
			logger.warning("pb forward "+ batchItem.getId());
			continue;
		    }
		    float vrais=fb.vrais(finalState,data.length-1);
		    logger.info("vrais:" +( vrais/data.length) + " "+ vrais+ " " + logMath.logToLinear(vrais)+ "trame" + data.length);
		    if (fileid==null && batchItem.getTranscript()!=null)
			{if (!fb.doBackward(initState,finalState,data,batchItem.getTranscript()))
				logger.warning("pb bacward"+ batchItem.getId()+ " tr:"+ batchItem.getTranscript());
			    else if (phraseDir !=null)
				logger.info("phoneinfo " + fb.getAlphaPrime() + " " + lattice.getCountPho());}
		    else
			if (!fb.doBackward(initState,finalState,data))
			    logger.warning("pb bacward"+ batchItem.getId());
			else if (phraseDir !=null)
			    {totalAlphaPrime+= fb.getAlphaPrime();
			    totalCountPho += lattice.getCountPho();
			    logger.info("phoneinfo " + fb.getAlphaPrime() + " " + lattice.getCountPho());
			    }
		    if (fileidWriter !=null) 
			fileidWriter.format(Locale.US,"%s %d %d %s %.10e\n",batchItem.getFilename(),
					    batchItem.getStartSentence(), batchItem.getEndSentence(), batchItem.getId(),vrais);
		 
		    System.gc();
		}   
		catch (IOException io) {
		    io.printStackTrace();
		    logger.severe("I/O error during decoding2: " + io.getMessage());
		};
	    }
	    batchManager.stop();
	    if (fileidWriter!=null) fileidWriter.close();
	}
	catch (IOException io) {
	    io.printStackTrace();
	    logger.severe("I/O error during decoding1: " + io.getMessage());
	}	    
	
	logger.info("fini bw");
	try {
	    fb.save(saver);
	    logger.info("BW: " + count + " filespasse "+ totalAlphaPrime+ " AlphaPrime "+  totalCountPho +" countPho");
        } catch (IOException io) {
            logger.severe("I/O error saving " + io.getMessage());
        };

    }
    private Data [] loadData() {
	ArrayList<Data> d= new ArrayList<Data>(1000);
	try  {
	    do {Data input= frontEnd.getData();
		if (input !=null) {
		    if (input instanceof FloatData) {
			d.add(input);
		    }
		    else if (input instanceof DataEndSignal)
			break;
		    else if (input instanceof DataStartSignal)
			assert (d.size()==0);
		    else return null;
		}
		else return null;
	    } while (true);
	}
	catch  (DataProcessingException dpe) {
            logger.warning(dpe.toString());
	}
	if (d.size()>=10)
	    return d.toArray(new Data[0]);
	return null;
    }
	    
    
    /**
     * Sets the input stream to the given filename
     * @param filename the filename to set the input stream to
     * @return the InputStream representing the filename
     * @throws IOException if an error occurs
     */
    private void setInputStream(String filename, int startSentence, int endSentence ) throws IOException {
		filename = feat_dir+ "/" + filename + "." +feat_ext;
        for (Iterator i = inputDataProcessors.iterator(); i.hasNext(); ) {
            DataProcessor dataSource = (DataProcessor) i.next();
            InputStream is = null;
	    File file = new File(filename);
	    logger.fine
                    ("Reading " + filename + " as raw audio file.");
                is = new FileInputStream( filename);
            
		if (dataSource instanceof StreamCepstrumSource) {
		    boolean isBigEndian = Utilities
			.isCepstraFileBigEndian(filename);
		    StreamCepstrumSource cepstrumSource =
			(StreamCepstrumSource) dataSource;
		    cepstrumSource.setInputStream(is, isBigEndian,startSentence,endSentence);
		    logger.fine("read between "+ startSentence + " "+ endSentence);
		}
        }
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
                    "Usage: BatchDecoder propertiesFile batchFile [-shell]");
            System.exit(1);
        }
        String cmFile = argv[0];
        String batchFile = argv[1];
        ConfigurationManager cm;
        BatchBW bmr = null;
      

        //BatchModeRecognizer recognizer;
        try {
            URL url = new File(cmFile).toURI().toURL();
            cm = new ConfigurationManager(url);
            bmr = (BatchBW) cm.lookup("batch");
	   
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
	bmr.decode(batchFile);
        
    }
}
