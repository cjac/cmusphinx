package edu.cmu.sphinx.tools.corpus.xml;

import edu.cmu.sphinx.tools.corpus.*;

import java.io.OutputStream;
import java.io.IOException;

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
 * Time: 7:17:12 PM
 */
public class CorpusXMLWriter implements CorpusWriter {

    protected OutputStream out;

    public CorpusXMLWriter( OutputStream out ) {
        this.out = out;
    }

    public void write(Corpus corpus) throws IOException {
        CorpusXMLJavalutionFormats.write(corpus,out);
    }
}