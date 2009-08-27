package edu.paul.split;
import java.util.logging.Logger;
import java.util.ArrayList;
import java.util.ListIterator;
import java.io.IOException;
import edu.cmu.sphinx.util.props.Configurable;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;
import edu.cmu.sphinx.util.props.PropertySheet;
import edu.cmu.sphinx.util.props.PropertyType;
import edu.cmu.sphinx.util.props.Registry;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.Loader;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.MixtureComponent;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.Pool;
import edu.cmu.sphinx.linguist.acoustic.tiedstate.GaussianMixture;
import edu.cmu.sphinx.frontend.FloatData;
import edu.cmu.sphinx.util.LogMath;

public class Stats implements Configurable {
    class MixtureComponentAvecPoids {
	MixtureComponent mix;
	double poids;
	MixtureComponentAvecPoids(MixtureComponent mix, double poids){
	    this();
	    this.mix=mix;
	    this.poids=poids;
	}
	private MixtureComponentAvecPoids() {
	}
	MixtureComponentAvecPoids copie() {
	    return new  MixtureComponentAvecPoids(new MixtureComponent(logMath,mix.getMean().clone(),
								       null,null,mix.getVariance().clone(),
								       null,null,0.0f,1e-4f),
						  poids);
	}
    }
    class ALM extends ArrayList <MixtureComponentAvecPoids> {
	ALM(int n) {
	    super(n);
	}
	ALM copie(){
	    ALM ret=new ALM(this.size());
	    for ( MixtureComponentAvecPoids m: this)
		ret.add(m.copie());
	    return ret;
	}
	    
	
    };
    
    private String name;
    private Logger logger;
    private edu.paul.bw.Saver saver;
    private LogMath logMath;
    private Loader loader;
    private float divVar=10.0f;
    private boolean initAlea;
    public void register(String name, Registry registry)       
	throws PropertyException {
        this.name = name;
        registry.register("saver", PropertyType.COMPONENT);
        registry.register("divVar", PropertyType.FLOAT);
        registry.register("initAlea", PropertyType.BOOLEAN);
	registry.register("logMath", PropertyType.COMPONENT);
    }  
    /*
     * (non-Javadoc)
     * 
     * @see edu.cmu.sphinx.util.props.Configurable#getName()
     */
    public String getName() {
        return name;
    }

    public void newProperties(PropertySheet ps) throws PropertyException {
        logger = ps.getLogger();
     
	logMath = (LogMath) ps.getComponent("logMath", LogMath.class);
	saver = (edu.paul.bw.Saver) ps.getComponent("saver",edu.paul.bw.Saver.class);
	divVar= ps.getFloat("divVar",10.0f);
	initAlea=ps.getBoolean("initAlea",false);
    }
    private double [][] accuMean;
    private double [][] accuVar;
    private double [] accuPoids;

    private ALM modele;
    private FloatData [] data;
    private float [][] noyaux;
    private int [] classe;
    private Pool gm;
    private ArrayList <ALM> lescis;
    void copie(float [] dest, float [] src) {
	for (int l=0;l<dest.length ;l++)
	    dest[l]=src[l];
    }

    void copie(int n , ALM maMixture) {
	GaussianMixture m=(GaussianMixture) gm.get(n);
	MixtureComponent [] lesmixs=m.getMixtureComponents();
	float [] logPoids=m.getLogMixtureWeights();
	for (int i =0; i<lesmixs.length;i++) {
	    copie( lesmixs[i].getVariance(),maMixture.get(i).mix.getVariance());
	    copie( lesmixs[i].getMean(),maMixture.get(i).mix.getMean());
	    logPoids[i]=logMath.linearToLog(maMixture.get(i).poids);
	}
    }
    public void init(int n) {

	GaussianMixture m=(GaussianMixture) gm.get(n);
	MixtureComponent [] lesmixs=m.getMixtureComponents();
	/*	noyaux=new float[lesmixs.length][39];
	for (int i =0; i< noyaux.length;i++)
	    for (int l=0 ;l<noyaux[i].length; l++)
		noyaux[i][l]=lesmixs[i].getMean()[l];
	*/
	
	ALM ret=new ALM (noyaux.length);
	float [] logPoids= m.getLogMixtureWeights();
	double[] poids= new double[logPoids.length];	
	for (int i =0; i<poids.length; i++)	
		poids[i]=logMath.logToLinear(logPoids[i]);
	for (int i=0 ;i<poids.length ;i++){
		float [] variance = lesmixs[i].getVariance().clone();
		ret.add(new MixtureComponentAvecPoids(new MixtureComponent(logMath,

									   lesmixs[i].getMean().clone(),
			       null,null,variance,null,null,
			       0.0f,1e-4f), 
	  poids[i]));
	}
	modele=ret;
	}
			 

