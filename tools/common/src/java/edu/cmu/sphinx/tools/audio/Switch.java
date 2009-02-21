package edu.cmu.sphinx.tools.audio;

import edu.cmu.sphinx.frontend.*;

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
 * Date: Mar 22, 2006
 * Time: 5:52:38 PM
 */
public class Switch extends BaseDataProcessor {

    protected ThrowAwayThread throwAwayThread;

    public void open() {
        throwAwayThread = new ThrowAwayThread();
        throwAwayThread.start();
    }

    public void close() {
        if (isOn()) {
            throwAwayThread.interrupt();
        }
    }

    public boolean isOn() {
        return false;
    }

    public void off() {
                         /*
        // wait until the BOS has been read
        while (state == GOING_ON) {
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
            }
        }

//        assert state == STREAMING;

        state = EOS;

        if (!throwingAwayData) {
        //            assert thread == null;

            throwingAwayData = true;
            thread = new SwitchThread();
            thread.start();
        }
        */
    }

    final int GOING_ON = 1;
    final int GOING_OFF = 2;
    final int ON = 3;
    final int OFF = 4;

    protected int state = OFF;
    protected int dataCount;

    /**
     * Returns the processed Data output.
     *
     * @return an Data object that has been processed by this DataProcessor
     * @throws edu.cmu.sphinx.frontend.DataProcessingException
     *          if a data processor error occurs
     */
    public Data getData() throws DataProcessingException {

        Data d = null;

        switch (state) {

            case GOING_ON:
                state = ON;
                d = new DataStartSignal(16000);
                break;

            case GOING_OFF:
                state = OFF;
                d = new DataEndSignal(dataCount);
                break;

            case ON:
                    d = getPredecessor().getData();
                    if (d instanceof DataEndSignal) {
                        off();
                        close();
                    }
                break;

            case OFF:
                d = null;
                break;

            default:
                throw new Error("Internal Error: Switch in bad state");
        }

        return d;
    }

    boolean throwingAwayData;

    class ThrowAwayThread extends Thread {

        public void run() {
            while (throwingAwayData) {
                try {
                    getPredecessor().getData();
                } catch (DataProcessingException e) {
                    // ignore exceptions
                }
            }
        }
    }

}
