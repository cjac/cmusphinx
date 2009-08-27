/*
 * Copyright 1999-2004 Carnegie Mellon University.
 * Portions Copyright 2004 Sun Microsystems, Inc.
 * Portions Copyright 2004 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 *
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 *
 * Created on Aug 11, 2004
 */

package edu.cmu.sphinx.result;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Set;
import java.util.Vector;
import java.util.Locale;
import java.util.ArrayList;
import edu.cmu.sphinx.util.LogMath;
import java.io.File;
import java.util.regex.Pattern;

/**
 * A Sausage is a sequence of confusion sets, one for each position in an utterance. 
 * 
 * @author pgorniak
 */

public class Sausage implements ConfidenceResult {
    protected List<ConfusionSet> confusionSets;
    


    public Sausage(File f,float seuil,float aveceps ,float sanseps) throws IOException{
	confusionSets=new ArrayList<ConfusionSet>();
	BufferedReader reader = new BufferedReader(new FileReader(f));
        Pattern p=Pattern.compile("\\s+");
        String aline = null, line = null;
        int aid=-1;
	boolean haseps=false;
	while ((line = reader.readLine()) != null) {
            if (line.length() > 0) {
		
	        String l[]=p.split(line);
		if (l.length<3) {
		    System.err.println("ligne courte "+line+"\n");
		continue;
	    }
		int id1 = Integer.parseInt(l[0]);
		int id2 = Integer.parseInt(l[1]);
		if (l[2].equals("!NULL")) break;
		Double proba = (Double.parseDouble(l[3]));
		String m=l[2];
		if (aid!=id1) haseps=false;
		if (m.equals("eps") ){
		    haseps=true;
		    if (proba<seuil) continue;
		}else
		    {
			if (haseps && proba<aveceps) continue;
			if (!haseps && proba<sanseps) continue;
		    }


		if (confusionSets.size()<=id1) confusionSets.add(new ConfusionSet());
		addWordHypothesis(id1,m,proba,null);
		aid =id1;
	    }
	}
	reader.close();
    }



    /**
     * Construct a new sausage.
     * 
     * @param size The number of word slots in the sausage
     */
    public Sausage(int size) {
        confusionSets = new Vector<ConfusionSet>(size);
        for (int i=0;i<size;i++) {
            confusionSets.add(new ConfusionSet());
        }
    }
    
    /**
     * Get an iterator for the sausage. The iterator will return
     * SortedMaps, which are confusion sets mapping Double posteriors
     * to Sets of word Strings.
     * 
     * @return an iterator that steps through confusion sets
     */
    public Iterator confusionSetIterator() {
        return confusionSets.iterator();
    }
    
    /**
     * Adds skip elements for each word slot in which the word posteriors do not
     * add up to linear 1.
     * 
     * @param logMath the log math object to use for probability computations
     */
    public void fillInBlanks(LogMath logMath) {
        for (ListIterator i=confusionSets.listIterator();i.hasNext();) {
            int index = i.nextIndex();
            ConfusionSet set = (ConfusionSet)i.next();
            float sum = LogMath.getLogZero();
            for (Iterator <WordResult>  j=set.iterator();j.hasNext();) {
                sum = logMath.addAsLinear(sum,(float)j.next().getConfidence());
            }
            if (sum < LogMath.getLogOne() - 20) {
                float remainder = logMath.subtractAsLinear
                    (LogMath.getLogOne(), sum);
                addWordHypothesis(index,"eps",remainder, logMath);

            }

	    set.sort();
// 	    else 
// 		if (false)	{
// 		    ConfusionSet newSet = new ConfusionSet();
// 		    for (Iterator j=set.keySet().iterator();j.hasNext();) {
// 			Double oldProb = (Double)j.next();
// 			Double newProb = new Double(oldProb.doubleValue() - sum);
// 			newSet.put(newProb,set.get(oldProb));
// 		    }
// 		    confusionSets.set(index,newSet);
// 		}

	    
        }
    }
    
    /**
     * Add a word hypothesis to a given word slot in the sausage.
     * 
     * @param position the position to add a hypothesis to
     * @param word the word to add
     */
    public void addWordHypothesis(int position, WordResult word) {
        getConfusionSet(position).addWordHypothesis(word);
    }

    public void addWordHypothesis(int position, String word, 
                                  double confidence, LogMath logMath) {
        WordResult wr = new SimpleWordResult(word, confidence, logMath);
        addWordHypothesis(position,wr);
    }
    
