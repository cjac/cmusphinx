package edu.cmu.sphinx.tools.corpus;

import java.util.List;
import java.util.ArrayList;

/**
 * An Utterance represents a single connected sequence of spoken words.  It contains the transcript of this speech,
 * the associated audio data, and the individual Word objects that describe the sub-segments containing the data for
 * the individual words.
 */
public class Utterance  {

    protected Corpus corpus;
    protected List<Word> words = new ArrayList<Word>();
    protected String transcript;
    protected RegionOfAudioData regionOfAudioData;

    public RegionOfAudioData getRegionOfAudioData() {
        return regionOfAudioData;
    }

    public int getBeginTime() {
        return regionOfAudioData.getBeginTime();
    }

    public int getEndTime() {
        return regionOfAudioData.getBeginTime();
    }

    public void setRegionOfAudioData(RegionOfAudioData regionOfAudioData) {
        this.regionOfAudioData = regionOfAudioData;
    }

    public void setCorpus(Corpus corpus) {
        this.corpus = corpus;
        for (Word w : words) {
            w.setUtterance(this);
            corpus.addWord(w);
        }
    }

    public void setTranscript(String transcript) {
        this.transcript = transcript;
    }

    public Utterance() {
    }

    public void addWords(List<Word> words) {
        for (Word w : words) {
            addWord(w);
        }
    }

    public List<Word> getWords() {
        return words;
    }

    public void addWord(Word w) {
        words.add(w);
    }

    public String getTranscript() {
        return transcript;
    }

    public Corpus getCorpus() {
        return corpus;
    }

    public String toString() {
        return getRegionOfAudioData() + " " + getTranscript();
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final Utterance utterance = (Utterance) o;

        if (regionOfAudioData != null ? !regionOfAudioData.equals(utterance.regionOfAudioData) : utterance.regionOfAudioData != null)
            return false;
        if (transcript != null ? !transcript.equals(utterance.transcript) : utterance.transcript != null) return false;
        if (words != null ? !words.equals(utterance.words) : utterance.words != null) return false;

        return true;
    }

    public int hashCode() {
        int result;
        result = (words != null ? words.hashCode() : 0);
        result = 29 * result + (transcript != null ? transcript.hashCode() : 0);
        result = 29 * result + (regionOfAudioData != null ? regionOfAudioData.hashCode() : 0);
        return result;
    }
}
