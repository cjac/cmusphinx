package edu.paul.majuscule;

import edu.cmu.sphinx.linguist.WordSequence;

class Token {
    float score;
    Token pred;
    int ligne;
    State  history;
    public Token(float score,Token pred, int ligne, State hist) 
    {this.score=score;this.pred=pred; this.ligne=ligne; history=hist;
    }
}
    

