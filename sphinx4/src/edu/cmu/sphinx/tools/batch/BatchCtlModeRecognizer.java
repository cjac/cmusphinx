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
import java.util.zip.GZIPOutputStream;
import java.net.URL;
import java.util.Iterator;
import java.util.List;
import java.util.logging.Logger;
import java.util.logging.Level;
import edu.cmu.sphinx.util.SplitNameFile;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.UnsupportedAudioFileException;
import edu.paul.Confidence;
import edu.cmu.sphinx.result.ConfidenceScorer;
import edu.cmu.sphinx.frontend.DataProcessor;
import edu.cmu.sphinx.frontend.util.StreamCepstrumSource;
import edu.cmu.sphinx.frontend.util.StreamDataSource;
import edu.cmu.sphinx.recognizer.Recognizer;
import edu.cmu.sphinx.recognizer.RecognizerState;
import edu.cmu.sphinx.result.Result;
import edu.cmu.sphinx.util.BatchItem;
import edu.cmu.sphinx.util.BatchManager;
import edu.cmu.sphinx.util.PooledBatchManager;
import edu.cmu.sphinx.util.SimpleBatchManager;
import edu.cmu.sphinx.util.Utilities;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.util.CommandInterpreter;
import edu.cmu.sphinx.util.CommandInterface;
import edu.cmu.sphinx.util.props.SetId;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.Loader;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.DynamicTransfo;
// je sais c'est pas beau mais ....

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
public class BatchCtlModeRecognizer implements Configurable {

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
    public final static String PROP_RECOGNIZER = "recognizer";

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
    /**
     * The sphinx property that specifies if ctm dum and where
     */
    public final static String PROP_CTM = "CTM";
    public final static String PROP_CTM_DEFAULT = null;
    public final static String PROP_ALIGNER = "aligner";
    public final static boolean PROP_ALIGNER_DEFAULT = false;
   

    /**
     * the fsm grammar if saucussice decoder
     */
    public final static String PROP_FSM_GRAMMAR = "fsmGrammar";
    public final static String PROP_CMLLR = "cmllr";
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
    private Recognizer recognizer;
    private Logger logger;
    private String feat_dir;
    private String feat_ext;
    private BatchItem curBatchItem;
    private ConfigurationManager cm;
    private String ctmName;
    private PrintWriter ctmWriter;
    private ConfidenceScorer cs = null;
    private SetId fsmGrammar=null;
    private SetId cmllr=null;
    private  Loader loader=null;
    private DynamicTransfo dynamicTransfo=null;
    private boolean dumpLattice=false;
    private boolean aligner=false;
    private boolean ctmWithPho=false;
    private String dumpLatticeDir="";
    private boolean dumpLatticeS3 =false;
    private boolean compound=false;
    private boolean variantesDico=false;
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
        registry.register(PROP_RECOGNIZER, PropertyType.COMPONENT);
        registry.register(PROP_INPUT_DATA_PROCESSORS, 
                          PropertyType.COMPONENT_LIST);

	registry.register(PROP_SIGNAL_DIR, PropertyType.STRING);
	registry.register(PROP_SIGNAL_EXT, PropertyType.STRING);
	registry.register(PROP_CTM,PropertyType.STRING);
	registry.register(PROP_FSM_GRAMMAR,PropertyType.COMPONENT);
	registry.register(PROP_CMLLR,PropertyType.COMPONENT);
	registry.register("AM",PropertyType.STRING);
	registry.register("mllrDir",PropertyType.STRING);
	registry.register("mllrByShow",PropertyType.BOOLEAN);
	registry.register("dumpLattice",PropertyType.BOOLEAN);
	registry.register("dumpLatticeS3",PropertyType.BOOLEAN);
	registry.register("dumpLatticeDir",PropertyType.STRING);
	registry.register(PROP_ALIGNER,PropertyType.BOOLEAN);
	registry.register("ctmWithPho",PropertyType.BOOLEAN);
	registry.register("compound",PropertyType.BOOLEAN);
	registry.register("variantesDico",PropertyType.BOOLEAN);

    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
     
