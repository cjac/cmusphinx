package edu.cmu.sphinx.linguist.acoustic.tiedstate;
import edu.cmu.sphinx.linguist.acoustic.HMM;
import edu.cmu.sphinx.linguist.acoustic.HMMPosition;
import edu.cmu.sphinx.linguist.acoustic.Unit;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class TableSenone {
    class AI extends ArrayList <Integer> {
    }
    int [] cd2ci;
    AI [] lesCDdeCI;
    public TableSenone (Loader loader) {
	cd2ci = new int[loader.getSenonePool().size()];
	 HMMManager hmmManager = loader.getHMMManager();
	Iterator it=hmmManager.getIterator();
	int maxCI =0;
	while (it.hasNext()) {
	    
	    SenoneHMM hmm = (SenoneHMM) it.next();
	   
	    SenoneHMM base= (SenoneHMM) hmmManager.get(HMMPosition.UNDEFINED,hmm.getBaseUnit());
	    Senone [] cd= hmm.getSenoneSequence().getSenones();
	    Senone [] ci = base.getSenoneSequence().getSenones();
	    assert (cd.length==ci.length);
	    for (int i= 0; i< cd.length; i++){
		if (! (cd[i] instanceof CompositeSenone))
		    cd2ci[(int) cd[i].getID()]= (int)ci[i].getID();
		if (ci[i].getID()>=maxCI) maxCI=(int)ci[i].getID();
	    }
	    
	}
	lesCDdeCI = new  AI[maxCI+1];
	for (int i=0 ;i< lesCDdeCI.length; i++) 
	    lesCDdeCI[i]=new AI();
	for (int i =0; i<cd2ci.length; i++)
	    lesCDdeCI[cd2ci[i]].add(i);
    }
    public int lengthCI () {
	return lesCDdeCI.length;
    }
    public Integer[] lesCD(int i) {
	return lesCDdeCI[i].toArray(new Integer[0]);
    }
    public int toCi(int i) {
	return cd2ci[i];
    }
}
