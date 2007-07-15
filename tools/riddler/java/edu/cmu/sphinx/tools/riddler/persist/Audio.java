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

package edu.cmu.sphinx.tools.riddler.persist;

import edu.cmu.sphinx.tools.riddler.persist.audio.AudioDescriptor;

import javax.persistence.*;
import java.util.List;
import java.util.ArrayList;

/**
 * Unique identifier for a piece of audio data.  Audio data belongs to an Item and consists of one or more
 * RegionOfAudio records.
 * @author Garrett Weinberg
 */
@Entity
public class Audio {
    @Id
    @GeneratedValue(strategy = GenerationType.TABLE)
    private String id;

    @OneToOne(cascade = CascadeType.ALL, fetch = FetchType.EAGER)
    private AudioDescriptor audioDescriptor;

    @OneToMany(cascade = CascadeType.ALL, fetch = FetchType.EAGER, mappedBy="audio")
    private List<RegionOfAudio> audioRegions = new ArrayList<RegionOfAudio>();

    /**
     * parent reference, for bi-directional fetching
     */
    @OneToOne
    private Item item;

    public Audio(AudioDescriptor audioDescriptor, List<RegionOfAudio> audioRegions) {
        this.audioDescriptor = audioDescriptor;
        this.audioRegions = audioRegions;
    }

    public Audio() {
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public List<RegionOfAudio> getAudioRegions() {
        return audioRegions;
    }

    public void setAudioRegions(List<RegionOfAudio> audioRegions) {
        this.audioRegions = audioRegions;
    }

    public AudioDescriptor getAudioDescriptor() {
        return audioDescriptor;
    }

    public void setAudioDescriptor(AudioDescriptor audioDescriptor) {
        this.audioDescriptor = audioDescriptor;
    }

    public Item getItem() {
        return item;
    }

    public void setItem(Item item) {
        this.item = item;
    }
}
