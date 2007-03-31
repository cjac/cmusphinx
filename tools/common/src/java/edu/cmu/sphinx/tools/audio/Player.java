package edu.cmu.sphinx.tools.audio;

import edu.cmu.sphinx.frontend.util.VUMeter;
import edu.cmu.sphinx.tools.corpus.RegionOfAudioData;

import javax.sound.sampled.*;
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
 * Date: Feb 23, 2006
 * Time: 4:25:19 PM
 */
public class Player {

    protected RegionOfAudioData rad;
    protected AudioFormat format;
    protected boolean stop;
    protected byte[] data;
    protected DataLine.Info info;
    protected SourceDataLine out;
    int bytesPerFrame;
    int framesPerSecond = 10;

    protected VUMeter vuMeter;

    protected PlayerThread thread=null;

    public Player( RegionOfAudioData rad ) {
        try {
            this.rad = rad;
            format = rad.getAudioDatabase().getPcm().getAudioFormat();
            data = toBytes( rad.getAudioData().getAudioData() );
            stop = false;
            vuMeter = new VUMeter();
            bytesPerFrame = (int) ((format.getChannels() * format.getSampleRate()) / framesPerSecond) * (format.getSampleSizeInBits() / 8);

            info = new DataLine.Info(SourceDataLine.class,format);
            if (!AudioSystem.isLineSupported(info)) {
                throw new Error( "Audio format is not supported" );
            }
            out = (SourceDataLine) AudioSystem.getLine(info);
        } catch (LineUnavailableException e) {
            throw new Error(e);
        } catch (IOException e) {
            throw new Error(e);
        }
    }

    public void start() {
        stop = false;
        thread = new PlayerThread();
        thread.start();
    }

    public void stop() {
        stop = true;
        try {
            thread.join();
        } catch (InterruptedException e) {
        }
    }

    public boolean isPlaying() {
        if( thread==null ) return false;
        else return thread.isAlive();
    }

    protected byte [] toBytes( short[] data ) {
        byte [] b = new byte[2];
        byte [] r = new byte[ data.length * 2 ];
        for( int i=0; i<data.length; i++ ) {
            Utils.toBytes( data[i], b, false );
            r[i*2] = b[0];
            r[(i*2)+1] = b[1];
        }
        return r;
    }

    public VUMeter getVUMeter() {
        return vuMeter;
    }

    protected class PlayerThread extends Thread {
        public void run() {
            try {

                out.open(format);
                out.start();

                for( int i=0; i<data.length; i+=bytesPerFrame ) {
                    if( stop ) break;
                    int cnt = bytesPerFrame;
                    if( data.length-i < cnt ) cnt = data.length-i;
                    out.write( data, i, cnt );
                    vuMeter.calculateVULevels( data,i,cnt );
                }

                out.drain();
                out.close();

            } catch (LineUnavailableException e) {
                throw new Error(e);
            }
        }
    }
}
