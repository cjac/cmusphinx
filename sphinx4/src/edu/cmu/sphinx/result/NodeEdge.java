package edu.cmu.sphinx.result;
import edu.cmu.sphinx.linguist.dictionary.Word;
public interface NodeEdge {
    public int getBeginTime();
    public int getEndTime();
    public Word getWord();
    public double getPosterior();
    public int getFirstEndTime();
    public boolean isAncestorOf(NodeEdge e);
    public boolean getIsNotPruned();
    public Node getFromNode();
    public int getLastBeginTime();
}
