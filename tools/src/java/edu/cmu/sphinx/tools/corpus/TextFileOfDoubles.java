package edu.cmu.sphinx.tools.corpus;

/**
 * Copyright 1999-2006 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * Portions Copyright 2002 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * User: Peter Wolf
 * Date: Mar 11, 2006
 * Time: 1:56:23 PM
 */
public class TextFileOfDoubles {

    String textFile;

    public String getTextFile() {
        return textFile;
    }

    public void setTextFile(String textFile) {
        this.textFile = textFile;
    }

    public TextFileOfDoubles() {

    }

    public double[] read(int offset, int length) {
        return new double[0];  //To change body of implemented methods use File | Settings | File Templates.
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final TextFileOfDoubles that = (TextFileOfDoubles) o;

        if (textFile != null ? !textFile.equals(that.textFile) : that.textFile != null) return false;

        return true;
    }

    public int hashCode() {
        return (textFile != null ? textFile.hashCode() : 0);
    }
}
