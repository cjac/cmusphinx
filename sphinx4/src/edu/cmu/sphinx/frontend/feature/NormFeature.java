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
import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;
import java.util.Locale;

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
import edu.cmu.sphinx.util.Utilities;
/**
 * calulate Cmllr of stream
 *  the current cepstrum.
 * cmllr by bloc 13x13
 */
public class NormFeature extends BaseDataProcessor implements SetId {
    /**
     * The name of the SphinxProperty for the base directory where the 
     * cmllr transfo
     */
 private boolean swap;
  protected final static int BYTE_ORDER_MAGIC = 0x11223344;	

	
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
    private boolean bySent=false;
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
	if (retour==null) return input;
	if (input != null) {
	    if (input instanceof FloatData) transfo((FloatData) input);
	    if (input instanceof DataStartSignal){
		logger.info("transfo en cours normage " +id);
		ntrame=0;

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
		
	if (feat.length!= retour[0].length)
	    throw new  DataProcessingException("mismatch "+ retour[0].length +"!=" +
					       feat.length );
	for (int i =0 ; i< feat.length; i++)
	    feat[i] = (feat[i]-retour[0][i])/ retour[1][i];
	}
	if (false && ntrame <200) {
		    
		    System.out.printf("%3d",ntrame);
		    for (float f : feat) 
			System.out.printf(" %6.2f",f);
		    System.out.println("norm");
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


    private float[][] retour;
 protected final static String DENSITY_FILE_VERSION = "1.0";
 private  void loadDensityFileBinary(File path) 
        throws FileNotFoundException, IOException {
	int token_type;
	int numStates;
	int numStreams;
	int numGaussiansPerState;
        Properties props = new Properties();
        int blockSize = 0;

        DataInputStream dis = readS3BinaryHeader( path, props);

        String version = props.getProperty("version");
        boolean doCheckSum;

        if (version == null || !version.equals(DENSITY_FILE_VERSION)) {
            throw new IOException("Unsupported version in " + path);
        }

        String checksum = props.getProperty("chksum0");
        doCheckSum =  (checksum != null && checksum.equals("yes"));

        numStates = readInt(dis);
        numStreams = readInt(dis);
        numGaussiansPerState = readInt(dis);


        int[] vectorLength = new int[numStreams];
        for (int i = 0; i < numStreams; i++) {
            vectorLength[i] = readInt(dis);
        }

        int rawLength = readInt(dis);

        logger.fine("Nstates " + numStates);
        logger.fine("Nstreams " + numStreams);
        logger.fine("NgaussiansPerState " + numGaussiansPerState);
        logger.fine("vectorLength " + vectorLength.length);
        logger.fine("rawLength " + rawLength);

        for (int i = 0;  i < numStreams; i++) {
            blockSize += vectorLength[i];
        }


        assert rawLength == numGaussiansPerState * blockSize * numStates;
        assert numStreams == 1;

    	retour = new float[numStates*numStreams* numGaussiansPerState][];     
        int r = 0;
	for (int i = 0; i < numStates; i++) {
            for (int j = 0; j < numStreams; j++) {
                for (int k = 0; k < numGaussiansPerState; k++) {
                    float[] density = readFloatArray(dis, vectorLength[j]);
                    retour[i * numGaussiansPerState + k]= density;
		}
	    }
	}

        int checkSum = readInt(dis);
        // BUG: not checking the check sum yet.
        dis.close();
       
    }    /**
          * Reads the S3 binary hearder from the given location+path. Adds
          * header information to the given set of properties.
          * 
          * @param location
          *                the location of the file
          * @param path
          *                the name of the file
          * @param props
          *                the properties
          * 
          * @return the input stream positioned after the header
          * 
          * @throws IOException
          *                 on error
          */
    protected DataInputStream readS3BinaryHeader( File path,
                                                 Properties props)
        throws IOException {

        // logger.fine("resource: " + path + ", " + getClass());
        java.io.InputStream inputStream;

	    inputStream= new java.io.FileInputStream(path);



        if (inputStream == null) {
            throw new IOException("Can't open " + path);
        }
        DataInputStream dis = new DataInputStream(new BufferedInputStream(
                inputStream));
        String id = readWord(dis);
        if (!id.equals("s3")) {
            throw new IOException("Not proper s3 binary file "
                    + path);
        }
        String name;
        while ((name = readWord(dis)) != null) {
            if (!name.equals("endhdr")) {
                String value = readWord(dis);
                props.setProperty(name, value);
            } else {
                break;
            }
        }
        int byteOrderMagic = dis.readInt();
        if (byteOrderMagic == BYTE_ORDER_MAGIC) {
            // logger.fine("Not swapping " + path);
            swap = false;
        } else if (byteSwap(byteOrderMagic) == BYTE_ORDER_MAGIC) {
            // logger.fine("SWAPPING " + path);
            swap = true;
        } else {
            throw new IOException("Corrupt S3 file " + path);
        }
        return dis;
    }
    /**
     * Reads the next word (text separated by whitespace) from the given stream
     * 
     * @param dis
     *                the input stream
     * 
     * @return the next word
     * 
     * @throws IOException
     *                 on error
     */
private     String readWord(DataInputStream dis) throws IOException {
        StringBuffer sb = new StringBuffer();
        char c;
        // skip leading whitespace
        do {
            c = readChar(dis);
        } while (Character.isWhitespace(c));
        // read the word
        do {
            sb.append(c);
            c = readChar(dis);
        } while (!Character.isWhitespace(c));
        return sb.toString();
    }
    /**
     * Reads a single char from the stream
     * 
     * @param dis
     *                the stream to read
     * @return the next character on the stream
     * 
     * @throws IOException
     *                 if an error occurs
     */
    private char readChar(DataInputStream dis) throws IOException {
        return (char) dis.readByte();
    }
    /**
     * swap a 32 bit word
     * 
     * @param val
     *                the value to swap
     * 
     * @return the swapped value
     */
    private int byteSwap(int val) {
        return ((0xff & (val >> 24)) | (0xff00 & (val >> 8))
                | (0xff0000 & (val << 8)) | (0xff000000 & (val << 24)));
    }
    /**
     * Read an integer from the input stream, byte-swapping as necessary
     * 
     * @param dis
     *                the inputstream
     * 
     * @return an integer value
     * 
     * @throws IOException
     *                 on error
     */
    protected int readInt(DataInputStream dis) throws IOException {
        if (swap) {
            return Utilities.readLittleEndianInt(dis);
        } else {
            return dis.readInt();
        }
    }


  protected int[] readIntArray(DataInputStream dis, int size)
            throws IOException {
        int[] data = new int[size];
        for (int i = 0; i < size; i++) {
            data[i] = readInt(dis);
        }
        return data;
    }

    /**
     * Read a float from the input stream, byte-swapping as necessary
     * 
     * @param dis
     *                the inputstream
     * 
     * @return a floating pint value
     * 
     * @throws IOException
     *                 on error
     */
    protected float readFloat(DataInputStream dis) throws IOException {
        float val;
        if (swap) {
            val = Utilities.readLittleEndianFloat(dis);
        } else {
            val = dis.readFloat();
        }
        return val;
    }

    /**
     * Dump the data
     * 
     * @param name
     *                the name of the data
     * @param data
     *                the data itself
     *  
     */
    private void dumpData(String name, float[] data) {
        System.out.println(" ----- " + name + " -----------");
        for (int i = 0; i < data.length; i++) {
            System.out.println(name + " " + i + ": " + data[i]);
        }
    }

    /**
     * Reads the given number of floats from the stream and returns them in an
     * array of floats
     * 
     * @param dis
     *                the stream to read data from
     * @param size
     *                the number of floats to read
     * 
     * @return an array of size float elements
     * 
     * @throws IOException
     *                 if an exception occurs
     */
    protected float[] readFloatArray(DataInputStream dis, int size)
            throws IOException {
        float[] data = new float[size];
        for (int i = 0; i < size; i++) {
            data[i] = readFloat(dis);
        }
        return data;
    }













    public void setId(String dumpDir,String id)throws IOException {
	if (dumpDir.equals("") ) { retour=null; return;}
	String []temp= id.split("!");
	String[] dec= temp[0].split("-");
        StringBuffer lenom=new StringBuffer(dec[3]);
        for (int l=4 ; l< dec.length ; l++){
            lenom.append("-");
	    lenom.append(dec[l]);
        }

	    
		String locId=dec[0]+"-"+lenom.toString();
		if (locId.equals(this.id)) return;
		this.id=locId;
	      loadDensityFileBinary(new File(new File(dumpDir,dec[0]),lenom.toString()));

    }
}
