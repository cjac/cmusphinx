package edu.cmu.sphinx.tools.corpus;

import edu.cmu.sphinx.tools.audio.AudioDataInterface;

import java.util.Collection;

/**
 * Copyright 1999-2006 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * <p/>
 * User: Peter Wolf
 * Date: Jan 4, 2007
 * Time: 10:59:48 PM
 */
public interface RegionOfAudioInterface {
    void setNotes(Collection<Note> notes);

    void addNote(Note note);

    Collection<Note> getNotes();

    AudioDatabaseInterface getAudioDatabase();

    void setAudioDatabase(AudioDatabaseInterface audioDatabase);

    int getBeginTime();

    void setBeginTime(int beginTime);

    int getEndTime();

    void setEndTime(int endTime);

    AudioDataInterface getAudioData();
}

