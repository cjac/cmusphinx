package edu.cmu.sphinx.tools.corpus.xml;

import edu.cmu.sphinx.tools.corpus.*;
import javolution.xml.ObjectReader;
import javolution.xml.ObjectWriter;
import javolution.xml.XmlElement;
import javolution.xml.XmlFormat;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collection;
import java.util.List;
import java.util.HashMap;

//import com.sun.corba.se.impl.orbutil.ObjectWriter;

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
 * Time: 9:07:05 PM
 */
public class CorpusXMLJavalutionFormats {

    static void write(Corpus corpus, OutputStream out) throws IOException {
        ObjectWriter<Corpus> ow = new ObjectWriter<Corpus>();
        ow.write(corpus,out);
    }

    static Corpus read(InputStream in) {
        Corpus c = new ObjectReader<Corpus>().read(in);
        return c;
    }


    static final XmlFormat<Corpus> CorpusXMLFormat = new XmlFormat<Corpus>(Corpus.class) {

        public void format(Corpus c, XmlElement xml) {
            xml.add(c.getDictionary(), "dictionary");
            xml.add(c.getUtterances(), "utterances");
            xml.add(c.getProperties(), "properties");
        }

        public Corpus parse(XmlElement xml) {
            Corpus c = xml.object();
            c.setDictionary((Dictionary) xml.get("dictionary"));
            c.setProperties((HashMap<String,String>) xml.get("properties"));
            c.setUtterances((List<Utterance>) xml.get("utterances"));

            return c;
        }
    };

    static final XmlFormat<Dictionary> DictionaryXMLFormat = new XmlFormat<Dictionary>(Dictionary.class) {

        public void format(Dictionary d, XmlElement xml) {
            xml.add("dictionaryFile", d.getDictionaryFile());
        }

        public Dictionary parse(XmlElement xml) {
            Dictionary d = xml.object();
            d.setDictionaryFile((String)xml.get("dictionaryFile"));
            return d;
        }

    };

    static final XmlFormat<Utterance> UtteranceXMLFormat = new XmlFormat<Utterance>(Utterance.class) {

        public void format(Utterance u, XmlElement xml) {
            xml.setAttribute("transcript", u.getTranscript());
            xml.add( u.getRegionOfAudioData(), "regionOfAudioData");
            xml.add(u.getWords(), "words");
        }

        public Utterance parse(XmlElement xml) {
            Utterance u = xml.object();
            u.setTranscript( xml.getAttribute("transcript", "") );
            u.setRegionOfAudioData( (RegionOfAudioData) xml.get("regionOfAudioData"));
            u.addWords( (List<Word>) xml.get("words") );
            return u;
        }

    };

    static final XmlFormat<RegionOfAudioData> RegionOfAudioDataXMLFormat = new XmlFormat<RegionOfAudioData>(RegionOfAudioData.class) {

        public void format(RegionOfAudioData r, XmlElement xml) {
            xml.setAttribute("beginTime", r.getBeginTime());
            xml.setAttribute("endTime", r.getEndTime());
            xml.setAttribute("isExcluded", r.isExcluded());
            xml.add(r.getNotes(), "notes");
            xml.add(r.getAudioDatabase(), "audioDatabase");
        }

        public RegionOfAudioData parse(XmlElement xml) {
            RegionOfAudioData r = xml.object();
            r.setBeginTime(xml.getAttribute("beginTime", -1));
            r.setEndTime(xml.getAttribute("endTime", -1));
            r.setExcluded(xml.getAttribute("isExcluded", false));
            r.setNotes((Collection<Note>) xml.get("notes"));
            r.setAudioDatabase((AudioDatabase) xml.get("audioDatabase"));
            return r;
        }

    };

    static final XmlFormat<Note> NoteXMLFormat = new XmlFormat<Note>(Note.class) {

        public void format(Note n, XmlElement xml) {
            xml.setAttribute("beginTime", n.getBeginTime());
            xml.setAttribute("endTime", n.getEndTime());
            xml.setAttribute("text", n.getText());
        }

        public Note parse(XmlElement xml) {
            Note n = xml.object();
            n.setBeginTime(xml.getAttribute("beginTime", -1));
            n.setEndTime(xml.getAttribute("endTime", -1));
            n.setText(xml.getAttribute("text", ""));
            return n;
        }
    };

    static final XmlFormat<Word> WordXMLFormat = new XmlFormat<Word>(Word.class) {

        public void format(Word w, XmlElement xml) {
            xml.setAttribute("spelling", w.getSpelling());
            xml.add( w.getRegionOfAudioData(), "regionOfAudioData");
        }

        public Word parse(XmlElement xml) {
            Word w = xml.object();
            w.setSpelling( xml.getAttribute("spelling", "") );
            w.setRegionOfAudioData( (RegionOfAudioData) xml.get("regionOfAudioData") );
            return w;
        }
    };

    static final XmlFormat<AudioDatabase> MandarinAudioDatabaseXMLFormat = new XmlFormat<AudioDatabase>(AudioDatabase.class) {

        public void format(AudioDatabase adb, XmlElement xml) {
            xml.add(adb.getPcm(),"pcm");
            xml.add(adb.getPitch(),"pitch");
            xml.add(adb.getEnergy(),"energy");
        }

        public AudioDatabase parse(XmlElement xml) {
            AudioDatabase adb = xml.object();
            adb.setPcm((PCMAudioFile) xml.get("pcm"));
            adb.setPitch((TextFileOfDoubles) xml.get("pitch"));
            adb.setEnergy((TextFileOfDoubles) xml.get("energy"));
            return adb;
        }

    };

       static final XmlFormat<PCMAudioFile> PCMAudioFileXMLFormat = new XmlFormat<PCMAudioFile>(PCMAudioFile.class) {

        public void format(PCMAudioFile adb, XmlElement xml) {
            xml.setAttribute("pcmFile", adb.getPcmFile());
            xml.setAttribute("bitsPerSample", adb.getBitsPerSample());
            xml.setAttribute("samplesPerSecond", adb.getSamplesPerSecond());
            xml.setAttribute("channelCount", adb.getChannelCount());
        }

        public PCMAudioFile parse(XmlElement xml) {
            PCMAudioFile paf = new PCMAudioFile();
            paf.setPcmFile( xml.getAttribute("pcmFile",""));
            paf.setBitsPerSample( xml.getAttribute("bitsPerSample",-1));
            paf.setSamplesPerSecond( xml.getAttribute("samplesPerSecond",-1));
            paf.setChannelCount( xml.getAttribute("channelCount",-1));
            return paf;
        }

    };

       static final XmlFormat<TextFileOfDoubles> TextFileOfDoublesXMLFormat = new XmlFormat<TextFileOfDoubles>(TextFileOfDoubles.class) {

        public void format(TextFileOfDoubles adb, XmlElement xml) {
            xml.setAttribute("textFile", adb.getTextFile());
        }

        public TextFileOfDoubles parse(XmlElement xml) {
            TextFileOfDoubles tf = new TextFileOfDoubles();
            tf.setTextFile( xml.getAttribute("textFile",""));
            return tf;
        }

    };

}
