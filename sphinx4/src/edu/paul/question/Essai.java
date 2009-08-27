package edu.paul.question;
import java.util.HashMap;
import java.util.HashSet;
import java.util.TreeMap;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.FileInputStream;
import java.io.PrintWriter;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.List;

public class Essai {
    static int numero=0;
    static int feuille=0;
    static 	ClassePhoneme lesPhons;
    static PrintWriter feuilles=null;
    static void imprimer(Noeud n,PrintWriter out,char b) {
	List<Etat> l=new ArrayList<Etat>();
	n.lesEtats(l);
	int num=0;
	for (Etat e:l) {
	    if (false) out.format("%c%d %s gggg %s  %s dddd %s %s fin %s %s\n",
		       b,num++,e.description[0],lesPhons.classes(e.description[1],false),
		       e.description[1],
		       lesPhons.classes(e.description[2],false),
		       e.description[2],
		       e.description[3],e.description[4]);
	    else {
		out.format("%c%d %s %S %s %s\n& %s %s e%s\n",
			   b,num++,e.description[1],
			   lesPhons.classes(e.description[1],false),
			   e.description[2],
			   lesPhons.classes(e.description[2],false),
			   e.description[0],
			   e.description[3],e.description[4]);




	    }
	    out.format("@ base-%02d-%06d 1\n",n.code,n.numero);
	}
		       
    }
    static void feuille(Noeud n) {
	feuilles.format("base-%02d-%06d %d\n",n.code,n.numero,n.feuille);
    }



    static void traiter(Noeud n) {
	System.out.println("rac:"+n);
	//	n.gauche.sort();
	// n.droit.sort();
	System.out.println("g: "+ n.gauche);
	System.out.println("g: "+ n.droit);
	System.out.println( " cout : " + n.droit.fusion(n.gauche).dist);


    }

    static void traiterBis(Noeud n) throws FileNotFoundException{
	if (n.feuille>=0) { feuille(n) ;return;}
	PrintWriter out= new PrintWriter(String.format("quest/base%02d-%06d",n.code,n.numero));
	imprimer(n.gauche,out,'g');
	imprimer(n.droit,out,'d');
	out.close();
	traiter(n.droit);
	traiter(n.gauche);
    }
	

    static int profondeur (Noeud n)  {
	n.numero=numero++;
	n.feuille=-1;
	if (n.gauche==null|| n.dist<=1.5801647) {
	    n.feuille=feuille++;
	    return 0;}
	return 	1+ Math.max(profondeur(n.gauche),profondeur(n.droit));
    }


    public static  void  main (String [] argv) 
	throws IOException,ClassNotFoundException, FileNotFoundException {
	TreeMap <Integer,HashSet <Noeud>> lesNoeuds=null;
        lesPhons = new ClassePhoneme(argv[1]); 
	System.out.print(lesPhons.dump());
	feuilles= new PrintWriter("lesfeuilles");
	    
	ObjectInputStream in =new ObjectInputStream(new FileInputStream(argv[0]));
	lesNoeuds=
	    (TreeMap <Integer,HashSet <Noeud>> ) in.readObject();
	in.close();

	// 	catch (IOException e ) {
	// 	    e.printStackTrace();
	// 	}
	// 	catch   ( ClassNotFoundException e){
	// 	} 

	for (Integer i  : lesNoeuds.keySet()) {
	    System.out.format("set:  %3d %5d \n",i.intValue(),lesNoeuds.get(i).size());
	    numero=0;
	    for (Noeud n : lesNoeuds.get(i) ){
		int c;

		System.out.format(" n:%d p:%d",c=n.compter(),profondeur(n));
		if (c!=1) System.out.format(" [%.0f]",n.count);
		else
		    System.out.format(" [%s]",n.toString());
		traiter(n);
	    } 
	    System.out.println();
	    // 	    for (Noeud n: ns) 
	    // 		n.saveArbre(System.out);
	}

       

    }
}
