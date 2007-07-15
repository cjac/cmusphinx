/** <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * <p/>
 */

package edu.cmu.sphinx.tools.riddler.shared;

import edu.cmu.sphinx.tools.riddler.shared.MetadataWrapper;

import java.net.URI;
import java.rmi.RemoteException;
import java.util.*;
import javax.ejb.Remote;


/**
 * Interface for Riddler, the corpus creation and training tool for Sphinx.
 * @author Garrett Weinberg
 */
@Remote
public interface RiddlerRemote {

    /**
     * Kick off a model training operation, given a suitable (i.e. full) Corpus
     * @param corpusID Corpus from which the models should be trained
     * @return a URI pointing to an RSS feed that will contain the models matching this request
     */
    public URI trainModelsFromCorpus(String corpusID);

    /**
     * Kick off a model training operation that uses multiple Corpora
     * @param corpusIDs Corpora from which the models should be trained
     * @return a URI pointing to an RSS feed that will contain the models matching this request
     */
    public URI trainModelsFromCorpora(ArrayList<String> corpusIDs);

    /**
     * Get a valid identifier for a new Dictionary
     * @return the new Dictionary's identifier
     * @throws RemoteException if a Dictionary with the given metadata already exists @param metadata a map of metadata about the Dictionary
     */
    public String createDictionary(MetadataWrapper metadata) throws RemoteException;

    /**
     * Get a Dictionary matching the provided descriptor
     * @return the existing Dictionary's identifier
     * @throws RemoteException if no Dictionary matching the metadata exists @param metadata metadata for the Dictionary you're trying to retrieve
     */
    public String getDictionary(MetadataWrapper metadata) throws RemoteException;

    /**
     * Fetch the DictionaryDescriptor matching the provided ID
     * @param dictionaryID an existing Dictionary identifier
     * @return the metadata provided when the given Dictionary was created
     * @throws java.rmi.RemoteException if the provided ID is invalid
     */
    public MetadataWrapper getDictionaryMetadata(String dictionaryID) throws RemoteException;

    /**
     * Create a Pronunciation record (case insensitive).
     * @return the new Pronunciation's identifier
     * @throws RemoteException if the word contained in the given PronunciationDescriptor already
     * has an entry in the given dictionary @param dictionaryID Dictionary in which the entry should be created
     * @param word the word to be added to the specified dictionary
     * @param pronunciations main and any variant pronunciations of the word
     */
    public String addPronuncations(String dictionaryID, String word, ArrayList<String> pronunciations) throws RemoteException;

    /**
     * Check whether a Pronunciation record exists for the given word (case insensitive)
     * @param dictionaryID Dictionary in which the entry should be created
     * @param word word that should be queried
     * @return true if the word has at least one pronunciation
     */
    public boolean hasPronuncation(String dictionaryID, String word) throws RemoteException;

    /**
     * Get a valid identifier for a new Corpus
     * @return a new Corpus identifier
     * @param dictionaryID Dictionary with which this Corpus should be associated
     * @param metadata map of metadata about this corpus
     * @param collectDate date with which this corpus should be associated (e.g. date when data collection
     * began or ended)
     */
    public String createCorpus(String dictionaryID, MetadataWrapper metadata, Date collectDate);

    /**
     * Deeply create a new, empty Item record associated with the given Corpus.
     * Also creates corresponding Audio and Text records, each consisting of a single
     * RegionOfAudio and RegionOfText, respectively.
     * @param corpusId Corpus to which the Item should be added
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     */
    public String createItem(String corpusId);

    /**
     * Deeply create a new Item record with exactly one Audio record containing one RegionOfAudio.
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     * @param corpusId Corpus to which the Item should be added
     * @param samplesPerSecond samples per second
     * @param channelCount number of audio channels
     * @param data audio data itself
     */
    public String createItemWithShortAudio(String corpusId, int samplesPerSecond, int channelCount, short[] data);

    /**
     * Deeply create a new Item record with exactly one Audio record containing one RegionOfAudio.     
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     * @param corpusId Corpus to which the Item should be added
     * @param samplesPerSecond samples per second
     * @param channelCount number of audio channels
     * @param data audio data itself
     */
    public String createItemWithByteAudio(String corpusId, int samplesPerSecond, int channelCount, byte[] data);

    /**
     * Deeply create a new Item record with exactly one Audio record containing one RegionOfAudio.
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     * @param corpusId Corpus to which the Item should be added
     * @param samplesPerSecond samples per second
     * @param channelCount number of audio channels
     * @param data audio data itself
     */
    public String createItemWithIntAudio(String corpusId, int samplesPerSecond, int channelCount, int[] data);

    /**
     * Deeply create a new Item record with exactly one Audio record containing one RegionOfAudio.
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     * @param corpusId Corpus to which the Item should be added
     * @param samplesPerSecond samples per second
     * @param channelCount number of audio channels
     * @param data audio data itself
     */
    public String createItemWithLongAudio(String corpusId, int samplesPerSecond, int channelCount, long[] data);

    /**
     * Deeply create a new Item record with exactly one Audio record containing one RegionOfAudio.
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     * @param corpusId Corpus to which the Item should be added
     * @param samplesPerSecond samples per second
     * @param channelCount number of audio channels
     * @param data audio data itself
     */
    public String createItemWithFloatAudio(String corpusId, int samplesPerSecond, int channelCount, float[] data);

