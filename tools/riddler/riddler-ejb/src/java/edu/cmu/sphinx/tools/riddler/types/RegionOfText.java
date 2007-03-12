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
 * Date: Mar 3, 2007
 * Time: 9:55:21 AM
 */

package edu.cmu.sphinx.tools.riddler.types;

import javax.persistence.*;

/**
 * document me!
 * @author Garrett Weinberg
 */
@Entity
public class RegionOfText {

    private int id;
    private int startIndex;
    private int endIndex;
    /**
     * parent reference, for bi-directional fetching
     */
    private Text text;

    public RegionOfText(int id, int startIndex, int endIndex) {
        this.id = id;
        this.startIndex = startIndex;
        this.endIndex = endIndex;
    }

    protected RegionOfText() {
    }

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public int getStartIndex() {
        return startIndex;
    }

    public void setStartIndex(int startIndex) {
        this.startIndex = startIndex;
    }

    public int getEndIndex() {
        return endIndex;
    }

    public void setEndIndex(int endIndex) {
        this.endIndex = endIndex;
    }

    public Text getText() {
        return text;
    }

    public void setText(Text text) {
        this.text = text;
    }
}
