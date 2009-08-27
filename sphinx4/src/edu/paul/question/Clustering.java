package edu.paul.question;
import java.util.logging.Logger;
import java.util.logging.Level;
import java.util.Properties;
import java.util.Arrays;
import java.util.List;
import java.util.HashSet;
import java.util.HashMap;
import java.util.TreeMap;
import java.util.ArrayList;
import java.util.Iterator;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import java.io.ObjectOutputStream;
import java.io.FileOutputStream;
public class Clustering implements Configurable {
    class Tas extends ArrayList<Candidat> {
    }
    private String name;
    private Logger logger;
    private float maxi;
    private int cardFinal;
    private ConfigurationManager cm;
    private String dump;
    public void register(String name, Registry registry)
	throws PropertyException {
        this.name = name;
	registry.register("maxi",PropertyType.FLOAT);
	registry.register("cardFinal",PropertyType.INT);
	registry.register("dump",PropertyType.STRING);
    }
    public String getName() {
	return name;
    }
    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
        cm = ps.getPropertyManager();
	maxi=ps.getFloat("maxi",100.0f);
	cardFinal=ps.getInt("cardFinal",6500);
	dump=ps.getString("dump","");
    }

    
    void push (ArrayList <Candidat> leTas, Candidat n) {
	leTas.add(n);
	int posi=leTas.size()-1;
	int pere=posi/2;
	while (pere>0 && leTas.get(pere).dist>n.dist) {
	    leTas.set(posi,leTas.get(pere));
	    posi=pere;
	    pere=posi/2;
	}
	leTas.set(posi,n);
    }


    
    Candidat  pop(ArrayList <Candidat> leTas) {
	
	if (leTas.size()<=2) {
	    if (leTas.size()==2) return leTas.remove(leTas.size()-1);
	    return null;
	}
	Candidat retour=leTas.get(1);
	Candidat fin=leTas.get(leTas.size()-1);
	leTas.remove(leTas.size()-1);
	int posi=1;
	while (posi*2+1<=leTas.size()-1) {
	    int fils= leTas.get(posi*2).dist<leTas.get(posi*2+1).dist ? posi*2 :posi*2+1;
	    if (leTas.get(fils).dist<fin.dist){
		leTas.set(posi,leTas.get(fils));
		posi=fils;
	    }
	    else {leTas.set(posi,fin); return retour;}

	}
	if (posi*2<=leTas.size()-1 && leTas.get(posi*2).dist<fin.dist) {
	    int fils=posi*2;
	    leTas.set(posi,leTas.get(fils));
	    posi=fils;
	}
	leTas.set(posi,fin);
	return retour;}

		

	
    private class Couple {
	int nombre;
	float val;
      
	Couple (float val, int nombre) {
	    this.nombre=nombre;
	    this.val=val;
	}
    }
    private class UneListe {
	Iterator<Couple> ite;
	int nombre;
	int index;
	float val;
	UneListe (Iterator<Couple> ite , int index) {
	    this.ite=ite;
	    this.index=index;
	    avance ();
	}
	void avance() {
	    if (ite.hasNext()) {
		Couple c=ite.next();
		nombre =c.nombre;
		val=c.val;
	    } else
		val=Float.MAX_VALUE;
	};
    }
    int somme (UneListe[] listes) {
	int sum=0;
	for (UneListe l:listes) {
	    sum += l.nombre;
	}
	return sum;
    }
    float avance(UneListe[] listes) {
	int pos=0;
	float retour;
	for (int i=1; i<listes.length; i++)
	    if (listes[i].val<listes[pos].val)
		pos=i;
	retour=listes[pos].val;
	listes[pos].avance();
	
	return retour;
    }
       
    ArrayList<Noeud>  dumper(float val,TreeMap <Integer,HashSet <Noeud>> lesNoeuds) {
	ArrayList<Noeud> resul=new ArrayList<Noeud>(cardFinal/3);

	for ( HashSet <Noeud> noeuds : lesNoeuds.values())

	    {  for (Noeud n: noeuds){ 
		    
		if ( n.dist <= val) 
		    resul.add(n);
		else n.expand(val,resul);
		
		logger.info( "j'ai trouver n  modele" + resul.size()+ " "+ n.toString());
		}
	    }
	return resul;
    }
    void sauver( TreeMap <Integer, HashSet <Noeud>> lesNoeuds) {
	try {
	    ObjectOutputStream out=
		new ObjectOutputStream (new FileOutputStream (dump));
	    out.writeObject(lesNoeuds);
	}
	catch (java.io.IOException ex) {
	    ex.printStackTrace();
	}
    }

    ArrayList <Noeud> mettreEnOrdre(ArrayList <ArrayList <Couple>> niveaux , TreeMap <Integer, HashSet <Noeud>> lesNoeuds  ) {
	UneListe [] index=new UneListe[niveaux.size()];
	int i=0;
	for (ArrayList<Couple> c: niveaux) {
	    index[i]= new UneListe(c.iterator(),i);
	    i++;
	}
        float val;
	while ((val=avance(index)) <Float.MAX_VALUE && somme(index) >cardFinal)
	    if (logger.isLoggable(Level.FINE))  logger.fine(String.format(" elimination %f  %d  nombre de code :%d",val,somme(index),index.length));
	/// reste plus qua dumper .
	logger.info("seuil" + val);
	return dumper(val,lesNoeuds);
	
    }
    void compterClasse( Noeud [] t) {
	int anc=0;
	for (int i =0 ; i<t.length; i++) {
	    if (i==t.length-1 || t[i].code!= t[i+1].code) {
		logger.info(String.format("%d dure %d %s ",
					  t[i].code,i+1-anc,t[i].etat.getBase()));
			    anc=i+1;
	    }
	}
    }
	    

    void faire(List<Etat> lesEtats, float[][] mixture, ClassePhoneme classe) {
	Noeud [] tous= new Noeud[lesEtats.size()];
	TreeMap <Integer, HashSet<Noeud>> lesNoeuds =new TreeMap<Integer,HashSet <Noeud>>();
	
	double masse=0.0;
	int i=0;
	for (Etat  e: lesEtats)  {
	    tous[i] =new Noeud(e,mixture[i],classe.getClasse(Classification.BASE,e.getBase()));
	    masse+= tous[i++].count;
	}
	float tailleMaxi=(float)  (masse/cardFinal*maxi);
	Arrays.sort(tous,   new java.util.Comparator<Noeud> () {
		public int compare (Noeud  i1, Noeud i2) {
		    return i1.code-i2.code;
		    
		}
	    });
       
	if (logger.isLoggable(Level.INFO)) compterClasse(tous);       
	for (Noeud n :tous) {
	    // modif paul pour bloquer 
	    //if (n.code>=2) continue;
	    HashSet<Noeud> set = lesNoeuds.get(n.code);
	    if (set==null) {set =new HashSet<Noeud>();
		lesNoeuds.put(n.code,set);
	    }
	    set.add(n);
	}
	ArrayList <ArrayList <Couple>> niveaux= new ArrayList<ArrayList <Couple>> ();
	
	int restart=0;
	for (int theCode=0 ; theCode<classe.getSize(Classification.BASE)
		 ; theCode++) {
	    niveaux.add(new ArrayList<Couple>());
	int compteur=tous.length;
	int ancCompteur=compteur;
        Tas  lesTas[]= new Tas[2];
	for(int lll=0; lll<lesTas.length; lll++)
	    lesTas[lll]=new Tas();
	ArrayList<Candidat>  leTas=lesTas[0];
	int enService=0;
	leTas.add(new Candidat(0f,null,null));
	String base="";

	for (i =restart ; i<tous.length; i++){
	    int j;
	    if (tous[i].code!=theCode) {restart=i; break;}
	    for ( j=i+1; j<tous.length && tous[i].code==tous[j].code ; j++)
		
		if (tous[i].count+tous[j].count<tailleMaxi)	
		    push(leTas,tous[i].fusion(tous[j]));
	    if (false && logger.isLoggable(Level.FINEST)) {logger.finest(tous[j-1].etat.toString()+" "+ tous[j-1].code);
		logger.finest(tous[j].etat.toString()+ " "+tous[j].code);}
	    if ( i % 100 == 0 || ! base.equals(tous[i].etat.getBase()))
		logger.info(String.format(" fin des %s :%d %d %d",base,leTas.size(),i,j));
	    base=tous[i].etat.getBase();
	}
		
	Candidat  cand;
	logger.info(String.format("Start : taille Tas %d taille Modele %d taille Maxi %f",leTas.size(),compteur,tailleMaxi));


	while (	(cand=pop(leTas)) !=null && compteur>cardFinal) {
// 	    float avaleur=0.0f;
// 	    while (cand!=null && cand.gauche==null)
// 		{ assert avaleur<=cand.dist;
// 		    avaleur=cand.dist;
// 		    cand=pop(leTas);
// 		}
// 	    if (cand==null) break;
	    if (cand.gauche==null ) continue;
	    HashSet<Noeud> set = lesNoeuds.get(cand.gauche.code);
	    if (set.size() <=3) {
		System.err.println("g: "+ cand.gauche);
		System.err.println("g: "+ cand.droit);
		System.err.println( " cout : " +cand.dist + " : "+ cand.droit.fusion(cand.gauche).dist);
	    }
	    Noeud racine = new Noeud(cand);
	    
	    set.remove(racine.droit);set.remove(racine.gauche);
	    if (logger.isLoggable(Level.FINE)) logger.fine(String.format("tailleav Tas %d taille Modele %d",leTas.size(),compteur));
	    for (Noeud n:set)

		
		if (racine.count+ n.count <tailleMaxi){
		Candidat tempo=racine.fusion(n);		    
		push(leTas,tempo);
		assert(racine.dist<=tempo.dist);
		}
		
	    
	    set.add(racine);
	    niveaux.get(theCode).add(new Couple(racine.dist,set.size()));
	    if (logger.isLoggable(Level.FINE)) logger.fine(String.format("tailleapres Tas %d taille Modele %d   set : %d   dist %f count %f",leTas.size(),compteur,set.size(),racine.dist,racine.count));

	    if (!logger.isLoggable(Level.FINE)&& set.size()%100==0) logger.info(String.format("tailleapres Tas %d taille Modele %d   set : %d   dist %f count %f",leTas.size(),compteur,set.size(),racine.dist,racine.count));


	    compteur--;
	    if (compteur+1000 <ancCompteur) {
		ancCompteur=compteur;
		int suivant =(enService+ 1) % 2;
		lesTas[suivant].add(lesTas[enService].get(0));
		while ((cand=pop(leTas))!=null) 
		    if (cand.gauche !=null) lesTas[suivant].add(cand);
		enService=suivant;
		leTas.trimToSize();
		leTas=lesTas[enService];
	    }
		
	   
	}     
	logger.info("taille du tas " + leTas.size()); 
	if (false) {
	    for (Noeud n: lesNoeuds.get(theCode))
		n.saveArbre(System.out);
	lesNoeuds.get(theCode).clear();
	}
       }
	sauver (lesNoeuds);
      List<Noeud>  res=	mettreEnOrdre(niveaux, lesNoeuds);
      int nCluster=0;
      for (Noeud n : res) 
	  n.saveArbre(System.out,nCluster++);
    }


}