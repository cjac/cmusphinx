package edu.cmu.sphinx.tools.audio;

import edu.cmu.sphinx.frontend.Data;
import edu.cmu.sphinx.frontend.DataProcessingException;
import edu.cmu.sphinx.frontend.DoubleData;
import edu.cmu.sphinx.frontend.util.Microphone;
import edu.cmu.sphinx.frontend.util.VUMeter;
import edu.cmu.sphinx.tools.corpus.AudioDatabase;
import edu.cmu.sphinx.tools.corpus.PCMAudioFile;
import edu.cmu.sphinx.tools.corpus.RegionOfAudioData;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;

import java.io.DataOutputStream;
import java.io.FileOutputStream;
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
 * Date: Mar 28, 2006
 * Time: 5:53:27 PM
 */
public class Recorder {

    private DataOutputStream pcmOut;
    private Microphone microphone;
    private Thread thread;
    private VUMeter vuMeter;
    private boolean isOpen;
    private boolean isRecording;
    private RegionOfAudioData regionOfAudioData;

    public void open(ConfigurationManager cm) {
        isRecording = false;
        isOpen = true;
        vuMeter = new VUMeter();
        try {
            microphone = (Microphone) cm.lookup("microphone");
            microphone.initialize();
        } catch (PropertyException e) {
            throw new Error(e);
        }
        microphone.startRecording();
        thread = new AudioRecorderThread();
        thread.start();
    }

    public void close() {

        isOpen = false;
        microphone.stopRecording();
        microphone.clear();

        boolean isDone = false;
        while (!isDone) {
            try {
                thread.join();
                isDone = true;
            } catch (InterruptedException e) {
            }
        }
    }

    public void start(String baseFileName) throws IOException {

        regionOfAudioData = new RegionOfAudioData();

        AudioDatabase audioDatabase = new AudioDatabase();

        PCMAudioFile pcm = new PCMAudioFile();
        pcm.setPcmFile(baseFileName + ".pcm");
        pcm.setBitsPerSample(16);
        pcm.setSamplesPerSecond(16000);
        pcm.setChannelCount(1);

        audioDatabase.setPcm(pcm);
        audioDatabase.setPitch(null);
        audioDatabase.setEnergy(null);

        regionOfAudioData.setAudioDatabase(audioDatabase);

        pcmOut = new DataOutputStream(new FileOutputStream(pcm.getPcmFile()));

        isRecording = true;
    }

    final int bytesPerMillisecond = (16 * 16000) / 8000;

    public synchronized RegionOfAudioData stop() throws IOException {
        isRecording = false;
        // wait for confirmation
        pcmOut.flush();

        regionOfAudioData.setBeginTime(0);
        regionOfAudioData.setEndTime(pcmOut.size() / bytesPerMillisecond);

        pcmOut.close();
        pcmOut = null;

        return regionOfAudioData;
    }

    public VUMeter getVUMeter() {
        return vuMeter;
    }

    private synchronized void writeData(Data data) throws IOException {
        if (isRecording) {
            if (data instanceof DoubleData) {
                for (double sample : ((DoubleData) data).getValues()) {
                    short s = (short) sample;
                    int l = s & 0xff;
                    int h = (s & 0xff00) >> 8;
                    pcmOut.writeByte(l);
                    pcmOut.writeByte(h);
                }
            }
        }
    }

    class AudioRecorderThread extends Thread {
        public void run() {
            try {
                // read from the Mic
                // feed the VU meter
                // if recording, store in pcm file
                while (isOpen) {
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                    }
                    if (microphone.hasMoreData()) {
                        Data data = microphone.getData();
                        vuMeter.calculateVULevels(data);
                        writeData(data);
                    }
                }
            }
            catch (DataProcessingException e) {
                throw new Error(e);
            } catch (IOException e) {
                throw new Error(e);
            }
        }


    }
}
