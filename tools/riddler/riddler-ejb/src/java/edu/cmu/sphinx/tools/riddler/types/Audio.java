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
 * Date: Jan 13, 2007
 * Time: 8:32:49 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

import edu.cmu.sphinx.tools.riddler.types.audio.AudioDescriptor;

import javax.persistence.*;
import java.util.List;

/**
 * Unique identifier for a piece of audio data.  Audio data belongs to an Item and consists of one or more
 * RegionOfAudio records.
 * @author Garrett Weinberg
 */
@Entity
public class Audio {
    private int id;
    private AudioDescriptor audioDescriptor;
    private List<RegionOfAudio> audioRegions;
    /**
     * parent reference, for bi-directional fetching
     */
    private Item item;

    public Audio(int id, AudioDescriptor audioDescriptor, List<RegionOfAudio> audioRegions) {
        this.id = id;
        this.audioDescriptor = audioDescriptor;
        this.audioRegions = audioRegions;
    }

    public Audio() {
    }

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    @OneToMany(cascade = CascadeType.ALL, fetch = FetchType.EAGER, mappedBy="audio")
    public List<RegionOfAudio> getAudioRegions() {
        return audioRegions;
    }

    public void setAudioRegions(List<RegionOfAudio> audioRegions) {
        this.audioRegions = audioRegions;
    }

    @OneToOne(cascade = CascadeType.ALL, fetch = FetchType.EAGER)
    public AudioDescriptor getAudioDescriptor() {
        return audioDescriptor;
    }

    public void setAudioDescriptor(AudioDescriptor audioDescriptor) {
        this.audioDescriptor = audioDescriptor;
    }

    @OneToOne
    public Item getItem() {
        return item;
    }

    public void setItem(Item item) {
        this.item = item;
    }
}
