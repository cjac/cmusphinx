package edu.paul.majuscule;

import edu.cmu.sphinx.linguist.WordSequence;

class State {
    WordSequence ws ;
    Object tete;

    public String toString() {
	return ws.toString() + " "+ tete.toString();
	    }
    public int hashCode() {
	return ws.hashCode();// +37* tete.hashCode();
    }
    public boolean equals (Object o) {
	if (o instanceof State) {
	    State ot= (State) o;
	    return ot.ws.equals(ws) ;//&& ot.tete.equals(tete);
	} 
	return false;
    }
    public State(WordSequence ws, Object o) {
	this.ws=ws;
	this.tete =o;
    }



}
    

