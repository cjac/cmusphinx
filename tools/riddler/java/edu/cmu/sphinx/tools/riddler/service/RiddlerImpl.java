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

package edu.cmu.sphinx.tools.riddler.service;

import edu.cmu.sphinx.tools.riddler.types.*;

import javax.jws.WebMethod;
import javax.jws.WebService;
import javax.jws.soap.SOAPBinding;
import java.net.URI;
import java.rmi.RemoteException;

/**
 * Main Riddler implementation class
 */
@WebService(
        name = "Riddler",
        targetNamespace = "http://edu.cmu.sphinx/riddler",
        serviceName = "RiddlerService")
@SOAPBinding(style = SOAPBinding.Style.RPC)
public class RiddlerImpl implements Riddler {

    @WebMethod
    public URI trainModelsFromCorpus(CorpusID id) {
        return null;
    }

    @WebMethod
    public URI trainModelsFromCorpora(CorpusID[] IDs) {
        return null;
    }

    @WebMethod
    public DictionaryID createDictionary(DictionaryDescriptor desc) throws RemoteException {
        return null;
    }

    @WebMethod
    public DictionaryID getDictionary(DictionaryDescriptor desc) throws RemoteException {
        return null;
    }

    @WebMethod
    public DictionaryDescriptor getDictionaryDescriptor(DictionaryID id) {
        return null;
    }

    @WebMethod
    public PronunciationID createPronuncation(DictionaryID id, PronunciationDescriptor desc) throws RemoteException {
        return null;
    }

    @WebMethod
    public boolean hasPronuncation(DictionaryID id, String word) {
        return false;
    }

    @WebMethod
    public CorpusID createCorpus(DictionaryID dictId, CorpusDescriptor desc) {
        return null;
    }

    @WebMethod
    public CorpusDescriptor getCorpusDescriptor(CorpusID id) {
        return null;
    }

    @WebMethod
    public ItemID createItem(CorpusID corpusId) {
        return null;
    }

    @WebMethod
    public ItemID createItemWithAudio(CorpusID corpusId, AudioDescriptor desc) {
        return null;
    }

    @WebMethod
    public ItemID createItemWithText(CorpusID corpusId, TextDescriptor desc) throws RemoteException {
        return null;
    }

    @WebMethod
    public ItemID createItemWithAudioAndText(CorpusID corpusId, AudioDescriptor audioDesc, TextDescriptor textDesc) throws RemoteException {
        return null;
    }

    @WebMethod
    public RegionOfTextID createTextRegion(ItemID id, RegionOfTextDescriptor desc) {
        return null;
    }

    @WebMethod
    public RegionOfAudioID createAudioRegion(ItemID id, RegionOfAudioDescriptor desc) {
        return null;
    }

    @WebMethod
    public RegionOfAudioID createAudioRegionWithText(ItemID id, RegionOfTextID textRegionID, RegionOfAudioDescriptor audioDesc) {
        return null;
    }
}
