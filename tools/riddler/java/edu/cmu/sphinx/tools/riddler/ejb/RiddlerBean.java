/*
 * RiddlerBean.java
 *
 * Created on February 10, 2007, 9:22 PM
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * <p/>
 */

package edu.cmu.sphinx.tools.riddler.ejb;

import edu.cmu.sphinx.tools.riddler.persist.Dictionary;
import edu.cmu.sphinx.tools.riddler.persist.Pronunciation;
import edu.cmu.sphinx.tools.riddler.persist.StringIdentified;
import edu.cmu.sphinx.tools.riddler.persist.Metadatum;
import edu.cmu.sphinx.tools.riddler.shared.RiddlerRemote;
import edu.cmu.sphinx.tools.riddler.shared.MetadataWrapper;

import javax.ejb.Remote;
import javax.ejb.Stateless;
import javax.ejb.EJBException;
import javax.jws.WebMethod;
import javax.jws.WebService;
import javax.persistence.*;
import java.net.URI;
import java.rmi.RemoteException;
import java.util.*;
import java.util.logging.Logger;

/**
 * Stateless session bean / web service implementation of Riddler
 * @author Garrett Weinberg
 */
@Stateless
@Remote(RiddlerRemote.class)
@WebService
public class RiddlerBean implements RiddlerRemote {

    @PersistenceContext
    private EntityManager em;
    private Logger logger = Logger.getLogger(getClass().getName());

    public RiddlerBean() {
    }

    @WebMethod
    public URI trainModelsFromCorpus(String corpusID) {
        return null;
    }

    @WebMethod
    public URI trainModelsFromCorpora(ArrayList<String> corpusIDs) {
        return null;
    }

    /**
     *
     * @param metadata
     * @param queryName
     * @return the first object (e.g. Corpus, Dictionary) having the provided metadata,
     * or null if none exist; <p/>
     * note that there should be only one object of a given entity type having exactly
     * the same set of both keys and values in its metadata. <p/>
     * this is why we invoke this method before creating such objects to see if duplicate
     * objects already exist.
     */
    private StringIdentified findByMetadata(MetadataWrapper metadata, String queryName, Class<? extends StringIdentified> queriedClass) {
        // maps a given Dictionary or Corpus to a counter for the number of matching metadata fields.
        // if all match, that is the sought object
        final Map<String, Integer> occurrences = new HashMap<String, Integer>();

        for (Map.Entry<String, String> metadatum : metadata.getContents().entrySet()) {
            final Query q = em.createNamedQuery(queryName);
            q.setParameter("key", metadatum.getKey());
            q.setParameter("value", metadatum.getValue());
            List<StringIdentified> results = q.getResultList();
            for (StringIdentified result : results) {
                if (occurrences.containsKey(result.getId())) {
                    // increment the occurence count for this particular key/value pair
                    occurrences.put(result.getId(), occurrences.get(result.getId())+1);
                }
                else
                    occurrences.put(result.getId(), 1); // start the occurrence counter at one
            }
        }

        for (Map.Entry<String, Integer> occurrence : occurrences.entrySet()) {
            // return the first one with a full occurrence count
            if (occurrence.getValue() == metadata.getContents().size())
                return em.find(queriedClass, occurrence.getKey());
        }
        return null;
    }

    @WebMethod
    public String createDictionary(MetadataWrapper metadata) throws RemoteException {
        final Dictionary existing = (Dictionary) findByMetadata(metadata, "findDictionariesByMetadatum", edu.cmu.sphinx.tools.riddler.persist.Dictionary.class);
        if (existing != null) {
            throw new EJBException("A dictionary with the provided metadata '" +
                    metadata + "' already exists. Its ID is " + existing.getId());
        }
        Dictionary d;
        d = new edu.cmu.sphinx.tools.riddler.persist.Dictionary();
        d.setMetadata(Metadatum.listFromWrapper(metadata));
        em.persist(d);
        d = em.merge(d);
        return d.getId();
    }

    @WebMethod
    public String getDictionary(MetadataWrapper metadata) throws RemoteException {
        final edu.cmu.sphinx.tools.riddler.persist.Dictionary existing = (Dictionary) findByMetadata(metadata, "findDictionariesByMetadatum", Dictionary.class);
        if (existing == null)
            throw new EJBException("No dictionary with the provided metadata '" + metadata + "' exists.");
        return existing.getId();
    }

    @WebMethod
    public MetadataWrapper getDictionaryMetadata(String dictionaryID) throws RemoteException {
        Dictionary d = fetchDictionary(dictionaryID);
        return Metadatum.wrapperFromList(d.getMetadata());
    }

