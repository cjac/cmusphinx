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

package edu.cmu.sphinx.tools.riddler.service;

import edu.cmu.sphinx.tools.riddler.types.*;

import java.net.URI;
import java.rmi.Remote;
import java.rmi.RemoteException;

/**
 * The main Riddler web service interface used to create utterance corpora and train models.
 */
public interface Riddler extends Remote {

    /**
     * Kick off a model training operation, given a suitable (i.e. full) Corpus
     * @param id Corpus from which the models should be trained
     * @return a URI pointing to an RSS feed that will contain the models matching this request
     */
    public URI trainModelsFromCorpus(CorpusID id);

    /**
     * Kick off a model training operation that uses multiple Corpora
     * @param IDs Corpora from which the models should be trained
     * @return a URI pointing to an RSS feed that will contain the models matching this request
     */
    public URI trainModelsFromCorpora(CorpusID[] IDs);

    /**
     * Get a valid identifier for a new Dictionary
     * @param desc descriptor for the new Dictionary
     * @return the new Dictionary's identifier
     * @throws RemoteException if a Dictionary with the given description already exists
     */
    public DictionaryID createDictionary(DictionaryDescriptor desc) throws RemoteException;

    /**
     * Get a Dictionary matching the provided descriptor
     * @param desc descriptor for the Dictionary that should match
     * @return the existing Dictionary's identifier
     * @throws RemoteException if no Dictionary matching the descriptor exists
     */
    public DictionaryID getDictionary(DictionaryDescriptor desc) throws RemoteException;

    /**
     * Fetch the DictionaryDescriptor matching the provided ID
     * @param id a legitimate Dictionary identifier
     * @return the DictionaryDescriptor provided when the given Dictionary was created
     */
    public DictionaryDescriptor getDictionaryDescriptor(DictionaryID id);

    /**
     * Create a Pronunciation record (case insensitive).
     * @param desc contains the word and its possible pronunciations
     * @param id Dictionary in which the entry should be created
     * @return the new Pronunciation's identifier
     * @throws RemoteException if the word contained in the given PronunciationDescriptor already
     * has an entry in the given dictionary
     */
    public PronunciationID createPronuncation(DictionaryID id, PronunciationDescriptor desc) throws RemoteException;

    /**
     * Check whether a Pronunciation record exists for the given word (case insensitive)
     * @param id Dictionary in which the entry should be created
     * @param word word that should be queried
     * @return true if the word has at least one pronunciation
     */
    public boolean hasPronuncation(DictionaryID id, String word);

    /**
     * Get a valid identifier for a new Corpus
     * @param dictId Dictionary with which this Corpus should be associated
     * @param desc descriptor for the new Corpus
     * @return a new Corpus identifier
     */
    public CorpusID createCorpus(DictionaryID dictId, CorpusDescriptor desc);

    /**
     * Fetch a Corpus descriptor
     * @param id identifier of an already existing Corpus
     * @return descriptor for the given Corpus identifier
     */
    public CorpusDescriptor getCorpusDescriptor(CorpusID id);

    /**
     * Deeply create a new, empty Item record associated with the given Corpus.
     * Also creates corresponding Audio and Text records, each consisting of a single
     * RegionOfAudio and RegionOfText, respectively.
     * @param corpusId Corpus to which the Item should be added
     * @return an ItemID with valid deep linkages to its newly-created component records
     */
    public ItemID createItem(CorpusID corpusId);

    /**
     * Deeply create a new Item record with exactly one Audio record containing one RegionOfAudio.
     * @param corpusId Corpus to which the Item should be added
     * @param desc descriptor for the Audio record
     * @return an ItemID with valid deep linkages to its newly-created component records
     */
    public ItemID createItemWithAudio(CorpusID corpusId, AudioDescriptor desc);

    /**
     * Deeply create a new Item record with exactly one Text record containing one RegionOfText.
     * @param corpusId Corpus to which the Item should be added
     * @param desc descriptor for the Text record
     * @return an ItemID with valid deep linkages to its newly-created component records
     * @throws java.rmi.RemoteException if the descriptor contains a word not found in the
     * provided Corpus' Dictionary
     */
    public ItemID createItemWithText(CorpusID corpusId, TextDescriptor desc) throws RemoteException;

    /**
     * Deeply create a new Item record with one Audio record and one Text record.  The Audio
     * record contains one RegionOfAudio that points to the Text record's single RegionOfText.<p/>
     * This method should be used to indicate that the Text is a transcript of the Audio.
     * @param corpusId Corpus to which the Item should be added
     * @param audioDesc descriptor for the Audio record
     * @param textDesc descriptor for the Text record
     * @return an ItemID with valid deep linkages to its newly-created component records
     * @throws java.rmi.RemoteException if the descriptor contains a word not found in the
     * provided Corpus' Dictionary
     */
    public ItemID createItemWithAudioAndText(CorpusID corpusId, AudioDescriptor audioDesc, TextDescriptor textDesc) throws RemoteException;

    /**
     * Add a RegionOfText to the given Item
     * @param id Item record to which the new RegionOfText should be added
     * @param desc descriptor for the new Text record
     * @return a Text identifier representing the newly created record
     */
    public RegionOfTextID createTextRegion(ItemID id, RegionOfTextDescriptor desc);

    /**
     * Add a RegionOfAudio to the given Item
     * @param id Item record to which the new RegionOfAudio should be added
     * @param desc descriptor for the new Audio record
     * @return an Audio identifier representing the newly created record
     */
    public RegionOfAudioID createAudioRegion(ItemID id, RegionOfAudioDescriptor desc);

    /**
     * Add a RegionOfAudio to the given Item, associating it with the given RegionOfText
     * @param id Item record to which the new RegionOfAudio should be added
     * @param textRegionID identifier of the RegionOfText with which the new RegionOfAudio should be associated
     * @param audioDesc descriptor for the new Audio record
     * @return an Audio identifier representing the newly created record
     */
    public RegionOfAudioID createAudioRegionWithText(ItemID id, RegionOfTextID textRegionID, RegionOfAudioDescriptor audioDesc);
}
