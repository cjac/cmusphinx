package edu.paul.question;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.TreeSet;
import java.util.EnumMap;
import java.util.List;

class ClassePhoneme {
    class Critere {
	Classification critere;
	ArrayList <ArrayList< String>> contenu;
	HashMap <String,Integer> valeur;

	Critere (Classification critere) {
	    this.critere=critere;
	    contenu=new ArrayList <ArrayList <String>> ();
	    valeur = new HashMap<String,Integer> ();
	    
	}
        int getNewClasse() {
	    int n=contenu.size();
	    contenu.add(new ArrayList<String>());
	    return n;
	}
	ArrayList<String> getLaClasse(int n ) {
	    return contenu.get(n);
	}
	int size() {return contenu.size();}
	int getClasse(String n,boolean b ) {
	    assert (n!=null);
	    Integer temp= valeur.get(n);
	    if (temp!=null) return temp;
	    if (b) System.err.println ("pas de classe pour "+n + "dans"  +critere);
	    return -1;
	}
	void add(int n,String ph) {
	    contenu.get(n).add(ph);
	    valeur.put(ph,n);
	}
	
    }
	
 
    private EnumMap<Classification,Critere> lesCriteres;
	TreeSet<String> lesPhonemes;
    public ClassePhoneme( String name ) throws IOException {
	lesCriteres=new EnumMap<Classification,Critere>(Classification.class);
	java.io.BufferedReader in
	    = new java.io.BufferedReader(new java.io.FileReader(name));
	String line;
	lesPhonemes=new TreeSet<String> ();
	while ((line=in.readLine())!= null) {
	    String [] mots = line.split("\\s+");
	    Classification classe=Classification.valueOf(mots[0]);
	    Critere crit= lesCriteres.get(classe);
	    if (crit==null) {
		crit=new Critere(classe);
		lesCriteres.put(classe,crit);
	    }
	    int index=crit.getNewClasse();
	    for (int i=1; i<mots.length; i++){
		crit.add(index,mots[i]);
		lesPhonemes.add(mots[i]);
		}
	}
	for (Classification cle : lesCriteres.keySet()) {
	    System.err.print(cle+" ");
	    for (ArrayList<String> liste : lesCriteres.get(cle).contenu) {
		for (String mot: liste) 
		    System.err.format("(%s,%d) ",mot,lesCriteres.get(cle).getClasse(mot,true));
		System.err.println();
	    }}
    }
    public int getSize(Classification uneClasse) {
	Critere crit=lesCriteres.get(uneClasse);
	return crit==null ? 0 : crit.size();
    }
    public List<String> getClasse(Classification uneClasse, int n) {
	return lesCriteres.get(uneClasse).getLaClasse(n);
    }
    public int getClasse(Classification c, String ph) {
	if (false) System.err.println(c+" pho:"+ph+":");
	return lesCriteres.get(c).getClasse(ph,false);
    }
    public String classes(String ph) {
	return classes(ph,true);
    }
    public String classes(String ph,boolean b) {
	StringBuffer s=new StringBuffer();
	classes(ph,s, b);
	return s.toString();
    }
    private void classes(String ph,StringBuffer s,boolean b) {
	boolean pasFirst=false;
	for (Classification k:lesCriteres.keySet()) {
	    Integer val=lesCriteres.get(k).getClasse(ph,b);
	    if (val!=null) {
		if (pasFirst) s.append(" ");
		s.append(k);
		s.append(val);
		pasFirst=true;
	    }
	}
    }
    public String dump()  {
	StringBuffer s= new StringBuffer();
	for (String ph : lesPhonemes) {
	    s.append(ph);
	    s.append(" : ");
	    classes(ph,s,true);
	    s.append("\n");
	}
	return s.toString();
    }
	
	
    
}
