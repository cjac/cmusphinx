/**
 * Copyright 1999-2007 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * <p/>
 * User: Garrett Weinberg
 * Date: Jan 26, 2007
 * Time: 11:24:07 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

/**
 * An Item's unique identifier.  Items belong to one or more Corpora.
 * An Item consists of an Audio record and/or a Text record. If it has both, the Text is serving
 * as a transcript of the Audio.
 * @see CorpusID
 * @see TextID
 * @see AudioID
 */
public class ItemID {
    long id;
    AudioID audioID;
    TextID textID;

    public ItemID(long id) {
        this.id = id;
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public AudioID getAudioID() {
        return audioID;
    }

    public void setAudioID(AudioID audioID) {
        this.audioID = audioID;
    }

    public TextID getTextID() {
        return textID;
    }

    public void setTextID(TextID textID) {
        this.textID = textID;
    }
}