    /**
     * @see edu.cmu.sphinx.result.ConfidenceResult#getBestHypothesis()
     */
    public Path getBestHypothesis() {
        WordResultPath path = new WordResultPath();
        Iterator i = confusionSetIterator();
        while (i.hasNext()) {
            ConfusionSet cs = (ConfusionSet)i.next();
            path.add(cs.getBestHypothesis());
        }
        return path;
    }
    
    /**
     * Get a string representing the best path through the sausage.
     * 
     * @return best string
     */
    public String getBestHypothesisString() {
        return getBestHypothesis().toString();
    }
    
    /**
     * Get the word hypothesis with the highest posterior for a word slot
     * 
     * @param pos the word slot to look at
     * @return the word with the highest posterior in the slot
     */
    public WordResult getBestWordHypothesis(int pos) {        
        ConfusionSet set = (ConfusionSet)confusionSets.get(pos);
        return set.get(0);
    }

    /**
     * Get the the highest posterior for a word slot
     * 
     * @param pos the word slot to look at
     * @return the highest posterior in the slot
     */

    public double getBestWordHypothesisPosterior(int pos) {
        ConfusionSet set = (ConfusionSet)confusionSets.get(pos);
        return set.get(0).getConfidence();        
    }
    
    /**
     * Get the confusion set stored in a given word slot.
     * 
     * @param pos the word slot to look at.
     * @return a map from Double posteriors to Sets of String words,
     *         sorted from lowest to highest.
     */
    public ConfusionSet getConfusionSet(int pos) {
        return (ConfusionSet)confusionSets.get(pos);
    }
    
    /**
     * size of this sausage in word slots.
     * 
     * @return The number of word slots in this sausage
     */
    public int size() {
        return confusionSets.size();
    }
    
    public void dumpFSM(java.io.File file,float limEps,float limWord) {
	try	{
	    java.io.PrintWriter f = new java.io.PrintWriter(file);
	    int count =0;
	    for (ConfusionSet set : confusionSets) {
		WordResult best =set.getBestHypothesis();
		if (!best.toString().equals("esp") || 
		    (set.size()>=2 && set.get(1).getConfidence()>limWord))
		    for (WordResult wr :set) {
			if (!best.toString().equals("esp")&& wr.getConfidence()<limWord) break;
                        if (!best.toString().equals("esp") || wr.getConfidence()>limEps)
			    f.printf(Locale.US,"%-4d %-4d %-10s %.3g\n",
				     count,count+1,wr.toString(), wr.getLogMath().logToLinear((float)wr.getConfidence()));
		    }
		count++;
	    }
	    f.printf(Locale.US,"%-4d %-4d %-10s %.3g\n",
		     count,count+1,"!NULL",1.0f);
	    f.println(count+1);
	    f.close();
	}
	catch (IOException e) {e.printStackTrace();
	    throw new Error(e.toString());}
    }


    /**
     * Write this sausage to an aisee format text file.
     * @param fileName The file to write to.
     * @param title the title to give the graph.
     */
    public void dumpAISee(String fileName, String title) {
     
       
        try {
            System.err.println("Dumping " + title + " to " + fileName);
            FileWriter f = new FileWriter(fileName);
            f.write("graph: {\n");
            f.write("title: \"" + title + "\"\n");
            f.write("display_edge_labels: yes\n");
            f.write( "orientation: left_to_right\n");
            ListIterator i = confusionSets.listIterator();
            while (i.hasNext()) {
                int index = i.nextIndex();
                ConfusionSet set = (ConfusionSet)i.next();
                Iterator <WordResult> j = set.iterator();
                f.write("node: { title: \"" + index + "\" label: \"" + index + "\"}\n");
		
                while (j.hasNext()) {
                    WordResult wr = j.next();
		    double pr;
		    if (wr.getLogMath()!=null)
			pr= wr.getLogMath().logToLinear((float)wr.getConfidence());
		    else
			pr=wr.getConfidence();
		String  prob=String.format(Locale.US,"%.3g",pr);
                    f.write("edge: { sourcename: \"" + index
                            + "\" targetname: \"" + (index + 1)
                            + "\" label: \"" + wr + ": " + prob + "\" }\n");
                }
            }
            f.write("node: { title: \"" + size() + "\" label: \"" + size() + "\"}\n");            
            f.write("}\n");
            f.close();
        } catch (IOException e) {
            throw new Error(e.toString());
        }
    }
}
