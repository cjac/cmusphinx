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

package edu.cmu.sphinx.tools.riddler.types;

import javax.persistence.*;

/**
 * document me!
 * @author Garrett Weinberg
 */
@Entity
public class RegionOfAudio {

    private int id;
    private int beginTime;
    private int endTime;
    private RegionOfText textRegion;
    /**
     * parent reference, for bi-directional fetching
     */
    private Audio audio;

    protected RegionOfAudio() {
    }

    public RegionOfAudio(int id, int beginTime, int endTime, RegionOfText textRegion) {
        this.id = id;
        this.beginTime = beginTime;
        this.endTime = endTime;
        this.textRegion = textRegion;
    }

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    @ManyToOne
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

    @OneToOne(cascade = CascadeType.PERSIST, fetch = FetchType.EAGER)
    public RegionOfText getTextRegion() {
        return textRegion;
    }

    public void setTextRegion(RegionOfText textRegion) {
        this.textRegion = textRegion;
    }
}
