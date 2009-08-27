package edu.paul.question;
import java.util.logging.Logger;
import java.util.Properties;
import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.FileInputStream;
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
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.logging.Logger;
import java.util.logging.Level;
import edu.cmu.sphinx.util.Utilities;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.util.ExtendedStreamTokenizer;
public class PetitLoader implements Configurable {
    protected final static int BYTE_ORDER_MAGIC = 0x11223344;
    protected final static String MIXW_FILE_VERSION = "1.0";
    public final static String MODEL_VERSION = "0.3";
    private float floor;
    private Logger logger;
    private ConfigurationManager cm;
    private String name;
    public void register(String name, Registry registry)
            throws PropertyException {
        this.name = name;
	registry.register("floor",PropertyType.FLOAT);

    }
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
        cm = ps.getPropertyManager();
	floor=ps.getFloat("floor",1.0e-15f);
    }
    public String getName() {
	return name;
    }
    public float [][] loadMixturePoids(String path) 
  throws FileNotFoundException, IOException {
	logger.fine("Loading mixture weights from: " + path );

	int numStates;
	int numStreams;
	int numGaussiansPerState;
        int numValues;
        Properties props = new Properties();

        DataInputStream dis = readS3BinaryHeader(path, props);

        String version = props.getProperty("version");
        boolean doCheckSum;

        if (version == null || !version.equals(MIXW_FILE_VERSION)) {
            throw new IOException("Unsupported version in " + path);
        }

        String checksum = props.getProperty("chksum0");
        doCheckSum =  (checksum != null && checksum.equals("yes"));

	numStates = readInt(dis);
	numStreams = readInt(dis);
	numGaussiansPerState = readInt(dis);
	numValues = readInt(dis);

        assert numValues == numStates * numStreams * numGaussiansPerState;
        assert numStreams == 1;
	float [][]pool =new float[numStates][numGaussiansPerState];
	float mini=100.0f;
	for (int i = 0; i < numStates; i++) {
	    pool[i] = readFloatArray(dis,numGaussiansPerState);
	    for (int j=0 ; j<pool[i].length ;j++)
		if (pool[i][j]<floor) {
		    if (pool[i][j]<mini) mini=pool[i][j];
		    pool[i][j]=floor;
		    
		}
	}

        dis.close();
	    logger.info("valeur mini :" + mini);
	return pool;
    }
private boolean swap;
   protected float readFloat(DataInputStream dis) throws IOException {
      float val;
        if (swap) {
            val = Utilities.readLittleEndianFloat(dis);
        } else {
            val = dis.readFloat();
        }
        return val;
    }

   protected DataInputStream readS3BinaryHeader(String path,
                                                 Properties props)
        throws IOException {

        // logger.fine("resource: " + path + ", " + getClass());
        InputStream inputStream;
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
            throw new IOException("Corrupt S3 file "  + path);
        }
        return dis;
    }
 
   private int byteSwap(int val) {
        return ((0xff & (val >> 24)) | (0xff00 & (val >> 8))
                | (0xff0000 & (val << 8)) | (0xff000000 & (val << 24)));
    }

    protected int readInt(DataInputStream dis) throws IOException {
        if (swap) {
            return Utilities.readLittleEndianInt(dis);
        } else {
            return dis.readInt();
        }
    }
    protected float[] readFloatArray(DataInputStream dis, int size)
	throws IOException {
        float[] data = new float[size];
        for (int i = 0; i < size; i++) {
            data[i] = readFloat(dis);
        }
        return data;
    }
   String readWord(DataInputStream dis) throws IOException {
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
     
    List<Etat> loadHMMDef(  String path,boolean mono, boolean filler, boolean tri)
                 
            throws FileNotFoundException, IOException {
	int token_type;
	int numBase;
	int numTri;
	int numStateMap;
	int numTiedState;
	int numStatePerHMM;
	int numContextIndependentTiedState;
	int numTiedTransitionMatrices;
	FileInputStream inputStream= new java.io.FileInputStream(path);
	List<Etat> result = new ArrayList<Etat>();
	ExtendedStreamTokenizer est = new ExtendedStreamTokenizer
            (inputStream, '#', false);

	logger.fine("Loading HMM file from: ");

	est.expectString(MODEL_VERSION);

	numBase = est.getInt("numBase");
	est.expectString("n_base");

	numTri = est.getInt("numTri");
	est.expectString("n_tri");

	numStateMap = est.getInt("numStateMap");
	est.expectString("n_state_map");

	numTiedState = est.getInt("numTiedState");
	est.expectString("n_tied_state");

	numContextIndependentTiedState = 
	    est.getInt("numContextIndependentTiedState");
	est.expectString("n_tied_ci_state");

	numTiedTransitionMatrices = est.getInt("numTiedTransitionMatrices");
	est.expectString("n_tied_tmat");

	numStatePerHMM = numStateMap/(numTri+numBase);

	// Load the base phones
	for (int i = 0; i < numBase; i++) {
	    String name = est.getString();
	    String left = est.getString();
	    String right = est.getString();
	    String position = est.getString();
	    String attribute  = est.getString();
	    int  tmat  = est.getInt("tmat");
	    
            int[] stid = new int[numStatePerHMM-1];
	    
	    for (int j=0; j < numStatePerHMM-1; j++) {
                stid[j] = est.getInt("j");
		assert stid[j] >= 0 && stid[j] < numContextIndependentTiedState;
	    }
	    est.expectString("N");
	    
	    assert left.equals("-");
	    assert right.equals("-");
	    assert position.equals("-");
	    assert tmat < numTiedTransitionMatrices;
            if (mono && ! attribute.equals("filler")
		|| filler && attribute.equals("filler")) {
		for (int k=0; k<stid.length;k++)
		    result.add(new Etat(name,left,right,position,k,stid[k]));
		if (logger.isLoggable(Level.FINE)) {
		    logger.fine("Loaded " + name);
		}
	    }
	    
	}
	
	// Load the context dependent phones. If the useCDUnits
	// property is false, the CD phones will not be created, but
        // the values still need to be read in from the file.
	
        String lastUnitName = "";
        int[] lastStid = null;
	int countBase=0;
	String abase="";
	if (tri)
	    for (int i = 0; i < numTri; i++) {
	    String name = est.getString();
	    String left = est.getString();
	    String right = est.getString();
	    String position = est.getString();
	    String attribute  = est.getString();
	    int  tmat  = est.getInt("tmat");

            int[] stid = new int[numStatePerHMM-1];

	    for (int j = 0; j < numStatePerHMM-1; j++) {
                stid[j] = est.getInt("j");
		assert stid[j] >= numContextIndependentTiedState && 
                       stid[j] < numTiedState;
	    }
	    est.expectString("N");

	    assert !left.equals("-");
	    assert !right.equals("-");
	    assert !position.equals("-");
	    assert attribute.equals("n/a");
	    assert tmat < numTiedTransitionMatrices;
	    if (! abase.equals(name)) {
		logger.fine(String.format("%s %d %d(total)",abase,countBase,result.size()));
		countBase=0;
	    }
	    countBase++;

	    //// truc pour tourner sur le Mac Paul
	    for (int k=0; k<stid.length;k+=1)
		result.add(new Etat(name,left,right,position,k,stid[k]));
	    abase=name;
	    }
    
    
	est.close();
	return result;
    }    
}

