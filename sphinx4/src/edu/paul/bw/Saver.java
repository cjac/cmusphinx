package edu.paul.bw;
import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.util.Properties;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.Locale;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.util.props.Configurable;
import java.util.Enumeration;
import edu.cmu.sphinx.util.Utilities;
public class Saver implements Configurable {
    private String name;
    private Logger logger;
    private String dirBase;
    protected final static int BYTE_ORDER_MAGIC = 0x11223344;
    public final static String MODEL_VERSION = "0.3";
    protected final static String DENSITY_FILE_VERSION = "1.0";
    protected final static String MIXW_FILE_VERSION ="mixwcnt1.0";
    protected final static String GAUCNT_FILE_VERSION  ="1.0";
 
    private String checksum;
    private boolean doCheckSum=false;
    private boolean swap;
    public void register(String name, Registry registry)
	throws PropertyException {
	this.name = name;
	registry.register("dirBase",PropertyType.STRING);
	registry.register("swap",PropertyType.BOOLEAN);
    }
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
	dirBase=ps.getString("dirBase","bwaccumdir");
	swap=ps.getBoolean("swap",false);
    }
    public String getName() { return name;}
    public void save(String ext , double [][] data, int sizeMix)  throws IOException {
	save(ext,data,sizeMix,"");
    }
    public void save(String ext , double [][] data, int sizeMix,String modif)  throws IOException {
	File f=new File(dirBase+modif, ext);
	logger.info("Saving density file to: "+ f);
	DataOutputStream dos = new DataOutputStream(
						    new BufferedOutputStream(new FileOutputStream(f)));
	Properties props = new Properties();
	int checkSum = 0;
	props.setProperty("version", DENSITY_FILE_VERSION);
	props.setProperty("chksum0", checksum+"");
	writeS3BinaryHeader(dos,props);
	writeInt(dos,data.length/sizeMix);
	writeInt(dos,1);
	writeInt(dos,sizeMix);
	writeInt(dos,data[0].length);
	writeInt(dos,data.length*data[0].length);
	float [] out=new float[data[0].length];
	for (int g=0; g< data.length; g++) {
	    for (int i=0; i< out.length ;i++)
		out[i] =(float) data[g][i];
	    writeFloatArray(dos,out);
	}
	writeInt(dos, checkSum);
	dos.close();
    }
    public void save( double [][] data)  throws IOException {
	save(data,"");
    }

    public void save( double [][] data,String Modif)  throws IOException {
	File f=new File(dirBase+Modif, "mixw_counts");
	logger.info("Saving mixture file to: "+ f);
	DataOutputStream dos = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(f)));
	Properties props = new Properties();
	int checkSum = 0;
	props.setProperty("version", MIXW_FILE_VERSION);
	//props.setProperty("chksum0", checksum+"");
	writeS3BinaryHeader(dos,props);
	writeInt(dos,data.length);
	writeInt(dos,1);
	writeInt(dos,data[0].length);
	writeInt(dos,data.length*data[0].length);
	float [] out=new float[data[0].length];
	for (int g=0; g< data.length; g++) {
	    //    for (int i=0; i< out.length ;i++)
	    //out[i] =(float) data[g][i];
	    writeDoubleArray(dos,data[g]);
	}
	writeInt(dos, checkSum);//a voir
	dos.close();
    }

    public void save( double [][] mixw, double [][] means , double [][] vars )  throws IOException {
	save(mixw,means,vars,"");
    }


    public void save( double [][] mixw, double [][] means , double [][] vars,String modif )  throws IOException {
	File f=new File(dirBase+modif, "gauden_counts");
	logger.info("Saving gaucnts file to: "+ f);
	DataOutputStream dos = new DataOutputStream(new BufferedOutputStream (new FileOutputStream(f),1024*1024));
	Properties props = new Properties();
	int checkSum = 0;
	props.setProperty("version", GAUCNT_FILE_VERSION);
	writeS3BinaryHeader(dos,props);
	writeInt(dos,1); //hasMean
	writeInt(dos,1); //hasVars
	writeInt(dos,1); //passe2 var donc il ne va pas toucher au carre
	writeInt(dos,mixw.length); //n_cb
	writeInt(dos,mixw[0].length);
	writeInt(dos,1);
	writeInt(dos,means[0].length);
	writeInt(dos,means.length*means[0].length);
	    {float [] outm=new float[means[0].length];
		for (int g=0; g< means.length; g++) {
		    //		    for (int i=0; i< means[g].length ;i++)
		    //outm[i] =(float) means[g][i];
		    writeDoubleArray(dos,means[g]);
		}
		writeInt(dos,vars.length*vars[0].length);
		    for (int g=0; g< vars.length; g++) {
			//	for (int i=0; i< vars[g].length ;i++)
			//  outm[i] =(float) vars[g][i];
			writeDoubleArray(dos,vars[g]);
		    }
	    }
	writeInt(dos,mixw.length);
	writeInt(dos,1);
	writeInt(dos,mixw[0].length);
	writeInt(dos,mixw.length*mixw[0].length);

	    for (int testId=0 ; testId<mixw.length ; testId++){
		double testCumul=0.0;
	    
		for (int imix=0 ; imix< mixw[testId].length;imix++) testCumul +=mixw[testId][imix];
		if (testCumul !=0.0)
		    System.err.format(Locale.US,"monophone vu %4d %7g\n",testId,testCumul);
	    }




	{//float [] outmix=new float[mixw[0].length];
	    for (int g=0; g< mixw.length; g++) {
	    //for (int i=0; i< mixw[g].length ;i++)
	    //    outmix[i] =(float) mixw[g][i];
		writeDoubleArray(dos,mixw[g]);
	    }
	}
	writeInt(dos, checkSum);//a voir
	dos.close();
    	
	
    }
    
    private void  writeS3BinaryHeader(	DataOutputStream dos, 	Properties props) throws IOException {
	writeWord(dos, "s3\n");
	for (Enumeration e = props.keys(); e.hasMoreElements(); ) {
	    String name = (String) e.nextElement();
	    String value = props.getProperty(name);
	    writeWord(dos, name + " " + value + "\n");
	}
	writeWord(dos, "endhdr\n");
	writeInt(dos, BYTE_ORDER_MAGIC);
    }
 
    private   void writeWord(DataOutputStream dos, String word) throws IOException {
	    dos.writeBytes(word);
    }
 
 
    private void writeChar(DataOutputStream dos, char character) 
	throws IOException {
        dos.writeByte(character);
    }
    private int byteSwap(int val) {
        return ((0xff & (val >>24)) | (0xff00 & (val >>8)) |
         (0xff0000 & (val <<8)) | (0xff000000 & (val <<24)));
    }

    protected void writeInt(DataOutputStream dos, int val) throws IOException {
        if (swap) {
            dos.writeInt(Utilities.swapInteger(val));
        } else {
            dos.writeInt(val);
        }
    }
    protected void writeFloat(DataOutputStream dos, float val) 
	throws IOException {
        if (swap) {
            dos.writeFloat(Utilities.swapFloat(val));
        } else {
            dos.writeFloat(val);
        }
    }
    protected void writeFloatArray(DataOutputStream dos, float[] data)
        throws IOException{
	
        for (int i = 0; i < data.length; i++) {
            writeFloat(dos, data[i]);
        }
    }
   protected void writeDouble(DataOutputStream dos, double val) 
	throws IOException {
        if (swap) {
            dos.writeLong(Utilities.swapDouble(val));
        } else {
            dos.writeLong(Double.doubleToRawLongBits(val));
        }
    }
  

  protected void writeDoubleArray(DataOutputStream dos, double[] data)
        throws IOException{
	
        for (int i = 0; i < data.length; i++) {
            writeDouble(dos, data[i]);
        }
    }
    public static void main( String [] argv) throws IOException {
	double test[] = new double [16];
	File f=new File("toto");
	DataOutputStream dos = new DataOutputStream(new BufferedOutputStream (new FileOutputStream(f),1024));
	for (int i =0 ;i<=15;i++)
	    test[i]=i+0.5;
	for (int i =0 ;i<=15;i++)
	    dos.writeLong(Double.doubleToRawLongBits(test[i]));
	
	for (int i =0 ;i<=15;i++)
	 dos.writeLong(Utilities.swapDouble(test[i]));
	dos.close();
    }
}
 
