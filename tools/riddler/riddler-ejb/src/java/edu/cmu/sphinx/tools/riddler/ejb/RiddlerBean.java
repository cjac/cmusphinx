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
    public URI trainModelsFromCorpus(int id) {
        return null;
    }

    @WebMethod
    public URI trainModelsFromCorpora(ArrayList<Integer> IDs) {
        return null;
    }

    @WebMethod
    public int createDictionary(MetadataWrapper metadata) {
        return 0;
    }

    @WebMethod
    public int getDictionary(MetadataWrapper metadata) {
        return 0;
    }

    @WebMethod
    public MetadataWrapper getDictionaryMetadata(int id) {
        return null;
    }

    @WebMethod
    public int createPronuncation(int id, String word, ArrayList<String> pronunciations) {
        return 0;
    }

    @WebMethod
    public boolean hasPronuncation(int id, String word) {
        return false;
    }

    @WebMethod
    public int createCorpus(int dictId, MetadataWrapper metadata, Date collectDate) {
        return 0;
    }

    @WebMethod
    public int createItem(int corpusId) {
        return 0;
    }

    @WebMethod
    public int createItemWithShortAudio(int corpusId, int samplesPerSecond, int channelCount, short[] data) {
        return 0;
    }

    @WebMethod
    public int createItemWithByteAudio(int corpusId, int samplesPerSecond, int channelCount, byte[] data) {
        return 0;
    }

    @WebMethod
    public int createItemWithIntAudio(int corpusId, int samplesPerSecond, int channelCount, int[] data) {
        return 0;
    }

    @WebMethod
    public int createItemWithLongAudio(int corpusId, int samplesPerSecond, int channelCount, long[] data) {
        return 0;
    }

    @WebMethod
    public int createItemWithFloatAudio(int corpusId, int samplesPerSecond, int channelCount, float[] data) {
        return 0;
    }

    @WebMethod
    public int createItemWithText(int corpusId, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public int createItemWithShortAudioAndText(int corpusId, int samplesPerSecond, int channelCount, short[] data, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public int createItemWithByteAudioAndText(int corpusId, int samplesPerSecond, int channelCount, byte[] data, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public int createItemWithIntAudioAndText(int corpusId, int samplesPerSecond, int channelCount, int[] data, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public int createItemWithLongAudioAndText(int corpusId, int samplesPerSecond, int channelCount, long[] data, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public int createItemWithFloatAudioAndText(int corpusId, int samplesPerSecond, int channelCount, float[] data, ArrayList<String> words) {
        return 0;
    }

    @WebMethod
    public int createTextRegion(int id, int startIndex, int endIndex) {
        return 0;
    }

    @WebMethod
    public int createAudioRegion(int id, int beginTime, int endTime) {
        return 0;
    }

    @WebMethod
    public int createAudioRegionWithText(int itemID, int beginTime, int endTime, int startIndex, int endIndex) {
        return 0;
    }

    @WebMethod
    public void associateAudioRegionWithText(int audioID, int textID) {

    }
}