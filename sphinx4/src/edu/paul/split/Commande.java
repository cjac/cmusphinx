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
package edu.paul.split;
import java.util.logging.Logger;
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
import edu.cmu.sphinx.util.Utilities;
import edu.cmu.sphinx.util.LogMath;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.Loader;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.TableSenone;
import edu.cmu.sphinx.linguist.acoustic.AcousticModel;

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
public class Commande implements Configurable {

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
    private edu.paul.bw.Saver saver;
    private LogMath logMath;
    private PrintWriter fileidWriter;
    private Loader loader;
    private Stats stats;
    private int first,last;
    private String dataDir,noyauDir;
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
 	registry.register("saver", PropertyType.COMPONENT);
 	registry.register("AM", PropertyType.STRING);
	//	registry.register("bug",PropertyType.COMPONENT);
	registry.register("stats", PropertyType.COMPONENT);
	registry.register("dataDir",PropertyType.STRING);
	registry.register("noyauDir",PropertyType.STRING);
	registry.register("first",PropertyType.INT);
	registry.register("last",PropertyType.INT);
    }

    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
	saver=(edu.paul.bw.Saver) ps.getComponent("saver", 
						  edu.paul.bw.Saver.class);
        cm = ps.getPropertyManager();
	logMath = (LogMath) ps.getComponent("logMath", LogMath.class);
	stats = (Stats) ps.getComponent("stats", Stats.class);
	dataDir=ps.getString("dataDir","");
	noyauDir=ps.getString("noyauDir","");
	first=ps.getInt("first",0);
	last=ps.getInt("last",-1);
	//ps.getComponent("bug",AcousticModel.class);
	try {
	    if (cm.lookup(ps.getString("AM",""))==null)
		logger.info("pas trouve "+ ps.getString("AM",""));
; // je le charge 
	    PropertySheet aps= cm.getPropertySheet(ps.getString("AM",""));
	    if (aps==null)
		logger.info("pas de am dans batch et donc pas de machin");
	    else
		{
		    String [] lesNoms= aps.getNames();
		    String loaderName;
		    int i;
		    for ( i=0 ; i<lesNoms.length && ! lesNoms[i].matches("loader"); i++);
			//	System.err.format("%d %s \n",i,lesNoms[i]);
		    logger.info(lesNoms[i]);
		    loader=(Loader)aps.getComponent(lesNoms[i],Loader.class);
		    //loader =(Loader)ps.getComponent("loader",loader.class);
		    logger.info("loader :" + loader.getName());
		    }
	}
	catch (Exception e) 
	    { 
		loader=null;
		logger.warning(e.toString());
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

    public void faire() {
	if (loader!=null)try { loader.load();
		stats.init(null,null,loader);
		stats.save("verif");
}
	    catch (IOException e)
		{logger.warning(e.toString() + ":loader pas charge");
		    loader=null;}
	TableSenone TS= new TableSenone(loader);
	for(int i=first; i<=last; i++) {
	    FloatData []data;
	    if (i>= TS.lengthCI())
		data = loadData(new File(dataDir,i + ".gz"));
	    else {
		ArrayList<FloatData> d= new ArrayList<FloatData>(1000);
		Integer [] lescds=TS.lesCD(i);
		for (Integer cd :lescds)
		    loadData(new File(dataDir,cd.toString() + ".gz"),d);
		data=  d.toArray(new FloatData[0]);
	    }
		
	    float [][] noyaux=//loadNoyaux(new File(noyauDir,i+".means"));
		faireNoyaux(data);
	    stats.traiter(data,noyaux,loader,i,TS.toCi(i));
	}
    }

    private FloatData [] loadData(File f) {
	ArrayList<FloatData> d= new ArrayList<FloatData>(1000);
	loadData(f,d);
	if (d.size()>=10)
	    return d.toArray(new FloatData[0]);
	return null;
    }
    
    
    private void loadData(File f,ArrayList<FloatData> d) {
	logger.info("loading :"+f);
	int compteur=0;
   	try  {Scanner s=new Scanner( f);
	    s.useLocale(Locale.US);
	    while (s.hasNextFloat()) {
		s.nextInt();
		float values[]= new float[39];
		for ( int i=0 ;i<39;i++) 
		    values[i]=s.nextFloat();
		d.add(new FloatData(values,16000,compteur++,0));
	    }
	    s.close();
	}
	catch  (IOException dpe) {
            logger.warning(dpe.toString()+ "  j'en suis :" +d.size());
	}
	catch (java.util.InputMismatchException  exe) {
	    logger.warning(exe.toString() + "  j'en suis :" +d.size());
	    
	}
    }
    private float[][] faireNoyaux(FloatData [] lesdatas) {
	ArrayList<float[] > d= new ArrayList<float[]>(1000);
	double tirage= 256.0/lesdatas.length;
	
	for (int it=0; it<lesdatas.length;it++) 
	    if (Math.random()<tirage) 
		{int i;
		    float [] values= lesdatas[it].getValues().clone();
		    for (i =1; i<3&& it<lesdatas.length-1;i++) {
			float [] tab= lesdatas[++it].getValues();
			for (int l=0; l<tab.length;l++)
			    values[l]+=tab[l];
		    }
		    for (int l=0; l<values.length;l++)
			values[l]  /=i;
		    d.add(values);
		} 
	if (d.size()>=10)
	    return d.toArray(new float[0][39]);
	return null;
	
    }
    private float [][] loadNoyaux(File f) {
	ArrayList<float[] > d= new ArrayList<float[]>(1000);
	try  {Scanner s=new Scanner( f);
	    s.useLocale(Locale.US);
	    while (s.hasNextFloat()) {
		float values[]= new float[39];
		for ( int i=0 ; i<39 ; i++) 
		    values[i]=s.nextFloat();
		d.add(values);
	    }
	}
	catch  (IOException dpe) {
            logger.warning(dpe.toString());
	}
	catch (java.util.InputMismatchException  exe) {
	    logger.warning(exe.toString() + "  j'en suis :" +d.size());
	}
	if (d.size()>=10)
	    return d.toArray(new float[0][39]);
	return null;
    }	
	    
    /**
     * Main method of this BatchBW.
     * 
     * @param argv
     *                argv[0] : config.xml argv[1] : a file listing
     *                all the audio files to decode
     */
    public static void main(String[] argv) {
        if (argv.length < 1) {
            System.out.println(
                    "Usage: BatchDecoder propertiesFile ");
            System.exit(1);
        }
        String cmFile = argv[0];
        ConfigurationManager cm;
        Commande bmr = null;
      

        //BatchModeRecognizer recognizer;
        try {
            URL url = new File(cmFile).toURI().toURL();
            cm = new ConfigurationManager(url);
            bmr = (Commande) cm.lookup("batch");
	   
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
	bmr.faire();
        
    }
}
