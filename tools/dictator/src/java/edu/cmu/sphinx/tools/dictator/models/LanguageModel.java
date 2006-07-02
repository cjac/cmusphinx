package edu.cmu.sphinx.tools.dictator.models;

import edu.cmu.sphinx.linguist.language.ngram.SimpleNGramModel;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
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
 * Date: May 14, 2006
 * Time: 10:00:47 AM
 */
public class LanguageModel extends SimpleNGramModel {
         /*
    protected BinaryLoader createLoader() throws IOException {
        return new JarBinaryLoader(format, location, applyLanguageWeightAndWip,
                logMath, languageWeight, wip, unigramWeight);
    }

    class JarBinaryLoader extends BinaryLoader {

        public JarBinaryLoader(String format, String location, boolean applyLanguageWeightAndWip, LogMath logMath, float languageWeight, double wip, float unigramWeight) throws IOException {
            super(format, location, applyLanguageWeightAndWip, logMath, languageWeight, wip, unigramWeight);
        }

        protected DataInputStream openDataFile(String location) throws FileNotFoundException {
            InputStream s = super.getClass().getResourceAsStream(location);
            System.out.println(location + " " + s);
            return new DataInputStream(new BufferedInputStream(s));
        }
    }
*/
         protected void open(URL location) throws FileNotFoundException,
            IOException {
            lineNumber = 0;
            fileName = location.toString();
            reader = new BufferedReader
	        (new InputStreamReader(super.getClass().getResourceAsStream(location.getPath())));
    }

}