        cm = ps.getPropertyManager();
	try {
	    cs =(ConfidenceScorer) cm.lookup("confidenceScorer");
	}
        catch (Exception e) 
	    {
		System.err.println(e);
		cs=null;
	    }
        if (cs != null)
	    System.out.println (cs.getName());
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
        recognizer = (Recognizer) ps.getComponent(PROP_RECOGNIZER,
						  Recognizer.class);
        inputDataProcessors = (List) ps.getComponentList
            (PROP_INPUT_DATA_PROCESSORS, DataProcessor.class);
        feat_dir = ps.getString(PROP_SIGNAL_DIR,PROP_SIGNAL_DIR_DEFAULT);
        feat_ext = ps.getString(PROP_SIGNAL_EXT,PROP_SIGNAL_EXT_DEFAULT);
        ctmName = ps.getString(PROP_CTM,PROP_CTM_DEFAULT);
        aligner = ps.getBoolean(PROP_ALIGNER,PROP_ALIGNER_DEFAULT);
        ctmWithPho = ps.getBoolean("ctmWithPho",false);
        compound = ps.getBoolean("compound",false);
	variantesDico=ps.getBoolean("variantesDico",false);
	if ((aligner||ctmWithPho)&& compound)
	    logger.warning("bizarre Compound avec aligner ou ctmpho");
        try {fsmGrammar= (SetId) ps.getComponent(PROP_FSM_GRAMMAR,SetId.class);}
        catch (Exception e) { fsmGrammar=null;logger.warning("pas de fsm lat gram"+e.toString());}
	try { PropertySheet aps= cm.getPropertySheet(ps.getString("AM",""));
	    if (aps==null)
		logger.info("pas de am dans batch et donc pas de mllr");
	    else
		{
		    String [] lesNoms= aps.getNames();
		    String loaderName;
		    int i;
		    for ( i=0 ; i<lesNoms.length && ! lesNoms[i].matches("loader"); i++);
		    //			System.err.format("%d %s \n",i,lesNoms[i]);
		    logger.info(lesNoms[i]);
		    loader=(Loader)aps.getComponent(lesNoms[i],Loader.class);
		    //loader =(Loader)ps.getComponent("loader",loader.class);
		    dynamicTransfo= new DynamicTransfo(loader,ps.getString("mllrDir",""),
						       ps.getBoolean("mllrByShow",false));}
	}
	catch (Exception e) 
	    { loader=null;dynamicTransfo=null;
		logger.warning(e.toString());}
	try {cmllr= (SetId) ps.getComponent(PROP_CMLLR,SetId.class);}
        catch (Exception e) 
	    { cmllr=null;logger.warning(e.toString());}
	dumpLattice=ps.getBoolean("dumpLattice",false);
	dumpLatticeS3=ps.getBoolean("dumpLatticeS3",false);
	dumpLatticeDir=ps.getString("dumpLatticeDir","");
   
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
        if (ctmName!= null) 
	    try { if (ctmName.endsWith(".gz")) 
		      ctmWriter =new PrintWriter(new BufferedWriter(new OutputStreamWriter(new GZIPOutputStream(new FileOutputStream(ctmName)),
        				"ISO8859_1")));
		    
		else
        		ctmWriter =new PrintWriter(new BufferedWriter(new OutputStreamWriter(new FileOutputStream(ctmName),
        				"ISO8859_1")));
        	
        	} catch (IOException ioe) {
        		System.err.println("IO erreur durant ctm file" + ctmName +" "+ ioe);
        		return;
        	}

    
        try {
            recognizer.allocate();
            setBatchFile(batchFile);
	    String baseLat=aligner? "lataligne": "lat";
	    if (! dumpLatticeDir.equals(""))
		baseLat = (new File(dumpLatticeDir,baseLat)).getPath();
            batchManager.start();
            logger.info("BatchDecoder: decoding files in "
                    + batchManager.getFilename());
        
            while (count < totalCount && 
                        (batchItem = batchManager.getNextItem()) != null) {
                if (fsmGrammar !=null) fsmGrammar.setId(aligner ? batchItem.getTranscript() :
							batchItem.getId());
                if (cmllr !=null) cmllr.setId(batchItem.getId());
                setInputStream(batchItem.getFilename(),batchItem.getStartSentence(),batchItem.getEndSentence());
		if (loader!=null)
		    dynamicTransfo.setId(batchItem.getId());
		logger.info("File  : " + batchItem.getFilename());
                {Result result = recognizer.recognize(batchItem.getTranscript());
		   
		    logger.info("Result: " + result);
		    //result.getBestToken().dumpTokenPath();
		    if (ctmWriter!=null)  
			result.dumpCtm(ctmWriter,batchItem.getStartSentence(),
				       batchItem.getFilename(),batchItem.getId(),
				       aligner|| ctmWithPho);
		    count++;
		    if (cs!=null) {
			Confidence.confucius(cs,result);
		    }
		    else if (dumpLattice|| dumpLatticeS3) {
			if (logger.isLoggable(Level.FINE)){
			    logger.fine("best path:");
			    result.getBestToken().dumpTokenPath();
			}
			//			(new edu.paul.lattice.Lattice(result)).dumpAISee("toto.gdl","toto");
			if (result.getBestFinalToken() != null) {
			    if (dumpLattice) 
				{(new edu.paul.lattice.Lattice(result,aligner)).dump(

								    SplitNameFile.splitNameFile(baseLat,batchItem.getId(),".lat.gz",true));
				    if (false)   (new edu.paul.lattice.Lattice(result,aligner)).dumpAISee("aisee"+batchItem.getId()+".gdl",
											     "aisee"+batchItem.getId()+ ".gdl");  
				    if (false && ! aligner)	
					throw new Error("j'ai pas modifie paul lattice avec new version of loser");		}
			    if (dumpLatticeS3)
				
				(new edu.cmu.sphinx.result.Lattice(result,false,variantesDico)).compound(compound).dumpS3(
										   SplitNameFile.splitNameFile(dumpLatticeDir,batchItem.getId(),".lat.gz",true));


			    
			    // (new edu.cmu.sphinx.result.Lattice(result)).compound(compound).dumpAISee(batchItem.getId()+".gdl",batchItem.getId(),true);




				}
			else 			    
			    logger.warning(" pas de lattice " + batchItem.getId());
		    }
		}
		System.gc();
            }
            batchManager.stop();
            recognizer.deallocate();
        } catch (IOException io) {
            logger.severe("I/O error during decoding: " + io.getMessage());
        };
        logger.info("BatchDecoder: " + count + " files decoded");
        ctmWriter.close();
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
            try {
                File file = new File(filename);
                logger.info
                    (AudioSystem.getAudioFileFormat(file).toString());
                is = AudioSystem.getAudioInputStream(file);
            } catch (UnsupportedAudioFileException uafe) {
                logger.info
                    ("Reading " + filename + " as raw audio file.");
                is = new FileInputStream( filename);
            }
            if (dataSource instanceof StreamDataSource) {
                ((StreamDataSource) dataSource).setInputStream(is, filename);
				// la il y un probleme mais plus tard
				throw  new RuntimeException();
            } else if (dataSource instanceof StreamCepstrumSource) {
                boolean isBigEndian = Utilities
                    .isCepstraFileBigEndian(filename);
                StreamCepstrumSource cepstrumSource =
                    (StreamCepstrumSource) dataSource;
                cepstrumSource.setInputStream(is, isBigEndian,startSentence,endSentence);
                logger.info("read between "+ startSentence + " "+ endSentence);
            }
        }
    }


    /**
     * Add commands to the given interpreter to support shell mode
     *
     *
     * @param ci the interpreter
     */
    private void addCommands(CommandInterpreter ci) {
	ci.add("ls", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length != 1) {
                    ci.putResponse("Usage: ls");
                 } else {
                     String[] names = cm.getInstanceNames(Configurable.class);
                     for (int i = 0; i < names.length; i++) {
                         ci.putResponse(names[i]);
                     }
                }
                return "";
           }
            public String getHelp() {
                return "list active components";
            }
        });
	ci.add("show", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length < 2) {
                    cm.showConfig();
                 } else {
                     for (int i = 1; i < args.length; i++) {
                        String name = args[i];
                        cm.showConfig(name);
                     }
                }
                return "";
           }
            public String getHelp() {
                return "show component configuration";
            }
        });
	ci.add("edit", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length != 2) {
                    ci.putResponse("Usage: edit component");
                 } else {
                    cm.editConfig(args[1]);
                }
                return "";
           }
            public String getHelp() {
                return "edit a  component's configuration";
            }
        });
	ci.add("save", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length != 2) {
                    ci.putResponse("Usage: save filename.xml");
                 } else {
                    try {
                         cm.save(new File(args[1]));
                    } catch (IOException ioe) {
                        ci.putResponse("Can't save, " + ioe);
                    }
                }
                return "";
           }
            public String getHelp() {
                return "save configuration to a file";
            }
        });
	ci.add("set", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length != 4) {
                    ci.putResponse("Usage: set component property value");
                 } else {
                    try {
                        cm.setProperty(args[1], args[2], args[3]);
                    } catch (PropertyException pe) {
                        ci.putResponse(pe.toString());
                    }
                }
                return "";
           }
            public String getHelp() {
                return "set component property to a given value";
            }
        });
	ci.add("recognize", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length < 4) {
                    ci.putResponse("Usage: recognize audio debut fin");
                 } else {
                    String audioFile = args[1];
                    int debut,fin;
                    
                       debut = Integer.parseInt(args[2]);
					   fin = Integer.parseInt(args[3]);
                   
                    try {
                        setInputStream(audioFile,debut,fin);
                        Result result = recognizer.recognize(null);
                    } catch (IOException io) {
                        ci.putResponse("I/O error during decoding: " +
                            io.getMessage());
                    }
                }
                return "";
           }
            public String getHelp() {
                return "perform recognition on the given audio";
            }
        });
        ci.addAlias("recognize", "rec");

	ci.add("statsReset", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length != 1) {
                    ci.putResponse("Usage: statsReset");
                 } else {
                    recognizer.resetMonitors();
                }
                return "";
           }
            public String getHelp() {
                return "resets gathered statistics";
            }
        });

	ci.add("batchRecognize", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length != 1) {
                    ci.putResponse("Usage: batchRecognize");
                 } else {
                    try {
                        if (curBatchItem == null) {
                            batchManager.start();
                            curBatchItem = batchManager.getNextItem();
                        }
                        String audioFile = curBatchItem.getFilename();
                        String transcript = curBatchItem.getTranscript();
                        setInputStream(audioFile,curBatchItem.getStartSentence(),curBatchItem.getEndSentence());
                        Result result = recognizer.recognize(transcript);
                    } catch (IOException io) {
                        ci.putResponse("I/O error during decoding: " +
                            io.getMessage());
                    }
                }
                return "";
           }
            public String getHelp() {
                return "perform recognition on the current batch item";
            }
        });
        ci.addAlias("batchRecognize", "br");

	ci.add("batchNext", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length != 1 && args.length != 2) {
                    ci.putResponse("Usage: batchNext [norec]");
                 } else {
                    try {

                        // if we don't have a batch item, start (or
                        // start over)

                        if (curBatchItem == null) {
                            batchManager.start();
                        }
                        curBatchItem = batchManager.getNextItem();

                        // if we reach the end, just loop back and
                        // start over.

                        if (curBatchItem == null) {
                            batchManager.start();
                            curBatchItem = batchManager.getNextItem();
                        }

                        String audioFile = curBatchItem.getFilename();
                        String transcript = curBatchItem.getTranscript();
                        if (args.length == 2) {
                            ci.putResponse("Skipping: " + transcript);
                        } else {
							setInputStream(audioFile,curBatchItem.getStartSentence(),curBatchItem.getEndSentence());
                            //setInputStream(audioFile);
                            Result result = recognizer.recognize(transcript);
                        }
                    } catch (IOException io) {
                        ci.putResponse("I/O error during decoding: " +
                            io.getMessage());
                    }
                }
                return "";
           }
            public String getHelp() {
                return "advance the batch and perform recognition";
            }
        });
        ci.addAlias("batchNext", "bn");

	ci.add("batchAll", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length != 1) {
                    ci.putResponse("Usage: batchAll");
                 } else {
                    try {
                        if (curBatchItem == null) {
                            batchManager.start();
                        }

                        while (true) {
                            curBatchItem = batchManager.getNextItem();
                            // if we reach the end  bail out

                            if (curBatchItem == null) {
                                return "";
                            }
                            String audioFile = curBatchItem.getFilename();
                            String transcript = curBatchItem.getTranscript();
							setInputStream(audioFile,curBatchItem.getStartSentence(),curBatchItem.getEndSentence());
							//setInputStream(audioFile);
                            Result result = recognizer.recognize(transcript);
                        }
                    } catch (IOException io) {
                        ci.putResponse("I/O error during decoding: " +
                            io.getMessage());
                    }
                }
                return "";
           }
            public String getHelp() {
                return "recognize all of the remaining batch items";
            }
        });

	ci.add("batchReset", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length != 1) {
                    ci.putResponse("Usage: batchReset");
                 } else {
                    try {
                        batchManager.start();
                    } catch (IOException ioe) {
                        ci.putResponse("trouble reseting batch");
                    }
                }
                return "";
           }
           public String getHelp() {
               return "reset the batch to the beginning";
           }
        });
	ci.add("batchLoad", new CommandInterface() {
	     public String execute(CommandInterpreter ci, String[] args) {
                 if (args.length != 2) {
                    ci.putResponse("Usage: batchReset batchfile");
                 } else {
                    try {
                        setBatchFile(args[1]);
                    } catch (IOException ioe) {
                        ci.putResponse("Can't load " + args[1] + " " + ioe);
                    }
                }
                return "";
           }
           public String getHelp() {
               return "reset the batch to the beginning";
           }
        });
    }

    public void shell(String batchfile) {
        try {
            CommandInterpreter ci = new CommandInterpreter();
            ci.setPrompt("s4> "); 
            addCommands(ci);
            setBatchFile(batchfile);
            recognizer.allocate();
            ci.run();
            batchManager.stop();
            if (recognizer.getState() == RecognizerState.READY) {
                recognizer.deallocate();
            }
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
        BatchCtlModeRecognizer bmr = null;
      

        //BatchModeRecognizer recognizer;
        try {
            URL url = new File(cmFile).toURI().toURL();
            cm = new ConfigurationManager(url);
            bmr = (BatchCtlModeRecognizer) cm.lookup("batch");
	   
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

        
        if (bmr == null) {
            System.err.println("Can't find batchModeRecognizer in " + cmFile);
            return;
        }
	System.gc();
        if (argv.length >= 3 && argv[2].equals("-shell")) {
            bmr.shell(batchFile);

        } else {
            bmr.decode(batchFile);
	    
        }
    }
}
