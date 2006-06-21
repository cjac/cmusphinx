package edu.cmu.sphinx.tools.datacollection.client;

import edu.cmu.sphinx.tools.corpus.CorpusReader;
import edu.cmu.sphinx.tools.corpus.Corpus;
import edu.cmu.sphinx.tools.corpus.Utterance;

import java.io.StringReader;
import java.io.BufferedReader;
import java.io.UnsupportedEncodingException;
import java.io.IOException;
import java.net.URLDecoder;

/**
 * Created by IntelliJ IDEA.
 * User: bertrand
 * Date: Apr 22, 2006
 * Time: 1:14:13 PM
 * Reads a corpus serialized by CorpusTranscriptsOnlyWriter, the corpus has only transcripts and properties
 */
public class CorpusTranscriptsOnlyReader implements CorpusReader {
    String _serializedCorpus;

    public CorpusTranscriptsOnlyReader(String s) {
        try {
       _serializedCorpus = URLDecoder.decode(s,"utf8");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }

    public Corpus read() {
        StringReader sr = new StringReader(_serializedCorpus);
        BufferedReader bf = new BufferedReader(sr);
        Corpus c = new Corpus();
        String line;

        try {
            while ((line = bf.readLine()) != null) {
                if (line.startsWith("*property")) {
                    final int anchor = line.indexOf("=");
                    final String key = line.substring(10, anchor);
                    final String value = line.substring(anchor+1, line.length());
                    c.setProperty(key, value);
                } else {
                    Utterance utt = new Utterance();
                    utt.setTranscript(line);
                    c.addUtterance(utt);
                }
            }
            sr.close();
            bf.close();

            return c;
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }
}
