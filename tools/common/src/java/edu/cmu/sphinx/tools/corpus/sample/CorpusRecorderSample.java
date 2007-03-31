package edu.cmu.sphinx.tools.corpus.sample;

import edu.cmu.sphinx.frontend.util.VUMeter;
import edu.cmu.sphinx.tools.audio.Player;
import edu.cmu.sphinx.tools.audio.Recorder;
import edu.cmu.sphinx.tools.corpus.Corpus;
import edu.cmu.sphinx.tools.corpus.RegionOfAudioData;
import edu.cmu.sphinx.tools.corpus.Utterance;
import edu.cmu.sphinx.tools.corpus.xml.CorpusXMLReader;
import edu.cmu.sphinx.tools.corpus.xml.CorpusXMLWriter;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;

import java.io.*;
import java.net.URL;

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
 * Date: Mar 9, 2006
 * Time: 10:05:04 PM
 */

/**
 * This a sample program that demonstrates the Corpus API and Recorder API.  It implements
 * a trivial corpus collection.  It reads transcripts from a text file, with each transcript separated by a LF.
 * It then creates a Corpus with an Utterance for each transcript.   It then prompts the user with the
 * transcript and records 5 seconds of audio.  Finally, it saves the Corpus as an XML file.
 */
public class CorpusRecorderSample {


    public static void main(String[] args) {
        try {

            if (args.length != 3) {
                throw new Error("USAGE: CorpusRecorderSample configXMLFile inputTranscriptFile outputCorpuXMLFile");
            }

            Recorder recorder = openAudioRecorder(args[0]);

            Corpus corpus = makeCorpusWithOnlyTrascripts(args[1]);


            for (Utterance utterance : corpus.getUtterances()) {
                recordAudioData(recorder,utterance);
            }

            FileOutputStream out = new FileOutputStream(args[2]);
            CorpusXMLWriter xmlOut = new CorpusXMLWriter(out);
            xmlOut.write(corpus);
            out.close();

            closeAudioRecorder(recorder);

            FileInputStream in = new FileInputStream( args[2] );
            CorpusXMLReader xmlIn = new CorpusXMLReader(in);
            Corpus corpus2 = xmlIn.read();

            if( !corpus.equals(corpus2) ) {
                throw new Error( corpus + " != " + corpus2 );
            }

            for( Utterance u : corpus.getUtterances() ) {
                Player player = new Player(u.getRegionOfAudioData());
                player.start();

                int b = u.getRegionOfAudioData().getBeginTime();
                int e = u.getRegionOfAudioData().getEndTime();
                for( int i=0; i<(e-b)/200; i++) {
                    printVULevel(player.getVUMeter());
                    sleep(100);
                }

                player.stop();
            }

        } catch (FileNotFoundException e) {
            throw new Error(e);
        } catch (IOException e) {
            throw new Error(e);
        } catch (PropertyException e) {
            throw new Error(e);
        }
    }

    private static void printVULevel(VUMeter vuMeter) {
        System.out.print("VU RMS=" + vuMeter.getRmsDB());
        System.out.print(" VU Peak=" + vuMeter.getPeakDB());
        System.out.println(" VU IsClipping="+vuMeter.getIsClipping());
    }

    private static Recorder openAudioRecorder(String configFile) throws IOException, PropertyException {
        // the recorder will store the data in the specified file
        //
        // the config file contains intialization info for the microphone
        //
        // also open() allocates the audio device and starts the VUMeter running,
        // but does not record audio.  This allows the application to monitor the
        // sound level before recording

        URL url = new File(configFile).toURI().toURL();
        ConfigurationManager cm = new ConfigurationManager(url);

        Recorder recorder = new Recorder();
        recorder.open(cm);
        return recorder;
    }

    private static void closeAudioRecorder(Recorder recorder) {
        recorder.close();
        recorder = null;
    }

    /**
     * Given a text file of transcripts, create a Corpus with Utterances that only contain transcripts.
     *
     * @param transcriptFile
     * @return the corpus
     * @throws IOException
     */
    private static Corpus makeCorpusWithOnlyTrascripts(String transcriptFile) throws IOException {

        // transcripts are stored as LF separated text
        LineNumberReader in = new LineNumberReader(new FileReader(transcriptFile));

        // create a new corpus
        Corpus corpus = new Corpus();

        // for each transcript in the file
        //   create a new utterance associated with that transcript
        //   add it to the corpus
        //   record audio associated with that utterance
        for (String transcript = in.readLine(); transcript != null; transcript = in.readLine()) {
            Utterance utterance = new Utterance();
            utterance.setTranscript(transcript);
            corpus.addUtterance(utterance);
        }

        return corpus;

    }

    /**
     * Prompt the user with the transcript, and record 5 seconds of audio.
     * Also demonstrate the VUMeter API
     *
     * @param utterance
     */
    private static void recordAudioData(Recorder recorder, Utterance utterance) throws IOException {

        // demonstrate the VU meter and sleep for 5 seconds
        sleepAndMonitorVUMeter(recorder.getVUMeter(), 100);

        // prompt the user
        System.out.println("Please say: " + utterance.getTranscript());

        // start recording.  This starts streaming the audio to the specified file
        recorder.start("utt" + utterance.hashCode());

        // demonstrate the VU meter and sleep for 5 seconds
        sleepAndMonitorVUMeter(recorder.getVUMeter(), 5000);

        // stop recording
        RegionOfAudioData data = recorder.stop();

        // associate the recorded audio with the utterance
        utterance.setRegionOfAudioData(data);
    }

    /**
     * Demonstrate the VUMeter.  Rms is the average level, and peak is the peak over
     * the last frame of audio.  Peak goes up instantly and then decays.
     * @param vuMeter
     * @param ms
     */
    private static void sleepAndMonitorVUMeter(VUMeter vuMeter, int ms) {
        for (int i = 0; i < 10; i++) {
            printVULevel(vuMeter);
            sleep(ms / 10);
        }
    }

    private static void sleep(int ms) {
        try {
            Thread.sleep(ms);
        } catch (InterruptedException e) {
        }
    }
}