    private void compare(int  n,FloatData[] lesds) {
    	GaussianMixture m=(GaussianMixture) gm.get(n);
    	float tv1=0.0f, tv2=0.0f;
    for (FloatData d : lesds) {
    	double v1=	m.getScoreDouble(d);
    	double v2=0.0;
    	for (MixtureComponentAvecPoids mix :modele) {
    		v2+= mix.poids * logMath.logToLinear(mix.mix.getScore(d));
    		}
    	tv1+=logMath.linearToLog(v1);
    	tv2+=logMath.linearToLog(v2);
    	//System.err.format( " comp %e %e \n",v1,v2);
    	}
    System.err.format( " comp %e %e \n",tv1,tv2);
    }
    public void init(FloatData [] data, float [][] noyaux, Loader loader) {
	this.data=data;
	this.noyaux=noyaux;
	this.loader=loader;
	modele=null;
	gm=null;
	
	if (loader!=null){
	    if (lescis==null) lescis=new ArrayList <ALM>();
	    gm = loader.getSenonePool();
	    logger.fine("gm " + gm.size());}

    }
	
    private float [] moyenne (FloatData [] data) {
	float [] ret = new float[data[0].getValues().length];
	double [] cumul = new double[data[0].getValues().length];
	for (int i =0 ;i<data.length ;i++)
	    for (int l=0; l<data[i].getValues().length;l++)
		cumul[l] += data[i].getValues()[l];
	for (int l=0; l<cumul.length;l++)
	    ret[l]=(float) (cumul[l]/data.length);
	return ret;
    }
    private float [] variance (FloatData [] data, float [] moy) {
	float [] ret = new float[data[0].getValues().length];
	double [] cumul = new double[data[0].getValues().length];
	for (int i =0 ;i<data.length ;i++)
	    for (int l=0; l<data[i].getValues().length;l++)
		cumul[l] += (data[i].getValues()[l]-moy[l]) *
		    (data[i].getValues()[l]-moy[l]);
	for (int l=0; l<cumul.length;l++)
	    ret[l]=(float) (cumul[l]/(data.length-1));
	return ret;
    }	
	
    private ALM initMix (float [][] noyaux, FloatData [] data) {
	float [] moy =moyenne(data);
	float [] var = variance(data,moy);
	for (int l=0 ; l<var.length ; l++)
	    var[l] = var[l]/divVar;
	ALM ret = new ALM (noyaux.length);
	for (int i=0; i<noyaux.length;i++)
	    ret.add(new MixtureComponentAvecPoids(new MixtureComponent(logMath,
								       noyaux[i],
								       null,null,var.clone(),null,null,
								       0.0f,1e-4f), 
						  1.0/noyaux.length));
	return ret;
    }

    double makeAccu( ALM lesMix , FloatData [] data ){
	double vrais=0.0;
	double miniProb=1.0;
	double [] proba= new double[lesMix.size()];
	if (accuVar==null || accuVar.length != lesMix.size())
	    accuVar= new double[lesMix.size()][data[0].getValues().length];
	else
	    for (int i=0 ;i< accuVar.length; i++)
		for (int l=0 ;l<accuVar[i].length; l++)
		    accuVar[i][l]=0.0;
	if (accuMean==null || accuMean.length != lesMix.size())
	    accuMean= new double[lesMix.size()][data[0].getValues().length];
	else
	    for (int i=0 ;i< accuMean.length; i++)
		for (int l=0 ;l<accuMean[i].length; l++)
		    accuMean[i][l]=0.0;
	if (accuPoids==null || accuPoids.length != lesMix.size())
	    accuPoids = new double[lesMix.size()];
	else
	    for (int i=0 ;i< accuPoids.length; i++)
		accuPoids[i]=0.0;
	double []dd = new double[data[0].getValues().length];
	for (FloatData d : data) {
	    for (int l =0 ; l<d.getValues().length;l++)
		dd[l]=d.getValues()[l]*d.getValues()[l];
	    double tout=0.0;
	    int id=0;
	    double somPartiel=0.0;
	    for (MixtureComponentAvecPoids mix: lesMix) 
		tout += proba[id++] =mix.poids* logMath.logToLinear(mix.mix.getScore(d));
	    if (tout <miniProb) miniProb=tout;
	    if(tout<1e-300) continue;
	    id=0;
	    for (MixtureComponentAvecPoids mix: lesMix){
		double partiel= proba[id]/tout;
		accuPoids[id]+=partiel;
		somPartiel += partiel;
		for (int l=0 ; l<d.getValues().length; l++) {
		    accuMean[id][l] += partiel*d.getValues()[l];
		    accuVar[id][l] += partiel*dd[l];
		}
		id++;
	    }
	    vrais+= logMath.linearToLog(tout);
	    assert(Math.abs(somPartiel-1.0) < 1e-4  );
	}
	logger.fine("mini prob :" + miniProb);
	return vrais;
    }

