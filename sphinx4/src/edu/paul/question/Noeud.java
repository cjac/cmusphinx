package edu.paul.question;
import java.util.List;
import java.util.LinkedList;
import java.util.Arrays;
import java.util.Locale;
import java.io.Serializable;
class Noeud  implements Serializable{
    float count;
    int feuille=-1;
    float [] poids;
    Integer []tri;
    float dist=0;
    int code;
    int numero;
    Etat etat;
    Noeud pere;
    transient List<Candidat> perePutatifs;
    Noeud gauche;
    Noeud droit;
   



    void expand(float val, List <Noeud> l) {
//       	System.err.print(this);
// 	if (gauche!=null) 
// 	    System.err.format("%f %f \n",gauche.dist,droit.dist);
// 	else
// 	    System.err.println("pas fils");
	if (gauche==null  || (dist <=val )) l.add(this);
	else { gauche.expand(val,l); droit.expand(val,l);
	}
    }

    public String toString() {
	return String.format("%s %5.3f %6f %s",pere==null?"rac ":"fils",dist,count,etat);
    }  
     void sort () {
	tri =new Integer[poids.length];
	for (int i =0 ; i<tri.length ;i++)
	    tri[i]=i;
	Arrays.sort(tri, new java.util.Comparator<Integer> () {
		public int compare (Integer i1, Integer i2) {
		    if (poids[i1]==poids[i2]) return 0;
		    if (poids[i1] >poids[i2]) return -1;
			return 1;
		}
	    });
    }
    float normalize() {
	float som=0.0f;
	for (int i=0; i<poids.length;i++)
	    som +=poids[i];
	for (int i=0;i<poids.length; i++)
	    poids[i] /=som;
	return som;
    }
    static  float ddiv_d (float [] p, float q[] ,Integer []tri) {
	float y=0.0f;
	for (int i =0 ; i<40; i++) {
	    int j=tri[i];
	    if (p[j]>0.00000001)
		y += p[j]*Math.log(p[j]/q[j]);
	}
	return y;
    }
    public Noeud(Etat etat,float[] poids,int code) {
	perePutatifs = new LinkedList<Candidat>();
	this.poids=poids;
	this.etat=etat;
	this.code=code;
	sort();
	count = normalize();
    }
    public Candidat fusion(Noeud autre) {
	float cout=ddiv_d(poids,autre.poids,tri)+
	    ddiv_d(autre.poids,poids,autre.tri);
	//cout=Math.max(cout-0.000001f,Math.max(dist,autre.dist))+ 0.000001f;
	Candidat n = new Candidat(cout,this,autre);
	assert (code==autre.code);
	this.perePutatifs.add(n);
	autre.perePutatifs.add(n);

	if (cout<dist || cout<autre.dist) 
	    System.err.format( "cout----- %f %f %f \n",cout,dist,autre.dist);
	return n;
    }

    public Noeud (Candidat cand) {
	droit=cand.droit;
	gauche=cand.gauche;
        dist=cand.dist;
	code=cand.droit.code;
	poids= new float[cand.droit.poids.length];
	for (int i =0 ; i<poids.length; i++) {
	    poids[i]=droit.poids[i]*droit.count + gauche.poids[i]*gauche.count;
	}
	count=normalize();
	sort();
	for (Candidat tmp: gauche.perePutatifs)
	    tmp.gauche=null;
	for (Candidat tmp: droit.perePutatifs)
	    tmp.gauche=null;

	gauche.perePutatifs.clear();
	perePutatifs = gauche.perePutatifs;
	gauche.perePutatifs=null;
	gauche.tri=null;
	droit.tri=null;
	gauche.poids=null;
	droit.poids=null;
	droit.perePutatifs.clear();droit.perePutatifs=null;
	gauche.pere=this;
	droit.pere=this;
    }
    private class Jeton {
	int numero=0;
	java.io.PrintStream sortie;
	private Jeton(){super();};
	Jeton(java.io.PrintStream sortie) {
	    this.sortie=sortie;
	}
	    
	int get() {return numero++;}
    }
    public void saveArbre(java.io.PrintStream sortie) {
	Jeton jeton=new Jeton(sortie);
	numero=jeton.get();
	parcours(jeton);
 }
    public void saveArbre(java.io.PrintStream sortie, int nCode ) {
	if (gauche==null && droit==null) 
	    sortie.format("%s %d\n",etat.toString(),nCode);
      	if (gauche!=null) gauche.saveArbre(sortie,nCode);
	if (droit!=null) droit.saveArbre(sortie,nCode);
    }
    void parcours(Jeton jeton) {
	jeton.sortie.format ("%d %d %d %f %f %s\n",numero, gauche!=null? gauche.numero=jeton.get():-1,
		       droit!=null ?droit.numero=jeton.get():-1,dist,count,etat!=null?etat.toString():"null");
	if (gauche!=null) gauche.parcours(jeton);
	if (droit!=null) droit.parcours(jeton);
    }
		       
    public void lesEtats(List l) {
	if (etat!=null) l.add(etat);
	else {
	    gauche.lesEtats(l);
	    droit.lesEtats(l);
	}
    }
    int compter() {
	if (gauche==null)
	    {assert (droit==null);
		return 1;
	    }
	return gauche.compter() +droit.compter();
    }
	

}




	

