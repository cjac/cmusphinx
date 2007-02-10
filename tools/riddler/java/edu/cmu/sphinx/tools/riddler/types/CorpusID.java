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
 * Time: 8:23:37 PM
 */

package edu.cmu.sphinx.tools.riddler.types;

import java.util.Arrays;
import java.util.List;

/**
 * A Corpus' unique identifier.  A Corpus consists of Items and has an associated Dictionary
 * mapping Words to pronunciations.
 * @see ItemID
 * @see PronunciationID
 */
public class CorpusID {
    long id;
    List<ItemID> itemIDs;
    DictionaryID dictionaryID;

    public CorpusID(long id, List<ItemID> itemIDs, DictionaryID dictionaryID) {
        this.id = id;
        this.itemIDs = itemIDs;
        this.dictionaryID = dictionaryID;
    }

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public ItemID[] getItemIDs() {
        return itemIDs.toArray(new ItemID[]{});
    }

    public void setItemIDs(ItemID[] itemIDs) {
        this.itemIDs = Arrays.asList(itemIDs);
    }

    public DictionaryID getDictionaryID() {
        return dictionaryID;
    }

    public void setDictionaryID(DictionaryID dictionaryID) {
        this.dictionaryID = dictionaryID;
    }
}
