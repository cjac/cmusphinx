package edu.cmu.sphinx.tools.corpus;

import edu.cmu.sphinx.tools.corpus.xml.CorpusXMLReader;
import edu.cmu.sphinx.tools.corpus.xml.CorpusXMLWriter;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.*;


/**
 * The Copus object encapsulates a collection of recordings associated with transcripts and dictionaries of pronunciations.
 * It contains all information needed to train and test a speech recognizers.
 * <br><br>
 * A Corpus is a tree data structure that contains meta-data, and references binary audio data stored externally.  A Corpus contains a collection of Utterances.  Each Utterance contains a collection
 * of Words.  Utterances and Words include begin and end times and a reference to an AudioData object that contains the audio data.
 * <br><br>
 * The Copus is initially created empty and then decorated with information and data as it becomes available.  As a typical
 * example, a Corpus might be created initially with Utterances containing only transcripts.  This Corpus would be then sent to a
 * CorpusRecorder tool that would create the AudioData objects and decorate the Utterances with begin and end times.  This Corpus
 * would then be sent to a ForceAlignRecognizerTool which would decorate the Word objects with begin and end times.  Finally,
 * the Corpus would be sent to a CorpusBrowser tool that would allow a human operator to edit the begin and end times and
 * remove bad data from the Corpus.
 * <br><br>
 * While in use, a Corpus is an in-memory data structure which references out-of-memory persistant binary data.  A Corpus
 * can be serialized as XML.  This XML is used to pass Corpra between users and tools.  The XML could be perhaps sent over a socket or stored as a file.  In the future we might consider
 * other forms of serialization such as serialized Java objects, or RDF.
 */
public class Corpus {

    protected Dictionary dictionary;
    protected List<Utterance> utterances = new ArrayList<Utterance>();
    protected Map<String, List<Word>> character2Words = new HashMap<String, List<Word>>();
    protected Map<String, List<Word>> spelling2Words = new HashMap<String, List<Word>>();
    protected Map<String, List<Word>> phonemeSequence2Words = new HashMap<String, List<Word>>();
    protected HashMap<String,String> properties = new HashMap<String, String>();



    static String getCorpusAsString(Corpus corpus) {
        ByteArrayOutputStream ostream = new ByteArrayOutputStream();
        CorpusXMLWriter cWriter = new CorpusXMLWriter(ostream);
        try {
            cWriter.write(corpus);

            return ostream.toString();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return null;
    }

    public static void main(String args[]) {
        Corpus corpus = null; //getFakeCorpus();
        String corpusString = getCorpusAsString(corpus);

        byte[] buf = corpusString.getBytes();

        ByteArrayInputStream istream = new ByteArrayInputStream(buf);
        System.out.println(new String(buf));
        CorpusXMLReader cReader = new CorpusXMLReader(istream);
        Corpus newcorpus = cReader.read();

        System.out.println(getCorpusAsString(newcorpus));

    }

    /* Testing Roundtrip End*/


    void init() {
        dictionary.init(this);
        for (Utterance u : utterances) {
            u.setCorpus(this);
        }
    }

    public Corpus() {
    }

    public HashMap<String, String> getProperties() {
        return properties;
    }

     public void setProperties(HashMap<String, String> p) {
       properties = p;
    }


    public String setProperty(String name, String value) {
        return properties.put(name, value);
    }

    public String getProperty(String name) {
        return properties.get(name);
    }

    public Dictionary getDictionary() {
        return dictionary;
    }

    public void setDictionary(Dictionary dictionary) {
        this.dictionary = dictionary;
    }

    public List<Utterance> getUtterances() {
        return utterances;
    }

    public void setUtterances(List<Utterance> utterances) {
        this.utterances = utterances;
    }

    public Set<String> getSpellings() {
        return spelling2Words.keySet();
    }

    public List<Word> getWords(String spelling) {
        return spelling2Words.get(spelling);
    }

    public void addUtterance(Utterance utterance) {
        utterances.add(utterance);
        for (Word w : utterance.getWords()) {
            addWord(w);
        }
    }

    void addWord(Word w) {

        addSpelling(w);

        addCharacters(w);

        addPhonemeSequences(w);
    }

    private void addPhonemeSequences(Word w) {
        String[] pron = dictionary.getPronunciation(w);
        if (pron != null) {
            for (int i = 0; i < pron.length; i++) {
                addPhonemeSequence(pron[i], w);
                if (i > 0) {
                    addPhonemeSequence(pron[i - 1] + " " + pron[i], w);
                }
                if (i < pron.length - 1) {
                    addPhonemeSequence(pron[i] + " " + pron[i + 1], w);
                }
                if (i > 0 && i < pron.length - 1) {
                    addPhonemeSequence(pron[i - 1] + " " + pron[i] + " " + pron[i + 1], w);
                }
            }
        }
    }

    private void addCharacters(Word w) {
        for (String character : w.getCharacters()) {
            addCharacter(character, w);
        }
    }

    private void addSpelling(Word w) {
        if (!spelling2Words.containsKey(w.spelling)) {
            spelling2Words.put(w.spelling, new ArrayList<Word>());
        }
        List<Word> l = spelling2Words.get(w.spelling);
        l.add(w);
    }

    private void addCharacter(String character, Word w) {
        if (!character2Words.containsKey(character)) {
            character2Words.put(character, new ArrayList<Word>());
        }
        List<Word> l = character2Words.get(character);
        l.add(w);
    }

    private void addPhonemeSequence(String ps, Word w) {
        if (!phonemeSequence2Words.containsKey(ps)) {
            phonemeSequence2Words.put(ps, new ArrayList<Word>());
        }
        List<Word> l = phonemeSequence2Words.get(ps);
        l.add(w);
    }

    public Set<String> getCharacters() {
        return character2Words.keySet();
    }

    public Set<String> getPhonemeSequences() {
        return phonemeSequence2Words.keySet();
    }

    public List<Word> character2Words(String character) {
        return character2Words.get(character);
    }

    public List<Word> phonemeSequence2Words(String phoneme) {
        return phonemeSequence2Words.get(phoneme);
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final Corpus corpus = (Corpus) o;

        if (dictionary != null ? !dictionary.equals(corpus.dictionary) : corpus.dictionary != null) return false;
        if (utterances != null ? !utterances.equals(corpus.utterances) : corpus.utterances != null) return false;

        return true;
    }

    public int hashCode() {
        int result;
        result = (dictionary != null ? dictionary.hashCode() : 0);
        result = 29 * result + (utterances != null ? utterances.hashCode() : 0);
        return result;
    }
}

