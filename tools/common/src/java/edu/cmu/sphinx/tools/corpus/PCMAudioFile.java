package edu.cmu.sphinx.tools.corpus;

import edu.cmu.sphinx.frontend.util.DataUtil;

import javax.sound.sampled.AudioFormat;
import java.io.IOException;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;

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
 * Time: 10:37:16 PM
 */
public class PCMAudioFile {

    String pcmFile;
    int bitsPerSample;
    int samplesPerSecond;
    int channelCount;
    DataInputStream pcm;
    private int bytesPerMillisecond;

    public String getPcmFile() {
        return pcmFile;
    }

    public void setPcmFile(String pcmFile) {
        this.pcmFile = pcmFile;
    }

    public int getBitsPerSample() {
        return bitsPerSample;
    }

    public void setBitsPerSample(int bitsPerSample) {
        this.bitsPerSample = bitsPerSample;
    }

    public int getSamplesPerSecond() {
        return samplesPerSecond;
    }

    public void setSamplesPerSecond(int samplesPerSecond) {
        this.samplesPerSecond = samplesPerSecond;
    }

    public int getChannelCount() {
        return channelCount;
    }

    public void setChannelCount(int channelCount) {
        this.channelCount = channelCount;
    }

    public PCMAudioFile() {
    }

    public void open() throws IOException {
        assert pcm == null;
        bytesPerMillisecond = (bitsPerSample * samplesPerSecond * channelCount)/8000;
        pcm = new DataInputStream( new FileInputStream( pcmFile) );
    }

    public void close() throws IOException {
        assert pcm != null;
        pcm.close();
        pcm = null;
    }

    public byte[] readPcmAsBytes(int beginTime, int endTime) throws IOException {
        int l = (endTime-beginTime) * bytesPerMillisecond;;
        byte[] buf = new byte[ l ];
        pcm.read(buf,(bytesPerMillisecond*beginTime),l);
        return buf;
    }

    public short[] readPcmAsShorts(int beginTime, int endTime) throws IOException {
        byte [] buf = readPcmAsBytes(beginTime,endTime);
        return DataUtil.byteToShortArray(buf,0,buf.length);
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final PCMAudioFile that = (PCMAudioFile) o;

        if (bitsPerSample != that.bitsPerSample) return false;
        if (channelCount != that.channelCount) return false;
        if (samplesPerSecond != that.samplesPerSecond) return false;
        if (pcmFile != null ? !pcmFile.equals(that.pcmFile) : that.pcmFile != null) return false;

        return true;
    }

    public int hashCode() {
        int result;
        result = (pcmFile != null ? pcmFile.hashCode() : 0);
        result = 29 * result + bitsPerSample;
        result = 29 * result + samplesPerSecond;
        result = 29 * result + channelCount;
        return result;
    }

    public AudioFormat getAudioFormat() {
        return new AudioFormat(samplesPerSecond,bitsPerSample,channelCount,true,true);
    }
}
