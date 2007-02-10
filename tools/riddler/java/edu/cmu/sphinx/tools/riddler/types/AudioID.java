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

import java.util.Arrays;
import java.util.List;

/**
 * Unique identifier for a piece of audio data.  Audio data belongs to an Item and consists of one or more
 * RegionOfAudio records.
 * @see ItemID
 * @see RegionOfAudioID
 */
public class AudioID {
    long id;
    List<RegionOfAudioID> audioRegionIDs;

    public AudioID(long id, List<RegionOfAudioID> audioRegionIDs) {
        this.id = id;
        this.audioRegionIDs = audioRegionIDs;
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public RegionOfAudioID[] getAudioRegionIDs() {
        return audioRegionIDs.toArray(new RegionOfAudioID[]{});
    }

    public void setAudioRegionIDs(RegionOfAudioID[] audioRegionIDs) {
        this.audioRegionIDs = Arrays.asList(audioRegionIDs);
    }
}