    void mise_a_jour(ALM lesmixs,boolean poids, boolean mean, boolean var) {
	double masse=0;
	if (true || poids) {
	    for (double x :accuPoids) masse+=x;
	    logger.info("la masse est total :"+ masse);
		   
	}
	int id=0;
	for (MixtureComponentAvecPoids mix : lesmixs) {
	    if (mean ){
		float [] m=mix.mix.getMean();
		for (int l=0 ; l < m.length; l++)
		    m[l] =(float)( accuMean[id][l]/accuPoids[id]);
	    }
	    if (var){
		float []m=mix.mix.getMean();
		float []v=mix.mix.getVariance();
		for (int l =0; l<v.length ;l++)
		    v[l]=(float)(accuVar[id][l]/accuPoids[id]-m[l]*m[l]);
	    }
	    if (poids)
		mix.poids=accuPoids[id]/masse;
		
	    if (mean | var)
		mix.mix.precomputeDistance();

	    id++;
	}
    }
    void clustering (ALM lesmixs, int npas) {
	float [][] distances = new float[lesmixs.size()][lesmixs.size()];
	for (int i=0 ;i<distances.length; i++)
	    for (int j=0; j<i; j++)
		distances[i][j]=distance(lesmixs.get(i),lesmixs.get(j));
	for (int pas=0 ; pas<npas; pas++) {
	    int si=0,sj=0;
	    float mini=1e30f;
	    for (int i=0 ;i<distances.length; i++)
		for (int j=0; j<i; j++)
		    if (distances[i][j]<mini) {
			si=i;sj=j;mini=distances[i][j];
		    }
	    logger.fine(String.format("perte : %f %d %d ",mini,si,sj));
	    fusion(lesmixs.get(si),lesmixs.get(sj));
	    for (int i=0;i<sj;i++ ) distances[sj][i]=1e30f;
	    for (int i=sj+1;i<distances.length;i++)
		distances[i][sj]=1e30f;
	    for (int j=0; j<si;j++)
		if (distances[si][j]<1e29f) distances[si][j]=distance(lesmixs.get(si),lesmixs.get(j));
	    for(int i=si+1;i<distances.length;i++)
		if (distances[i][si]<1e29f)
		    distances[i][si]=distance(lesmixs.get(i),lesmixs.get(si));
	}
	
    }
    private float[] vtemp=null;
    void fusionVar(MixtureComponentAvecPoids m1, MixtureComponentAvecPoids m2) {
	if (vtemp==null) vtemp=new float[m1.mix.getVariance().length];
	double n1=m1.poids, n2=m2.poids;
	double n3=n1+n2;
	float [] moy1=m1.mix.getMean(), moy2=m2.mix.getMean();
	for (int l=0; l<vtemp.length; l++)
	    vtemp[l]=(float) (n1/n3*m1.mix.getVariance()[l]+
			      n2/n3*m2.mix.getVariance()[l]+
			      n1*n2/(n3*n3)*(moy1[l]-moy2[l])*(moy1[l]-moy2[l]));
    }

