/*
 * Copyright 2004 Carnegie Mellon University.  
 * Portions Copyright 2004 Sun Microsystems, Inc.  
 * Portions Copyright 2004 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 * 
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL 
 * WARRANTIES.
 *
 */

package edu.cmu.sphinx.util;


/**
 * Provides a standard interface to for a single decode in a batch of
 * decodes
 *
 */
public class BatchItem {

    private String filename;
    private String transcript;
    private int startSentence;
    private int endSentence;
    private String id;
    private boolean ctl=false;

    /**
     * Creates a batch item 
     *
     * @param filename the filename
     * @param transcript the transcript
     */
    public BatchItem(String filename, String transcript) {
        this.filename = filename;
        this.transcript = transcript;
    }

    /**
     * Creates a batch item 
     * for ctl
     * @param ligne the ligne splitte
      */
    public BatchItem(String[] compose ) {
	this.filename= compose[0];
        this.startSentence = Integer.parseInt(compose[1]);
        this.endSentence = Integer.parseInt(compose[2]);
        if (compose.length>3)
	    this.id = compose[3];
	if (compose.length >4) {
	    StringBuffer sb=new StringBuffer(compose[4]);
	    for (int i=5 ; i< compose.length -1; i++)
		sb.append(" "+ compose[i]);
	    this.transcript=sb.toString();
	    if (compose.length !=5 && !compose[compose.length-1].equals("("+this.id+")"))
		throw new Error("incoherence :" + compose[compose.length-1]+ " "+this.id);

	}
	this.ctl=true;
    }

    /**
     * Gets the filename for this batch
     *
     * @return the file name
     */
    public String getFilename() {
        return filename;
    }


    /**
     * Gets the transcript for the batch
     *
     * @return the transcript (or null if there is no transcript)
     */
    public String getTranscript() {
        return transcript;
    }

	public int getStartSentence() {
		return startSentence;
	}

	public int getEndSentence() {
		return endSentence;
	}

	public String getId() {
		return id;
	}
}
  
