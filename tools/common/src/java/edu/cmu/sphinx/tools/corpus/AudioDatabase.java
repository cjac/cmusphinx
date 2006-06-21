package edu.cmu.sphinx.tools.corpus;

import javax.sound.sampled.AudioFormat;

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
 * Date: Mar 11, 2006
 * Time: 1:57:03 PM
 */
public class AudioDatabase {

    private PCMAudioFile pcm;
    private TextFileOfDoubles pitch;
    private TextFileOfDoubles energy;

    public PCMAudioFile getPcm() {
        return pcm;
    }

    public void setPcm(PCMAudioFile pcm) {
        this.pcm = pcm;
    }

    public TextFileOfDoubles getPitch() {
        return pitch;
    }

    public void setPitch(TextFileOfDoubles pitch) {
        this.pitch = pitch;
    }

    public TextFileOfDoubles getEnergy() {
        return energy;
    }

    public void setEnergy(TextFileOfDoubles energy) {
        this.energy = energy;
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final AudioDatabase that = (AudioDatabase) o;

        if (energy != null ? !energy.equals(that.energy) : that.energy != null) return false;
        if (pcm != null ? !pcm.equals(that.pcm) : that.pcm != null) return false;
        if (pitch != null ? !pitch.equals(that.pitch) : that.pitch != null) return false;

        return true;
    }

    public int hashCode() {
        int result;
        result = (pcm != null ? pcm.hashCode() : 0);
        result = 29 * result + (pitch != null ? pitch.hashCode() : 0);
        result = 29 * result + (energy != null ? energy.hashCode() : 0);
        return result;
    }
}