    /**
     * Deeply create a new Item record with exactly one Text record containing one RegionOfText.
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     * @throws RemoteException if the words list contains a word not found in the
     * provided Corpus' Dictionary
     * @param corpusId Corpus to which the Item should be added
     * @param words text with which the Item should be created; note that a List is used
     * because not all languages use spaces consistently as word delimiters
     */
    public String createItemWithText(String corpusId, ArrayList<String> words);

    /**
     * Deeply create a new Item record with one Audio record and one Text record.  The Audio
     * record contains one RegionOfAudio that points to the Text record's single RegionOfText.<p/>
     * This method should be used to indicate that the Text is a transcript of the Audio.
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     * @throws RemoteException if the words list contains a word not found in the
     * provided Corpus' Dictionary
     * @param corpusId Corpus to which the Item should be added
     * @param samplesPerSecond samples per second
     * @param channelCount number of audio channels
     * @param data audio data itself
     * @param words transcript of the audio being sent
     */
    public String createItemWithShortAudioAndText(String corpusId, int samplesPerSecond, int channelCount, short[] data, ArrayList<String> words);

    /**
     * Deeply create a new Item record with one Audio record and one Text record.  The Audio
     * record contains one RegionOfAudio that points to the Text record's single RegionOfText.<p/>
     * This method should be used to indicate that the Text is a transcript of the Audio.
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     * @throws RemoteException if the words list contains a word not found in the
     * provided Corpus' Dictionary
     * @param corpusId Corpus to which the Item should be added
     * @param samplesPerSecond samples per second
     * @param channelCount number of audio channels
     * @param data audio data itself
     * @param words transcript of the audio being sent
     */
    public String createItemWithByteAudioAndText(String corpusId,  int samplesPerSecond, int channelCount, byte[] data, ArrayList<String> words);

    /**
     * Deeply create a new Item record with one Audio record and one Text record.  The Audio
     * record contains one RegionOfAudio that points to the Text record's single RegionOfText.<p/>
     * This method should be used to indicate that the Text is a transcript of the Audio.
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     * @throws RemoteException if the words list contains a word not found in the
     * provided Corpus' Dictionary
     * @param corpusId Corpus to which the Item should be added
     * @param samplesPerSecond samples per second
     * @param channelCount number of audio channels
     * @param data audio data itself
     * @param words transcript of the audio being sent
     */
    public String createItemWithIntAudioAndText(String corpusId,  int samplesPerSecond, int channelCount, int[] data, ArrayList<String> words);

    /**
     * Deeply create a new Item record with one Audio record and one Text record.  The Audio
     * record contains one RegionOfAudio that points to the Text record's single RegionOfText.<p/>
     * This method should be used to indicate that the Text is a transcript of the Audio.
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     * @throws RemoteException if the words list contains a word not found in the
     * provided Corpus' Dictionary
     * @param corpusId Corpus to which the Item should be added
     * @param samplesPerSecond samples per second
     * @param channelCount number of audio channels
     * @param data audio data itself
     * @param words transcript of the audio being sent
     */
    public String createItemWithLongAudioAndText(String corpusId,  int samplesPerSecond, int channelCount, long[] data, ArrayList<String> words);

    /**
     * Deeply create a new Item record with one Audio record and one Text record.  The Audio
     * record contains one RegionOfAudio that points to the Text record's single RegionOfText.<p/>
     * This method should be used to indicate that the Text is a transcript of the Audio.
     * @return the ID of an Item with valid deep linkages to its newly-created component records
     * @throws RemoteException if the words list contains a word not found in the
     * provided Corpus' Dictionary
     * @param corpusId Corpus to which the Item should be added
     * @param samplesPerSecond samples per second
     * @param channelCount number of audio channels
     * @param data audio data itself
     * @param words transcript of the audio being sent
     */
    public String createItemWithFloatAudioAndText(String corpusId,  int samplesPerSecond, int channelCount, float[] data, ArrayList<String> words);

    /**
     * Add a RegionOfText to the given Item
     * @return a Text identifier representing the newly created record
     * @param itemID Item record to which the new RegionOfText should be added
     * @param startIndex index of the word at which this text region begins
     * @param endIndex index of the word at which this text region ends
     * @see RiddlerRemote#createItemWithText(String, java.util.ArrayList)
     */
    public String createTextRegion(String itemID, int startIndex, int endIndex);

    /**
     * Add a RegionOfAudio to the given Item
     * @return an Audio identifier representing the newly created record
     * @param itemID Item record to which the new RegionOfAudio should be added
     * @param beginTime time (in milliseconds) within the parent Audio record when this region begins
     * @param endTime time (in milliseconds) within the parent Audio record when this region ends
     */
    public String createAudioRegion(String itemID, int beginTime, int endTime);

    /**
     * Add a RegionOfAudio to the given Item, associating it with the given RegionOfText
     * @return an Audio identifier representing the newly created record
     * @param itemID Item to which the matched regions should be added
     * @param beginTime time (in milliseconds) within the parent Audio record when this region begins
     * @param endTime time (in milliseconds) within the parent Audio record when this region ends
     * @param startIndex index of the word at which this text region begins
     * @param endIndex index of the word at which this text region ends
     */
    public String createAudioRegionWithText(String itemID, int beginTime, int endTime, int startIndex, int endIndex);

    /**
     * Link a pre-existing audio region to a pre-existing text region
     * @param audioID ID of an Audio record
     * @param textID ID of a Text record
     * @throws RemoteException if either the audio or text ID's are invalid
     */
    public void associateAudioRegionWithText(String audioID, String textID);
}
