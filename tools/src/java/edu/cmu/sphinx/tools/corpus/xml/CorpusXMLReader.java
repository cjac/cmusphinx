package edu.cmu.sphinx.tools.corpus.xml;

import edu.cmu.sphinx.tools.corpus.CorpusReader;
import edu.cmu.sphinx.tools.corpus.Corpus;

import java.io.InputStream;

import javolution.xml.ObjectReader;

/**
 * Copyright 1999-2006 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * Portions Copyright 2002 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * User: Peter Wolf
 * Date: Apr 2, 2006
 * Time: 9:04:47 PM
 */
public class CorpusXMLReader implements CorpusReader {

    InputStream in;

    public CorpusXMLReader( InputStream in ) {
        this.in = in;
    }

    public Corpus read() {
        return new ObjectReader<Corpus>().read(in);
    }
}
