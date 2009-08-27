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

import java.io.IOException;
import java.util.List;



/**
 * A simple implementation of the batch manager suitable for single
 * threaded batch processing
 *
 */
public class SimpleBatchManager implements BatchManager {
    private String batchFile;
    private int skip;
    private int beginSkip;
    private int whichBatch;
    private int totalBatches;
    private int curItem;
    private List <String> items;
    private boolean modeCtl;
    /**
     * Constructs a SimpleBatchManager object.
     *
     * @param filename      the name of the batch file
     * @param skip          number of records to skip between items
     * @param whichBatch    which chunk of the batch should we process
     * @param totalBatches  the total number of chuncks that the batch
     *                      is divided into.
     */
    public SimpleBatchManager(String filename,
            int skip, int whichBatch, int totalBatches) {
        this.batchFile = filename;
        this.skip = skip;
        this.whichBatch = whichBatch;
        this.totalBatches = totalBatches;
        this.modeCtl =false;
        this.beginSkip =0;
    }

 /**
     * Constructs a SimpleBatchManager object.
     *
     * @param filename      the name of the batch file
     * @param skip          number of records to skip between items
     * @param whichBatch    which chunk of the batch should we process
     * @param totalBatches  the total number of chuncks that the batch
     *                      is divided into.
     *@param modeCtl       as name 
     */
  public SimpleBatchManager(String filename,
			    int skip, int whichBatch, int totalBatches, 
			    boolean modeCtl,int beginSkip) {
        this.batchFile = filename;
        this.skip = skip;
        this.beginSkip=beginSkip;
        this.whichBatch = whichBatch;
        this.totalBatches = totalBatches;
        this.modeCtl = modeCtl;
    }
    /**
     * Starts processing the batch
     *
     */
    public void start() throws IOException {
        curItem = 0;
        items = getBatchItems(batchFile);
    }

    /**
     * Gets the next available batch item or null if no more are
     * available
     *
     * @return the next available batch item
     *
     * @throws IOException if an I/O error occurs while getting the
     * next item from the batch file.
     */
    public BatchItem getNextItem() throws IOException {
        if (curItem >= items.size()) {
            return null;
        } else {
            String line = (String) items.get(curItem++);
            if (modeCtl) 
		return new BatchItem(line.split(" +"));
           else
            return new BatchItem(BatchFile.getFilename(line),
                            BatchFile.getReference(line));
        }
    }

    /**
     * Stops processing the batch
     */
    public void stop() throws IOException {
    }

    /**
     * Returns the name of the file
     *
     * @return the filename
     */
    public String getFilename() {
        return batchFile;
    }

    /**
     * Gets the set of lines from the file
     *
     * @param file the name of the file 
     */
    private List <String> getBatchItems(String file) throws IOException {
	List <String> list = BatchFile.getLines(file, skip,beginSkip);

	if (totalBatches > 1) {
	    if (modeCtl) {
		//j'essaye un découpage en temps.
		int total=0;
		for (String line : list) {
		    String tab[]=line.split(" +");
		    total+= Integer.parseInt(tab[2]) - Integer.parseInt(tab[1]);
		}
		int dureePerBatch = total / totalBatches;
		int start=dureePerBatch*whichBatch;
		int fin =dureePerBatch*(whichBatch+1);
		int startLine=0,endLine=0;
		System.err.format("total :%d, duree:%d, start:%d, fin:%d\n",total, dureePerBatch,start,fin);
		total=0;
		int numero=0;
		for (String line:list) {
		    String tab[]=line.split(" +");
	// 	    for(String s:tab) System.err.format("|%s|",s);
// 		    System.err.println();
		    total+= Integer.parseInt(tab[2]) - Integer.parseInt(tab[1]);
		    if (total>=fin) { endLine=numero-2; break;}
		    if (total< start) startLine=numero;
		    numero++;		    
		}
		if (whichBatch==(totalBatches-1))
		    endLine=list.size();
		System.err.println(" batchs  start ==>"+startLine+ " end==>"+ endLine);
		return list.subList(startLine,endLine);



	    }
	    int linesPerBatch = list.size() / totalBatches;
	    if (linesPerBatch < 1) {
		linesPerBatch = 1;
	    }
	    if (whichBatch >= totalBatches) {
		whichBatch = totalBatches - 1;
	    }
	    int startLine = whichBatch * linesPerBatch;
	    // last batch needs to get all remaining lines
	    if (whichBatch == (totalBatches - 1)) {
		list = list.subList(startLine, list.size());
	    } else {
		list = list.subList(startLine, startLine +
			linesPerBatch);
	    }
	}
	return list;
    }
}

