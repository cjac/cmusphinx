package edu.cmu.sphinx.tools.dictator.models;

import edu.cmu.sphinx.linguist.dictionary.FastDictionary;
import edu.cmu.sphinx.util.Timer;

import java.io.IOException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Iterator;
import java.net.URL;

/**
 * Copyright 1999-2006 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * User: Peter Wolf
 * Date: May 13, 2006
 * Time: 4:18:56 PM
 */
public class Dictionary extends FastDictionary {
        public void allocate() throws IOException {
        if (!allocated) {
            dictionary = new HashMap();
            Timer loadTimer = Timer.getTimer("DictionaryLoad");
            fillerWords = new HashSet();

            loadTimer.start();

            System.out.println("Loading dictionary from: " + wordDictionaryFile.getPath());

            loadDictionary(getClass().getResourceAsStream(wordDictionaryFile.getPath()), false);

            loadCustomDictionaries(addendaUrlList);

            logger.info("Loading filler dictionary from: " +
                        fillerDictionaryFile);

            loadDictionary(getClass().getResourceAsStream(fillerDictionaryFile.getPath()), true);

            loadTimer.stop();
        }

    }

    protected void loadCustomDictionaries(List addenda) throws IOException {
        if (addenda != null) {
            for (Iterator i = addenda.iterator(); i.hasNext();) {
                URL addendumUrl = (URL) i.next();
                loadDictionary(getClass().getResourceAsStream(addendumUrl.getPath()), false);
            }
        }
    }
}
