/**
 * Copyright 1999-2007 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * <p/>
 * User: Garrett Weinberg
 * Date: Jan 13, 2007
 * Time: 8:19:51 PM
 */

package edu.cmu.sphinx.tools.riddler;

import edu.cmu.sphinx.tools.riddler.types.*;

import java.rmi.Remote;

/**
 * The main Riddler web service interface used to create utterance corpora and build models using them.
 */
public interface Riddler extends Remote {

    public CorpusID createCorpus();

    public void describeCorpus(CorpusID id, CorpusDescriptor desc);

    public CorpusDescriptor getCorpusDescriptor(CorpusID id);

    public UtteranceID createUtterance(byte[] data);

    public void describeUtterance(UtteranceID id, UtteranceDescriptor desc);

// throws an exception if the given UtteranceID has no associated UtteranceDescriptor (needed to calculate time)
//    public int getUtteranceLengthInMilliseconds(UtteranceID id) throws javax.xml.rpc.soap.SOAPFaultException;

    public UtteranceDescriptor getUtteranceDescriptor(UtteranceID id);

    public UtteranceID[] getUtterancesMatchingDescriptor(UtteranceDescriptor desc);

    public void addUtteranceToCorpus(CorpusID corpusID, UtteranceID utteranceID);

    public void addUtterancesToCorpus(CorpusID corpusID, UtteranceID[] utteranceIDs);

    public DocumentID createDocument(String document);

    /**
     * assign the document with the given ID to a particular utterance (implies the document is a
     * word-for-word match to the utterance); replaces any existing assignment
     * @param uttID
     * @param docID
     */
    public void assignDocumentToUtterance(DocumentID docID, UtteranceID uttID);

    public void addDocumentToCorpus(DocumentID docID, CorpusID corpusID);

    public void addDocumentsToCorpus(DocumentID[] docID, CorpusID corpusID);

    /**
     * describe some time-based region of audio
     * @param desc
     * @return
     */
    public AudioRegionID createAudioRegion(AudioRegionDescriptor desc);

    public void addAudioRegionToUtterance(AudioRegionID regionID, UtteranceID uttID);

    public void addAudioRegionsToUtterance(AudioRegionID[] regionID, UtteranceID uttID);

    public WordID createWord(String word);

    public void addPronunciation(WordID wordID, String prons);

    public void addPronunciations(WordID wordID, String[] prons);
}
