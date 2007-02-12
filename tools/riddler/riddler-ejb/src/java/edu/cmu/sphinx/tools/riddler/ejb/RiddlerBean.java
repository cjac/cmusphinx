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

import edu.cmu.sphinx.tools.riddler.types.CorpusDescriptor;
import edu.cmu.sphinx.tools.riddler.types.CorpusID;
import edu.cmu.sphinx.tools.riddler.types.DictionaryDescriptor;
import edu.cmu.sphinx.tools.riddler.types.DictionaryID;
import edu.cmu.sphinx.tools.riddler.types.ItemID;
import edu.cmu.sphinx.tools.riddler.types.PronunciationDescriptor;
import edu.cmu.sphinx.tools.riddler.types.PronunciationID;
import edu.cmu.sphinx.tools.riddler.types.RegionOfAudioDescriptor;
import edu.cmu.sphinx.tools.riddler.types.RegionOfAudioID;
import edu.cmu.sphinx.tools.riddler.types.RegionOfTextDescriptor;
import edu.cmu.sphinx.tools.riddler.types.RegionOfTextID;
import edu.cmu.sphinx.tools.riddler.types.TextDescriptor;
import edu.cmu.sphinx.tools.riddler.types.audio.ByteAudioDescriptor;
import edu.cmu.sphinx.tools.riddler.types.audio.FloatAudioDescriptor;
import edu.cmu.sphinx.tools.riddler.types.audio.IntAudioDescriptor;
import edu.cmu.sphinx.tools.riddler.types.audio.LongAudioDescriptor;
import edu.cmu.sphinx.tools.riddler.types.audio.ShortAudioDescriptor;
import java.net.URI;
import javax.ejb.Stateless;
import javax.jws.WebMethod;
import javax.jws.WebService;

/**
 *
 */
@Stateless
@WebService
public class RiddlerBean implements RiddlerRemote {
    
    /** Creates a new instance of RiddlerBean */
    public RiddlerBean() {
    }
    
    @WebMethod
    public URI trainModelsFromCorpus(CorpusID id) {
        return null;
    }
    
    @WebMethod
    public URI trainModelsFromCorpora(CorpusID[] IDs) {
        return null;
    }
    
    @WebMethod
    public DictionaryID createDictionary(DictionaryDescriptor desc) {
        return null;
    }
    
    @WebMethod
    public DictionaryID getDictionary(DictionaryDescriptor desc) {
        return null;
    }
    
    @WebMethod
    public DictionaryDescriptor getDictionaryDescriptor(DictionaryID id) {
        return null;
    }
    
    @WebMethod
    public PronunciationID createPronuncation(DictionaryID id, PronunciationDescriptor desc) {
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
    public ItemID createItemWithByteAudio(CorpusID corpusId, ByteAudioDescriptor desc) {
        return null; 
    }
    
    @WebMethod
    public ItemID createItemWithShortAudio(CorpusID corpusId, ShortAudioDescriptor desc) {
        return null; 
    }
    
    @WebMethod
    public ItemID createItemWithIntAudio(CorpusID corpusId, IntAudioDescriptor desc) {
        return null; 
    }
    
    @WebMethod
    public ItemID createItemWithLongAudio(CorpusID corpusId, LongAudioDescriptor desc) {
        return null; 
    }
    
    @WebMethod
    public ItemID createItemWithFloatAudio(CorpusID corpusId, FloatAudioDescriptor desc) {
        return null; 
    }
    
    @WebMethod
    public ItemID createItemWithText(CorpusID corpusId, TextDescriptor desc) {
        return null; 
    }
    
    @WebMethod
    public ItemID createItemWithShortAudioAndText(CorpusID corpusId, ShortAudioDescriptor audioDesc, TextDescriptor textDesc) {
        return null; 
    }
    
    @WebMethod
    public ItemID createItemWithByteAudioAndText(CorpusID corpusId, ByteAudioDescriptor audioDesc, TextDescriptor textDesc) {
        return null; 
    }
    
    @WebMethod
    public ItemID createItemWithIntAudioAndText(CorpusID corpusId, IntAudioDescriptor audioDesc, TextDescriptor textDesc) {
        return null; 
    }
    
    @WebMethod
    public ItemID createItemWithLongAudioAndText(CorpusID corpusId, LongAudioDescriptor audioDesc, TextDescriptor textDesc) {
        return null; 
    }
    
    @WebMethod
    public ItemID createItemWithFloatAudioAndText(CorpusID corpusId, FloatAudioDescriptor audioDesc, TextDescriptor textDesc) {
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
