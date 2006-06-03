package edu.cmu.sphinx.tools.corpus;

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
 * Time: 9:40:28 PM
 */

/**
 * A Note is a piece of text associated with a span of time.  It might be used to describe
 * a region of data in a Word or and Utterance.
 */
public class Note {

    String text;
    int beginTime;
    int endTime;

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

    public String getText() {
        return text;
    }

    public void setText(String text) {
        this.text = text;
    }

    public Note(int beginTime, int endTime, String text) {
        this.beginTime = beginTime;
        this.endTime = endTime;
        this.text = text;
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final Note note = (Note) o;

        if (beginTime != note.beginTime) return false;
        if (endTime != note.endTime) return false;
        if (text != null ? !text.equals(note.text) : note.text != null) return false;

        return true;
    }

    public int hashCode() {
        int result;
        result = (text != null ? text.hashCode() : 0);
        result = 29 * result + beginTime;
        result = 29 * result + endTime;
        return result;
    }
}
