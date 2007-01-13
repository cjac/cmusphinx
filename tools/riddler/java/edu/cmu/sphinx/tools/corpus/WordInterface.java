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
 * Time: 10:58:40 PM
 */
public interface WordInterface {
    RegionOfAudioData getRegionOfAudioData();

    void setRegionOfAudioData(RegionOfAudioData regionOfAudioData);

    UtteranceInterface getUtterance();

    int getBeginTime();

    int getEndTime();

    String getSpelling();

    void setSpelling(String spelling);

    void setUtterance(UtteranceInterface utterance);

    List<String> getCharacters();
}

