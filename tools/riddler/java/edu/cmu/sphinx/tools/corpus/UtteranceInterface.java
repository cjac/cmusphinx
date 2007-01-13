package edu.cmu.sphinx.tools.corpus;

import java.util.List;

/**
 * Copyright 1999-2006 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * <p/>
 * User: Peter Wolf
 * Date: Jan 4, 2007
 * Time: 10:57:30 PM
 */
public interface UtteranceInterface {
    RegionOfAudioInterface getRegionOfAudioData();

    int getBeginTime();

    int getEndTime();

    void setRegionOfAudioData(RegionOfAudioInterface regionOfAudioData);

    void setCorpus(Corpus corpus);

    void setTranscript(String transcript);

    void addWords(List<Word> words);

    List<Word> getWords();

    void addWord(Word w);

    String getTranscript();

    CorpusInterface getCorpus();
}

