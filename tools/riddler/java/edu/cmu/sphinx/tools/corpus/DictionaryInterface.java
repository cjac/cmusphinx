package edu.cmu.sphinx.tools.corpus;

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
 * Time: 11:02:11 PM
 */
public interface DictionaryInterface {
    String[] getPronunciation(WordInterface w);

    CorpusInterface getCorpus();

    void setCorpus(CorpusInterface corpus);
}

