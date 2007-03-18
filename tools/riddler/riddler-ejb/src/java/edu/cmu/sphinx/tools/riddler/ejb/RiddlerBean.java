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

import java.net.URI;
import java.util.*;
import javax.ejb.Stateless;
import javax.ejb.Remote;
import javax.jws.WebMethod;
import javax.jws.WebService;

/**
 * Stateless session bean / web service implementation of Riddler
 * @author Garrett Weinberg
 */
@Stateless
@Remote(RiddlerRemote.class)
@WebService
public class RiddlerBean implements RiddlerRemote {

    public RiddlerBean() {
    }

    @WebMethod
    public URI trainModelsFromCorpus(long corpusID) {
        return null;
    }

    @WebMethod
    public URI trainModelsFromCorpora(ArrayList<Integer> corpusIDs) {
        return null;
    }

    @WebMethod
    public long createDictionary(MetadataWrapper metadata) {
        return 0;
    }

    @WebMethod
    public long getDictionary(MetadataWrapper metadata) {
        return 0;
    }

    @WebMethod
    public MetadataWrapper getDictionaryMetadata(long dictionaryID) {
        return null;
    }

    @WebMethod
    public long createPronuncation(long dictionaryID, String word, ArrayList<String> pronunciations) {
        return 0;
    }

    @WebMethod
    public boolean hasPronuncation(long dictionaryID, String word) {
        return false;
    }

    @WebMethod
    public long createCorpus(long dictionaryID, MetadataWrapper metadata, Date collectDate) {
        return 0;
    }

    @WebMethod
    public long createItem(long corpusId) {
        return 0;
    }

    @WebMethod
    public long createItemWithShortAudio(long corpusId, int samplesPerSecond, int channelCount, short[] data) {
        return 0;
    }

    @WebMethod
    public long createItemWithByteAudio(long corpusId, int samplesPerSecond, int channelCount, byte[] data) {
        return 0;
    }

    @WebMethod
    public long createItemWithIntAudio(long corpusId, int samplesPerSecond, int channelCount, int[] data) {
        return 0;
    }

    @WebMethod
    public long createItemWithLongAudio(long corpusId, int samplesPerSecond, int channelCount, long[] data) {
        return 0;
    }

    @WebMethod
    public long createItemWithFloatAudio(long corpusId, int samplesPerSecond, int channelCount, float[] data) {
        return 0;
    }

    @WebMethod
    public long createItemWithText(long corpusId, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public long createItemWithShortAudioAndText(long corpusId, int samplesPerSecond, int channelCount, short[] data, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public long createItemWithByteAudioAndText(long corpusId, int samplesPerSecond, int channelCount, byte[] data, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public long createItemWithIntAudioAndText(long corpusId, int samplesPerSecond, int channelCount, int[] data, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public long createItemWithLongAudioAndText(long corpusId, int samplesPerSecond, int channelCount, long[] data, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public long createItemWithFloatAudioAndText(long corpusId, int samplesPerSecond, int channelCount, float[] data, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public long createTextRegion(long itemID, int startIndex, int endIndex) {
        return 0;
    }

    @WebMethod
    public long createAudioRegion(long itemID, int beginTime, int endTime) {
        return 0;
    }

    @WebMethod
    public long createAudioRegionWithText(long itemID, int beginTime, int endTime, int startIndex, int endIndex) {
        return 0;
    }

    @WebMethod
    public void associateAudioRegionWithText(long audioID, long textID) {

    }
}