    float distance(MixtureComponentAvecPoids m1, MixtureComponentAvecPoids m2){
	double sum1=0.0,sum2=0.0,sum3=0.0;
	fusionVar(m1,m2);
	// je mets a jour vtemp
	for (int l=0 ; l<vtemp.length;l++) {
	    sum1 += Math.log(m1.mix.getVariance()[l]);
	    sum2 += Math.log(m2.mix.getVariance()[l]);
	    sum3 += Math.log(vtemp[l]);
	}
	return (float) ( (sum3*(m1.poids+m2.poids)-sum1*m1.poids-sum2*m2.poids)/2.0);
    }
    void fusion(MixtureComponentAvecPoids m1, MixtureComponentAvecPoids m2) {
	// on met m2 dans m1 et j'annule le poids de m2
	fusionVar(m1,m2);
	// je mets a jour vtemp
	double n1=m1.poids, n2=m2.poids;
	double n3=n1+n2;
	float [] moy1=m1.mix.getMean(), moy2=m2.mix.getMean(),
	    v=m1.mix.getVariance();
	for (int l=0 ;l<vtemp.length; l++){
	    moy1[l] =(float) ((n1*moy1[l]+ n2*moy2[l])/n3);
	    v[l] = vtemp[l];
	}
	m1.poids=n3;
	m2.poids=0.0;
    }
    public float vraisemblance(FloatData [] lesds, int n) {
	float resu=0.0f;
	GaussianMixture m= (GaussianMixture) gm.get(n);
	for (FloatData d:lesds)
	    resu+= m.getScore(d);
	return resu;
    }
    public void traiter(FloatData [] data, float [][] noyaux,Loader loader, int n, int ci) {
	boolean tailleInsuffisante=false;
	int tailleMixs;
	logger.info("data taille : " + data.length+ " cd:"+n + "  ci:"+ci);
	init(data,noyaux,loader);
	tailleInsuffisante= data.length<200;
	//if (noyaux==null)
	    init(n);
	    if (n==ci) tailleMixs=32;
	    else tailleMixs=22;

	    ALM lesmixs;
	    if(n!=ci || initAlea || lescis.size()<=ci || lescis.get(ci)==null)
		lesmixs=initMix(this.noyaux,data);
	    else lesmixs= lescis.get(ci).copie();
	double vrais=0.0;
	if (modele!=null) compare(n,data);
	if (modele!=null) 
		logger.info(String.format("vraisemblance du modele par mix  :%e",
				makeAccu(modele,data)));
	for (int i=0; i<3 ;i++){
	    vrais=makeAccu(lesmixs,data);
	    logger.info(String.format("passe %d vrais: %e",i,vrais));
	    mise_a_jour(lesmixs,!tailleInsuffisante,tailleInsuffisante,tailleInsuffisante);
	}
	while (lesmixs.size()>tailleMixs) {
	    int reduction =(2*lesmixs.size()+2)/3;
	    clustering(lesmixs,Math.min(lesmixs.size()-22,reduction));
	    reduire(lesmixs);
	    for (int i=0; i<2 ;i++){
		vrais=makeAccu(lesmixs,data);
		logger.info(String.format("size : %d passe %d vrais: %e",lesmixs.size(),i,vrais));
		mise_a_jour(lesmixs,!tailleInsuffisante,tailleInsuffisante,tailleInsuffisante);
	    }
	}
	for (int i=0; i<10 ;i++){
	    double oldVrais=vrais;
	    vrais=makeAccu(lesmixs,data);
	    logger.info(String.format("size : %d passe %d vrais: %e",lesmixs.size(),i,vrais));
	    mise_a_jour(lesmixs,i>0,true,true);
	    if (Math.abs(vrais-oldVrais)/(Math.abs(oldVrais)+1.0)<1e-3) break;
	}
       
	if (n==ci) {while(lescis.size()<=n) lescis.add(null);lescis.set(n,lesmixs);}
	else copie(n,lesmixs);
	if (false && gm!=null) 
	    logger.info(String.format("vraisemblance du modele :%e",
				  vraisemblance(data,n)));

	
	if (modele!=null) {
		logger.info(String.format("vraisemblance du modele par mix  :%e",
					  makeAccu(modele,data)));
		for (int i=0; i<10 ;i++){
		    double oldVrais=vrais;
		    vrais=makeAccu(modele,data);
		    logger.info(String.format("modele base : %d passe %d vrais: %e",modele.size(),i,vrais));
		    mise_a_jour(modele,i>0,true,true);
		    if (Math.abs(vrais-oldVrais)/(Math.abs(oldVrais)+1.0)<1e-4) break;
	}
	


}

    }
    
    void reduire( ALM lesmixs) {
	ListIterator<MixtureComponentAvecPoids> it =lesmixs.listIterator();
	while(it.hasNext())
	    { MixtureComponentAvecPoids mix=it.next();
		if (mix.poids<1e-9) it.remove();
		else mix.mix.precomputeDistance();
	    }
    }

 protected final static String NUM_GAUSSIANS_PER_STATE = "num_gaussians";

    void save(String ext)throws IOException {
	Pool means= loader.getMeansPool();
	Pool mixtureWeights= loader.getMixtureWeightPool();
	int nMixture= mixtureWeights.size() ;
	int nGaus=means.size();
	int nGausByMixture=mixtureWeights.getFeature(NUM_GAUSSIANS_PER_STATE, 0);
	int vectLen=((float [])means.get(0)).length;
       	float [] mixture=new float[nGausByMixture] ;
	//float [][] globalMean= new float[nGaus][vectLen] ;
	java.io.DataOutputStream dos;

	dos=saver.open(nGausByMixture,nMixture,ext);
	for (int i=0; i< nMixture;i++) {
	    float []mix= (float []) mixtureWeights.get(i);
	    for (int p=0;p<mix.length;p++)
		mixture[p]=(float)(100.0*logMath.logToLinear(mix[p]));
	    saver.writeFloatArray(dos,mixture);
	}
	dos.close();
	dos=saver.open("means",nGaus,vectLen,nGausByMixture,ext);
	for (int i=0;i<nGaus;i++)
	    saver.writeFloatArray(dos, (float [])means.get(i));
	dos.close();
	Pool variances=loader.getVariancePool();
	dos=saver.open("variances",nGaus,vectLen,nGausByMixture,ext);
	for (int i=0;i<nGaus;i++)
	    saver.writeFloatArray(dos, (float [])variances.get(i));
	dos.close();
    }
}
