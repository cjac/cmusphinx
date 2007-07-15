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
 * Date: Mar 8, 2007
 * Time: 11:12:23 PM
 */

package edu.cmu.sphinx.tools.riddler.persist;

import javax.persistence.*;

/**
 * document me!
 * @author Garrett Weinberg
 */
@Entity
public class RegionOfAudio {

    @Id
    @GeneratedValue(strategy = GenerationType.TABLE)
    private String id;

    private int beginTime;
    private int endTime;

    @OneToOne(cascade = CascadeType.PERSIST, fetch = FetchType.EAGER)
    private RegionOfText textRegion;

    /**
     * parent reference, for bi-directional fetching
     */
    @ManyToOne
    private Audio audio;

    protected RegionOfAudio() {
    }

    public RegionOfAudio(int beginTime, int endTime, RegionOfText textRegion) {
        this.beginTime = beginTime;
        this.endTime = endTime;
        this.textRegion = textRegion;
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

    public RegionOfText getTextRegion() {
        return textRegion;
    }

    public void setTextRegion(RegionOfText textRegion) {
        this.textRegion = textRegion;
    }
}
