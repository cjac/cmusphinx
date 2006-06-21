package edu.cmu.sphinx.tools.datacollection.client;

import edu.cmu.sphinx.tools.corpus.Corpus;
import edu.cmu.sphinx.tools.corpus.CorpusWriter;
import edu.cmu.sphinx.tools.corpus.Utterance;
import edu.cmu.sphinx.tools.corpus.xml.CorpusXMLWriter;

import java.io.*;
import java.util.Iterator;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/**
 * Created by IntelliJ IDEA.
 * User: bertrand
 * Date: Apr 23, 2006
 * Time: 2:09:55 PM
 * This serializer is meant to be used for completed corpora. It gathers all the dependent utterances
 * zips them up puts in on the stream, and them serializes the actual corpus in XML,
 * zips it and puts in on the stream
 */
public class CorpusXMLZipWriter implements CorpusWriter {
    ZipOutputStream _zip;

    CorpusXMLZipWriter(OutputStream os) {
        _zip = new ZipOutputStream(os);
        _zip.setLevel(9);
        _zip.setMethod(ZipOutputStream.DEFLATED);
    }

    public void write(Corpus corpus) {
        Iterator<Utterance> it = corpus.getUtterances().iterator();
        while (it.hasNext()) {
            final Utterance utt = it.next();
            final String filename = (utt.getRegionOfAudioData()).getAudioDatabase().getPcm().getPcmFile();
            try {
                FileInputStream fis = new FileInputStream(filename);
                addFile(filename, fis);
                fis.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        CorpusXMLWriter cxw = new CorpusXMLWriter(baos);
        try {
            cxw.write(corpus);
            baos.flush();
            ByteArrayInputStream bais = new ByteArrayInputStream(baos.toByteArray());
            addFile("corpus-" + corpus.getProperty("userID") + "-" + corpus.getProperty("ID") + ".xml", bais);
            bais.close();
            _zip.finish();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * streams out a file in zip format onto the stream
     */

    protected void addFile(String filename, InputStream stream)
            throws IOException {
        ZipEntry entry = new ZipEntry(filename);
        byte[] chunk = new byte[1024];
        int len;

        _zip.putNextEntry(entry);
        while ((len = stream.read(chunk)) > 0) {
            _zip.write(chunk, 0, len);

        }
        _zip.closeEntry();
        _zip.flush();

    }
}
