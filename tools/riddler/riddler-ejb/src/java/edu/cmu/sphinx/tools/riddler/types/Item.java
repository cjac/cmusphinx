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

import javax.persistence.*;

/**
 * An Item's unique identifier.  Items belong to one or more Corpora.
 * An Item consists of an Audio record and/or a Text record. If it has both, the Text is serving
 * as a transcript of the Audio.
 * @see Corpus
 * @see Text
 * @see Audio
 * @author Garrett Weinberg
 */
@Entity
public class Item {
    private int id;
    private Audio audio;
    private Text text;
    /**
     * parent reference, for bi-directional fetching
     */
    private Corpus corpus;

    public Item(int id, Audio audio, Text text) {
        this.id = id;
        this.audio = audio;
        this.text = text;
    }

    public Item() {
    }

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    @OneToOne(cascade = CascadeType.ALL, fetch = FetchType.EAGER, mappedBy = "item")
    public Audio getAudio() {
        return audio;
    }

    public void setAudio(Audio audio) {
        this.audio = audio;
    }

    @OneToOne(cascade = CascadeType.ALL, fetch = FetchType.EAGER, mappedBy = "item")
    public Text getText() {
        return text;
    }

    public void setText(Text text) {
        this.text = text;
    }

    @ManyToOne
    public Corpus getCorpus() {
        return corpus;
    }

    public void setCorpus(Corpus corpus) {
        this.corpus = corpus;
    }
}
