package edu.cmu.sphinx.tools.corpus;

import edu.cmu.sphinx.tools.audio.AudioDataInterface;

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
 * Time: 7:52:03 AM
 */
public interface ShortAudioDataInterface extends AudioDataInterface {
    public short[] getAudioData();
    public void setAudioData(short[] data);
}

