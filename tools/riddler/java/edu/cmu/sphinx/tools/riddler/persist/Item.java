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

package edu.cmu.sphinx.tools.riddler.persist;

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

    @Id
    @GeneratedValue(strategy = GenerationType.TABLE)
    private String id;

    @OneToOne(cascade = CascadeType.ALL, fetch = FetchType.EAGER, mappedBy = "item")
    private Audio audio;

    @OneToOne(cascade = CascadeType.ALL, fetch = FetchType.EAGER, mappedBy = "item")
    private Text text;

    /**
     * parent reference, for bi-directional fetching
     */
    @ManyToOne
    private Corpus corpus;

    public Item(Audio audio, Text text) {
        this.audio = audio;
        this.text = text;
    }

    public Item() {
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public Audio getAudio() {
        return audio;
    }

    public void setAudio(Audio audio) {
        this.audio = audio;
    }

    public Text getText() {
        return text;
    }

    public void setText(Text text) {
        this.text = text;
    }

    public Corpus getCorpus() {
        return corpus;
    }

    public void setCorpus(Corpus corpus) {
        this.corpus = corpus;
    }
}
