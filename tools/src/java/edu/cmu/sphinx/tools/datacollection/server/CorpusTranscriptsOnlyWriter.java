package edu.cmu.sphinx.tools.datacollection.server;

import edu.cmu.sphinx.tools.corpus.Corpus;
import edu.cmu.sphinx.tools.corpus.CorpusWriter;
import edu.cmu.sphinx.tools.corpus.Utterance;

import java.io.UnsupportedEncodingException;
import java.io.StringWriter;
import java.io.BufferedWriter;
import java.io.IOException;
import java.net.URLEncoder;
import java.util.Iterator;
import java.util.HashMap;

/**
 * Created by IntelliJ IDEA.
 * User: bertrand
 * Date: Apr 22, 2006
 * Time: 12:46:56 PM
 * Copyright Apr 22, 2006, All rights reserved.
 * Writes a serialized corpus smeant to be read by CorpusTranscriptsOnlyReader, the corpus has only transcripts
 * and properties
 */
public class CorpusTranscriptsOnlyWriter implements CorpusWriter {
    StringBuffer _sbuf;

    public CorpusTranscriptsOnlyWriter(StringBuffer sbuf) {
        _sbuf = sbuf;
    }

    public void write(Corpus corpus) {
        Iterator<Utterance> utts = corpus.getUtterances().iterator();
        StringWriter sw = new StringWriter();
        BufferedWriter bw = new BufferedWriter(sw);
        String s = "";
        HashMap<String,String> h = corpus.getProperties();
        Iterator<String> properties = h.keySet().iterator();
        try {

            while (properties.hasNext()) {
                final String key = properties.next();
                final String value = h.get(key);
                bw.write("*property " + key + "=" + value);
                bw.newLine();
            }

            while (utts.hasNext()) {
                bw.write((utts.next()).getTranscript());
                if (utts.hasNext()) bw.newLine();
            }
            
            bw.flush();
            sw.flush();
            s = sw.getBuffer().toString();

            sw.close();
            bw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {

            _sbuf.replace(0, _sbuf.capacity() - 1, URLEncoder.encode(s, "utf8"));

        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }
}
