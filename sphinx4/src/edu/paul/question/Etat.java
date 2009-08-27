package edu.paul.question;
import java.io.Serializable;
class Etat implements Serializable {

    String  []description;
    int code;


    public   Etat(String base, String gauche, String droite, String position, int numero, int code) {
	description = new String[5];
	description[0]=base;
	description[1]=gauche;
	description[2]=droite;
	description[3]=position;
	description[4]=Integer.toString(numero);
	this.code=code;
    }
    public String getBase() {
	return description[0];
    }
    public String toString() {
	StringBuffer sb=new StringBuffer();
	for (String s:description) {
	    sb.append(s) ;sb.append(" ");
	}
	sb.append(code);
	return sb.toString();
    }
}
