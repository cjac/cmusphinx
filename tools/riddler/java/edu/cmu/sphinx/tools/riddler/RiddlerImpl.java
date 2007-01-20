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
 * Date: Jan 20, 2007
 * Time: 11:01:38 AM
 */

package edu.cmu.sphinx.tools.riddler;

import edu.cmu.sphinx.tools.riddler.types.*;

import javax.jws.WebMethod;
import javax.jws.WebService;
import javax.jws.soap.SOAPBinding;
import java.net.URI;

/**
 * document me!
 */
@WebService(
        name = "Riddler",
        targetNamespace = "http://edu.cmu.sphinx/riddler",
        serviceName = "RiddlerService")
@SOAPBinding(style = SOAPBinding.Style.RPC)
public class RiddlerImpl implements Riddler {

    @WebMethod
    public CorpusID createCorpus() {
        return null;
    }

    @WebMethod
    public void describeCorpus(CorpusID id, CorpusDescriptor desc) {
    }

    @WebMethod
    public CorpusDescriptor getCorpusDescriptor(CorpusID id) {
        return null;
    }

    @WebMethod
    public UtteranceID createUtterance(byte[] data) {
        return null;
    }

    @WebMethod
    public void describeUtterance(UtteranceID id, UtteranceDescriptor desc) {
    }

    @WebMethod
    public UtteranceDescriptor getUtteranceDescriptor(UtteranceID id) {
        return null;
    }

    @WebMethod
    public UtteranceID[] getUtterancesMatchingDescriptor(UtteranceDescriptor desc) {
        return new UtteranceID[0];  //To change body of implemented methods use File | Settings | File Templates.
    }

    @WebMethod
    public void addUtteranceToCorpus(CorpusID corpusID, UtteranceID utteranceID) {
    }

    @WebMethod
    public void addUtterancesToCorpus(CorpusID corpusID, UtteranceID[] utteranceIDs) {
    }

    @WebMethod
    public DocumentID createDocument(String document) {
        return null;
    }

    @WebMethod
    public void assignDocumentToUtterance(DocumentID docID, UtteranceID uttID) {
    }

    @WebMethod
    public void addDocumentToCorpus(DocumentID docID, CorpusID corpusID) {
    }

    @WebMethod
    public void addDocumentsToCorpus(DocumentID[] docID, CorpusID corpusID) {
    }

    @WebMethod
    public AudioRegionID createAudioRegion(AudioRegionDescriptor desc) {
        return null;
    }

    @WebMethod
    public void addAudioRegionToUtterance(AudioRegionID regionID, UtteranceID uttID) {
    }

    @WebMethod
    public void addAudioRegionsToUtterance(AudioRegionID[] regionID, UtteranceID uttID) {
    }

    @WebMethod
    public WordID createWord(String word) {
        return null;
    }

    @WebMethod
    public void addPronunciation(WordID wordID, String prons) {
    }

    @WebMethod
    public void addPronunciations(WordID wordID, String[] prons) {
    }

    @WebMethod
    public URI trainModelsFromCorpus(CorpusID id) {
        return null;
    }

    @WebMethod
    public URI trainModelsFromCorpora(CorpusID[] id) {
        return null;
    }
}
