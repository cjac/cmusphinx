package edu.cmu.sphinx.tools.corpus;

import edu.cmu.sphinx.tools.audio.AudioData;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Collection;

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
 * Date: Mar 3, 2006
 * Time: 9:48:04 PM
 */

/**
 * RegionOfAudioData is a base class of behavior common to all ojects that describe a region of audio data such as
 * a recording of a Word or an Utterance.  All RegionOfAudioData objects may be decorated with mulitple Notes that associate
 * user text with a region of time. Finally, all RegionOfAudioData objects may be marked at "excluded" which means that
 * refer to flawed data and should not be used in the testing and training.
 */
public class RegionOfAudioData {

    protected AudioDatabase audioDatabase;
    protected int beginTime;
    protected int endTime;
    protected Collection<Note> notes;
    protected boolean isExcluded;

    public boolean isExcluded() {
        return isExcluded;
    }

    public void setNotes(Collection<Note> notes) {
        this.notes = notes;
    }

    public void setExcluded(boolean excluded) {
        isExcluded = excluded;
    }

    public void addNote(Note note) {
        this.notes.add(note);
    }

    public Collection<Note> getNotes() {
        return notes;
    }

    public AudioDatabase getAudioDatabase() {
        return audioDatabase;
    }

    public void setAudioDatabase(AudioDatabase audioDatabase) {
        this.audioDatabase = audioDatabase;
    }

    public int getBeginTime() {
        return beginTime;
    }

    public void setBeginTime(int beginTime) {
        this.beginTime = beginTime;
    }

    public int getEndTime() {
        return endTime;
    }

    public void setEndTime(int endTime) {
        this.endTime = endTime;
    }

    public InputStream toInputStream() throws IOException {
        audioDatabase.getPcm().open();
        InputStream s = new ByteArrayInputStream(audioDatabase.getPcm().readPcmAsBytes(beginTime, endTime));
        audioDatabase.getPcm().close();
        return s;
    }

    public AudioData getAudioData() throws IOException {
        audioDatabase.getPcm().open();
        AudioData ad = new AudioData(audioDatabase.getPcm().readPcmAsShorts(beginTime, endTime), audioDatabase.getPcm().getSamplesPerSecond());
        audioDatabase.getPcm().close();
        return ad;
    }

    public double[] getPitchData() {
        return null;
    }

    public double[] getEnergyData() {
        return null;
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final RegionOfAudioData that = (RegionOfAudioData) o;

        if (beginTime != that.beginTime) return false;
        if (endTime != that.endTime) return false;
        if (isExcluded != that.isExcluded) return false;
        if (audioDatabase != null ? !audioDatabase.equals(that.audioDatabase) : that.audioDatabase != null)
            return false;
        if (notes != null ? !notes.equals(that.notes) : that.notes != null) return false;

        return true;
    }

    public int hashCode() {
        int result;
        result = (audioDatabase != null ? audioDatabase.hashCode() : 0);
        result = 29 * result + beginTime;
        result = 29 * result + endTime;
        result = 29 * result + (notes != null ? notes.hashCode() : 0);
        result = 29 * result + (isExcluded ? 1 : 0);
        return result;
    }

}
