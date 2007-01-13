package edu.cmu.sphinx.tools.corpus;

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
 * Date: Jan 9, 2007
 * Time: 7:57:40 AM
 */
public interface NoteInterface {
    int getBeginTime();

    void setBeginTime(int beginTime);

    int getEndTime();

    void setEndTime(int endTime);

    String getText();

    void setText(String text);
}