    @WebMethod
    public String addPronuncations(String dictionaryID, String word, ArrayList<String> pronunciations) throws RemoteException {
        Dictionary d = fetchDictionary(dictionaryID);
        Pronunciation p;
        try {
            final Query q = em.createNamedQuery("findPronunciationByWord");
            q.setParameter("word", word);
            q.setParameter("dictionary", d);
            p = (Pronunciation) q.getSingleResult();

            // Pronounciation found: add all the words in the input parameter
            HashSet<String> variants = p.getVariants();
            variants.addAll(pronunciations);
            p.setVariants(variants);
            p = em.merge(p);
        }
        catch (NonUniqueResultException nure) {
            throw new IllegalStateException("multiple Pronunciations for the word '" + word + "' found in Dictionary " + d);
        }
        catch (NoResultException nre) {
            // no Pronunciation found: create a new record
            p = new Pronunciation(word, new HashSet<String>(pronunciations));
            em.persist(p);
            p = em.merge(p);
        }
        return p.getId();
    }

    @WebMethod
    public boolean hasPronuncation(String dictionaryID, String word) throws RemoteException {
        edu.cmu.sphinx.tools.riddler.persist.Dictionary d = fetchDictionary(dictionaryID);
        try {
            final Query q = em.createNamedQuery("findPronunciationByWord");
            q.setParameter("word", word);
            q.setParameter("dictionary", d);
            q.getSingleResult();
            return true;
        }
        catch (NonUniqueResultException nure) {
            throw new IllegalStateException("multiple Pronunciations for the word '" + word + "' found in Dictionary " + d);
        }
        catch (NoResultException nre) {
            return false;
        }
    }

    @WebMethod
    public String createCorpus(String dictionaryID, MetadataWrapper metadata, Date collectDate) {
        return null;
    }

    @WebMethod
    public String createItem(String corpusId) {
        return null;
    }

    @WebMethod
    public String createItemWithShortAudio(String corpusId, int samplesPerSecond, int channelCount, short[] data) {
        return null;
    }

    @WebMethod
    public String createItemWithByteAudio(String corpusId, int samplesPerSecond, int channelCount, byte[] data) {
        return null;
    }

    @WebMethod
    public String createItemWithIntAudio(String corpusId, int samplesPerSecond, int channelCount, int[] data) {
        return null;
    }

    @WebMethod
    public String createItemWithLongAudio(String corpusId, int samplesPerSecond, int channelCount, long[] data) {
        return null;
    }

    @WebMethod
    public String createItemWithFloatAudio(String corpusId, int samplesPerSecond, int channelCount, float[] data) {
        return null;
    }

    @WebMethod
    public String createItemWithText(String corpusId, ArrayList<String> words) {
        return null;
    }

    @WebMethod
    public String createItemWithShortAudioAndText(String corpusId, int samplesPerSecond, int channelCount, short[] data, ArrayList<String> words) {
        return null;
    }

    @WebMethod
    public String createItemWithByteAudioAndText(String corpusId, int samplesPerSecond, int channelCount, byte[] data, ArrayList<String> words) {
        return null;
    }

    @WebMethod
    public String createItemWithIntAudioAndText(String corpusId, int samplesPerSecond, int channelCount, int[] data, ArrayList<String> words) {
        return null;
    }

    @WebMethod
    public String createItemWithLongAudioAndText(String corpusId, int samplesPerSecond, int channelCount, long[] data, ArrayList<String> words) {
        return null;
    }

    @WebMethod
    public String createItemWithFloatAudioAndText(String corpusId, int samplesPerSecond, int channelCount, float[] data, ArrayList<String> words) {
        return null;
    }

    @WebMethod
    public String createTextRegion(String itemID, int startIndex, int endIndex) {
        return null;
    }

    @WebMethod
    public String createAudioRegion(String itemID, int beginTime, int endTime) {
        return null;
    }

    @WebMethod
    public String createAudioRegionWithText(String itemID, int beginTime, int endTime, int startIndex, int endIndex) {
        return null;
    }

    @WebMethod
    public void associateAudioRegionWithText(String audioID, String textID) {

    }


    private edu.cmu.sphinx.tools.riddler.persist.Dictionary fetchDictionary(String dictionaryID) throws RemoteException {
        Dictionary d = em.find(Dictionary.class, dictionaryID);
        if (d == null)
            throw new RemoteException("no Dictionary with ID " + dictionaryID + " exists");
        else
            return d;
    }
}