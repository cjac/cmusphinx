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

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import edu.cmu.sphinx.frontend.BaseDataProcessor;
import edu.cmu.sphinx.frontend.Data;
import edu.cmu.sphinx.frontend.DataEndSignal;
import edu.cmu.sphinx.frontend.DataProcessingException;
import edu.cmu.sphinx.frontend.DoubleData;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import java.util.logging.Logger;
import java.util.logging.Level;

/**
 * Applies cepstral mean normalization (CMN), sometimes called channel
 * mean normalization, to incoming cepstral data. Its goal is to
 * reduce the distortion caused by the transmission channel.  The
 * output is mean normalized cepstral data.
 * <p>
 * The CMN processing subtracts the mean from all the {@link Data}
 * objects between a {@link edu.cmu.sphinx.frontend.DataStartSignal}
 * and a {@link DataEndSignal}.  BatchCMN will read in all the {@link
 * Data} objects, calculate the mean, and subtract this mean from all
 * the {@link Data} objects. For a given utterance, it will only
 * produce an output after reading all the incoming data for the
 * utterance. As a result, this process can introduce a significant
 * processing delay, which is acceptable for batch processing, but not
 * for live mode. In the latter case, one should use the {@link
 * LiveCMN}.
 * <p>
 * CMN is a technique used to reduce distortions that are introduced
 * by the transfer function of the transmission channel (e.g., the
 * microphone). Using a transmission channel to transmit the input
 * speech translates to multiplying the spectrum of the input speech
 * with the transfer function of the channel (the distortion).  Since
 * the cepstrum is the Fourier Transform of the log spectrum, the
 * logarithm turns the multiplication into a summation. Averaging over
 * time, the mean is an estimate of the channel, which remains roughly
 * constant. The channel is thus removed from the cepstrum by
 * subtracting the mean cepstral vector. Intuitively, the mean
 * cepstral vector approximately describes the spectral
 * characteristics of the transmission channel (e.g., microphone).
 *
 * @see LiveCMN
 */
public class BatchCMN extends BaseDataProcessor {
    /**
     * The SphinxProperty specifying whether the input is in binary.
     */
    public final static String PROP_AGC = "AGC";
    /**
     * The default value for PROP_AGC.
     */
    public final static boolean PROP_AGC_DEFAULT = false;

    private double[] sums;           // array of current sums
    private double maxi; // max of c0
    private List cepstraList;
    private int numberDataCepstra;
    private boolean agc;
    private Logger logger;
    /* (non-Javadoc)
     * @see edu.cmu.sphinx.util.props.Configurable#register(java.lang.String, edu.cmu.sphinx.util.props.Registry)
     */
    public void register(String name, Registry registry) throws PropertyException {
        super.register(name, registry);
	registry.register(PROP_AGC, PropertyType.BOOLEAN);
        
    }


    /* (non-Javadoc)
     * @see edu.cmu.sphinx.util.props.Configurable#newProperties(edu.cmu.sphinx.util.props.PropertySheet)
     */
    public void newProperties(PropertySheet ps) throws PropertyException {
        super.newProperties(ps);
	 agc = ps.getBoolean(PROP_AGC, PROP_AGC_DEFAULT);
         logger = ps.getLogger();
    }

    /**
     * Initializes this BatchCMN.
     *
     */
    int gicle=0;
    int compte=0;
    public void initialize() {
        super.initialize();
        sums = null;
        cepstraList = new LinkedList();
	maxi = -Double.MAX_VALUE;
	/* java.util.Scanner sc = new java.util.Scanner(System.in);
        System.err.println(" on entre gicle :");
	gicle  = sc.nextInt();
	 compte=0;*/
    }


    /**
     * Initializes the sums array and clears the cepstra list.
     */
    private void reset() {
        sums = null; // clears the sums array
	cepstraList.clear();
	numberDataCepstra = 0;
        maxi = -Double.MAX_VALUE;
	compte=0;
      
    }


    /**
     * Returns the next Data object, which is a normalized cepstrum.
     * Signal objects are returned unmodified.
     *
     * @return the next available Data object, returns null if no
     *         Data object is available
     *
     * @throws DataProcessingException if there is an error processing data
     */
    public Data getData() throws DataProcessingException {

        Data output = null;

        if (cepstraList.size() > gicle ) {  // il y avait 0
            output = (Data) cepstraList.remove(0);

        } else {
            reset();
            // read the cepstra of the entire utterance, calculate
            // and apply the cepstral mean
            if (readUtterance() > 0) {
                normalizeList();
                output = (Data) cepstraList.remove(0);//getData();
            }            
        }

        return output;
    }


    /**
     * Reads the cepstra of the entire Utterance into the cepstraList.
     *
     * @return the number cepstra (with Data) read
     *
     * @throws DataProcessingException if an error occurred reading the Data
     */
    private int readUtterance() throws DataProcessingException {

        Data input = null;

        do {
            input = getPredecessor().getData();
            if (input != null) {
                if (input instanceof DoubleData) {
                    numberDataCepstra++;
		    double[] cepstrumData = ((DoubleData) input).getValues();
                    if (sums == null) {
                        sums = new double[cepstrumData.length];
                    } else {
                        if (sums.length != cepstrumData.length) {
                            throw new Error
                                ("Inconsistent cepstrum lengths: sums: " +
                                 sums.length + ", cepstrum: " + 
                                 cepstrumData.length);
                        }
                    }
		    // add the cepstrum data to the sums
		    if (maxi < cepstrumData[0]) maxi = cepstrumData[0];
		    for (int j = 0; j < cepstrumData.length; j++) {
                        sums[j] += cepstrumData[j];
                    }
                    cepstraList.add(input);
		    
                } else if (input instanceof DataEndSignal) {
                    cepstraList.add(input);
                    break;
                } else { // DataStartSignal or other Signal
                    cepstraList.add(input);
                }
            }
        } while (input != null);

        return numberDataCepstra;
    }
        

    /**
     * Normalizes the list of Data.
     */
    private void normalizeList() {

        // calculate the mean first
        for (int i = 0; i < sums.length; i++) {
            sums[i] /= numberDataCepstra;
        }
        if (logger.isLoggable(Level.FINE))
	    logger.fine("normalize "+ cepstraList.size() + "trames");
        for (Iterator iterator = cepstraList.iterator(); iterator.hasNext();) {
            Data cepstrumObject = (Data) iterator.next();
            if (cepstrumObject instanceof DoubleData) {
                double[] cepstrum = ((DoubleData) cepstrumObject).getValues();
		if (agc) cepstrum[0] -= maxi;
		else
		    cepstrum[0] -= sums[0];
		for (int j = 1; j < cepstrum.length; j++) {
                    cepstrum[j] -= sums[j]; // sums[] is now the means[]	  
		}		 
		if (false  && compte <200) {
		    
		    System.out.printf("%3d",compte++);
		    for (double f : cepstrum) 
			System.out.printf(" %9.5f",f);
		    System.out.println();
		}
		
            }
        }
    }
    

}
