//-----------------------------------------------------------------------------------------------------
//  Consensus.cc : finds the consensus hypothesis and the confusion network corresponding to a lattice
//-----------------------------------------------------------------------------------------------------

// Copyright (c) 1999      Lidia Mangu  lidia@cs.jhu.edu  All rights reserved.
//--------------------------------------------------------------------------------------

#include <cstdlib>
#include <climits>
#include <cassert>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctype.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

#include "common_lattice.h"
#include "zio.h"
#include "Prob.h"
#include "GetOpt.h"
#include "LatticeInfo.h"
#include "Link.h"
#include "Lattice.h"
#include "Prons.h"
#include "Cluster.h"
#include "Clustering.h"

void Usage() {
    cout << "\nUSAGE:\n";
    cout << "consensus -i <infile> [-c <outfile>] [-C <outdir>] -R <pronfile> [-G <logfile>] [-S <scale>] [-L <LMweight>] [-l <PMweight>] " << endl 
         << " [-P <PMweight>] [-p <PMoldscale>] [-I <WIP>] [-D <DELweight>] [-T <thresh>] [-t <percentage>] [-m <intraword>] [-M <interword>]  " << endl
         << " [-e <lowEpsT>} [-E <highEpsT>] [-w <lowWordT>] [-W <highWordT>] [-f] [-o] [-s] [-b] [-n]" << endl << endl
	 << "where " << endl
	 << "-f               - specifies that the format of the input file is FSM (default SLF)" << endl               
	 << "-o               - don't use the time information available(if) in the file " << endl
	 << "-s               - don't use the phonetic similarity " << endl   
	 << "-i <inSLFfile>   - the input file (default stdin)" << endl
	 << "-c <outfile>     - the output file for the consensus hyp (default stdout)" << endl
	 << "-C <outdir>      - the output dir for the conf nets in FSM format (default don't output)" << endl   
	 << "-R <pronfile>    - the file containing the number of prons and the most likely pron for a word" << endl
	 << "-G <logfile>     - the log file showing the values of the parameters used to produce the output" << endl
	 << "-S <scale>       - the scale for the posterior distribution (default 1)" << endl
	 << "-L <LMweight>    - language model weight (default 1)" << endl
	 << "-l <LMoldscale>  - specify this value only if the LM scores are already scaled (we need to unscale them before using the new LM weight" << endl
	 << "-P <PMweight>    - pronunciation model weight (default 0)" << endl
	 << "-p <PMoldscale>  - specify this value only if the PM scores are already scaled"  << endl
	 << "-I <wdpenalty>   - word insertion penalty " << endl
	 << "-D <DELweight>   - the weight of the deletion (default 1)" << endl
	 << "-T <thresh>      - the threshold used for pruning low posterior links (default 0;  i.e no pruning)" << endl
	 << "-t <percetage>   - the percentage of links that are kept; this is an alternative pruning method" << endl
	 << "-m <intraword>   - the method used in the intraword clustering (max or avg; default max)" << endl
	 << "-M <interword>   - the method used in the interword clustering (max or avg; default max)" << endl
	 << "-e <lowEpsT>     - prune eps if its score is less than lowEpsT (default 0; i.e no pruning)" << endl
	 << "-E <highEpsT>    - prune all the other words in the bin if the score of deletion (eps) is greater than highEpsT (default 1; i.e no pruning)" << endl
	 << "-w <lowWordT>    - prune all the words with score less than lowWordT (default 0; i.e no pruning)" << endl	
	 << "-W <highWordT>   - prune all the other words if the highest scoring one has a score greater than highWordT (default 1; i.e no pruning)" << endl	
	 << "-b               - constrain the intra-word clustering step (default false)" << endl
	 << "-n               - constrain the inter-word clustering step (default false)" << endl
	 << endl;
}

float lmweight   = 0.0;
float prweight   = 0.0;
float wdpenalty  = 0.0;

float delweight  = 1.0;
float thresh     = INT_MAX;   // none of the links is pruned  
float scale      = 1.0;
float lmoldscale = 0.0;
float proldscale = 0.0;
float lowEpsT    = 0.0;
float highEpsT   = 1.0;
float lowWordT   = 0.0;
float highWordT  = 1.0;

string type             = "SLF";
string method_intraword = "max";
string method_interword = "max";
 
bool wdpenalty_was_set     = false;
bool lmscale_was_set       = false;
bool lmweight_was_set      = false;
bool prscale_was_set       = false;
bool prweight_was_set      = false;
bool percentage_pruning    = false;
bool score_pruning         = false; 

bool use_time_info         = true;
bool use_phon_info         = true;

bool constrain_intra       = false;
bool constrain_inter       = false;


int main(int argc, char * argv[]) {
    GetOpt opt(argc, argv, "fosnbi:c:C:R:S:G:L:l:P:p:I:E:e:W:w:D:T:t:m:M:");
    int ch;
    string infile(""), outfile(""), outdir(""); 
    string pronfile(""),logfile("");
    
    bool first = true;
    while((ch = opt()) != EOF || first){
	first = false;
	switch(ch) 
	    {
            case 'f':
                type = "FSM";
                break;
            case 'i':
                infile = opt.optarg;
                break;
            case 'c':
                outfile = opt.optarg;
                break;
            case 'C':	
		outdir = opt.optarg;
		break;
	    case 'R':
		pronfile = opt.optarg;
		break;
	    case 'G':
		logfile  = opt.optarg;
		break;
	    case 'S':
		scale = atof(opt.optarg);
		break;
	    case 'P':
		prweight = atof(opt.optarg);
		prweight_was_set = 1;
		break;
	    case 'p':
		proldscale = atof(opt.optarg);
		break;	  
	    case 'L':
		lmweight = atof(opt.optarg);
		lmweight_was_set = 1;
		break;
	    case 'l': 
		lmoldscale = atof(opt.optarg);
		break;
	    case 'I':
		wdpenalty = atof(opt.optarg);
		wdpenalty_was_set = 1;
		break;
	    case 'D':
		delweight = atof(opt.optarg);
		break;
	    case 'm':
		method_intraword = opt.optarg;
		break;
	    case 'M':
		method_interword = opt.optarg;
		break;
	    case 'e':
		lowEpsT = atof(opt.optarg);
		break;
	    case 'E':
		highEpsT = atof(opt.optarg);
		break; 
	    case 'w':
		lowWordT = atof(opt.optarg);
		break;
	    case 'W':
		highWordT = atof(opt.optarg);
		break; 
	    case 'T':
		thresh = atof(opt.optarg);
		score_pruning = true;
		break;
	    case 't':
		thresh = atof(opt.optarg);
		percentage_pruning = true;
		break;
	    case 'o':
		use_time_info = false;
		break;
            case 's':
		use_phon_info = false;
		break;
            case 'b':
		constrain_intra = true;
		break;	
	    case 'n':
		constrain_inter = true;
		break;
            default:
		Usage();
		exit(1);
		break;
	    }
    }

    if (percentage_pruning && score_pruning){
      cerr << "ERROR: contradictory pruning methods!" << endl;
      percentage_pruning = false;
    }
    
    Prons P(pronfile);
    ofstream flog;
    if (logfile != ""){
      flog.open(logfile.c_str());
      assert(flog);
    }

    if (outdir != "")
      mkdir(outdir.c_str(),0777);
    
    float lmscale_from_header;
    float prscale_from_header;
    float wdpenalty_from_header;
    float WIP = 0;
    float LMweight = 1;
    float PRweight = 1;
    
    ifstream flist(infile.c_str());
    assert(flist);
    
    first = true;  //we get all the info about the parameters from the first lattice
    
    string file_name;

    while(getlineH(flist, file_name)){
      // the files are compressed, but in the list of lattices they don't have the extension .gz
    //  if (file_name.find(".gz") == string::npos)
//	file_name += ".gz";
      
      
      // ///////////////////////////////////////////////////////////////////////////////////////
      //											//
      //                                READ THE LATTICE 					//
      //											//
      // ///////////////////////////////////////////////////////////////////////////////////////
      
      
      // read the lattice header for SLF lattices; count the number of links for FSM lattices	
      LatticeInfo info(file_name, type);
      Lattice lat(info, P);
      // ///////////////////////////////////////////////////////////////////////////////////////
      //    											//
      //  	Figure out the values of different parameters by combining the information      //
      //  	in the lattice header with the values of the arguments in the command line      //
      //  	Output these values in a log file.                                              //
      //  											//
      // ///////////////////////////////////////////////////////////////////////////////////////
      
      
      if (use_time_info == false)
	lat.set_no_time_info();
      
      if (lat.Type() == "SLF"){
	if (first){
	  lmscale_from_header   = info.LMscale();
	  prscale_from_header   = info.PRscale();
	  wdpenalty_from_header = info.Wdpenalty();
	  
	  if (wdpenalty_was_set)
	    WIP = wdpenalty;
	  else if (wdpenalty_from_header != 0)
	    WIP = wdpenalty_from_header;
	  
	  if (WIP < 0) WIP *= -1;
	  
	  if (lmweight_was_set)
	    LMweight = lmweight;	
	  else if (lmscale_from_header != 0)
	    LMweight = lmscale_from_header;
	  
	  // if the lm scores are already scaled, unscale them
	  if (lmoldscale > 0)
	    LMweight /= lmoldscale;
	  
	  if (prweight_was_set)
	    PRweight = prweight;
	  else if (prscale_from_header != 0)
	    PRweight = prscale_from_header;
	  
	  
	  // if the pron scores are already scaled, unscale them
	  if (proldscale > 0)
	    PRweight /= proldscale;
	  
	  if (logfile != ""){
	    flog << endl << "Formula used for computing the link probabilities is: ";
	    flog << "1/" << scale << "*( AC + " << LMweight << "*LM + " << PRweight << "*PR" << " - " << WIP << ")" << endl;
	    flog << "where AC, LM and PR are the unscaled acoustic, language and pronunciation model scores" << endl;
	    
	    if (lmoldscale > 1)
	      flog << "LM scores found in the lattice file were already weighted by " << lmoldscale << endl;
	    if (proldscale > 1)
	      flog << "PM scores found in the lattice file were already  weighted by " << proldscale << endl;
	  }
	}		
	
	// if PRweight is positive you need a pronunciation model; if the lattice doesn't contain any pronunciation scores
	// then put uniform pronunciation probababilities
	if (PRweight > 0){
	  unsigned has_unif_pron_prob = lat.put_uniform_pron_prob(P); 
	  if (logfile != "" && first){
	    flog << endl << "Pronunciation model: ";
	    if (has_unif_pron_prob) 
	      flog << "uniform\n";
	    else		
	      flog << "taken from the input file\n"; 
	    flog << "Pronunciations file: " <<  pronfile.c_str() << endl << endl;
	  }
	}	
	// compute the link scores as a combination of LM, PR, AC and WIP	
	lat.compute_link_scores(LMweight, PRweight, WIP, scale); 
      }
	
      if (wdpenalty != 0 && lat.Type() == "FSM"){
	if (logfile != "" && first) 
	  flog << endl << "The link scores were modified by WIP = " << wdpenalty << endl;
	
	if (scale < 0)
	  wdpenalty *= -1;
	lat.add_WIP(wdpenalty);      
      }       	
      
      if (prweight != 0 && lat.Type() == "FSM"){
	if (logfile != "" && first) 
	  flog << endl << "The pronunciation probs were added to the link scores; PR weight = " << prweight << endl;
	
	if (scale < 0) 
	  prweight *= -1;
	
	lat.add_prons(P, prweight); 
      }
      
      if (scale != 1 && lat.Type() == "FSM"){
	lat.scale_link_scores(scale);	
	
	if (logfile != "" && first)	
	  flog << endl << "The link scores are scaled by: " << scale << endl;
      }
      
      if (logfile != "" && first){
	if (percentage_pruning)
	  flog << "Keep " << thresh << "% links" << endl;
	else
	  flog << "Threshold: " << thresh << endl;  
	flog << endl;
	
	if (outdir != ""){
	  flog << "Thresholds used for pruning the bins in the confusion networks: ";
	  flog << "lowEpsT(" << lowEpsT << "), highEpsT(" << highEpsT << "), lowWordT(" << lowWordT << "), highWordT(" << highWordT << ")\n";
	}
	if (delweight != 1)
	  flog << "Weight of the deletion is:" << delweight << endl;
	flog << endl;
	
	flog << "Using time information : " << lat.has_time_info() << endl;
	flog << "Using phon similarity  : " << use_phon_info << endl << endl;
      }	
      
      
      // compute the Forward Backward scores
      lat.do_ForwardBackward();
	
      // if the lattice doesn't have time information, assign to each node a number representing the length of the
      // longest path from the lattice start to it

      if (lat.has_time_info() == false) 
	lat.put_max_dist(P);
      
      // prune links 
      if (percentage_pruning)
	lat.mark_pruned_percentage(thresh);
      else if (score_pruning)
	lat.mark_pruned_score(thresh);
      
      // ///////////////////////////////////////////////////////////////////////////////////////
      //											//
      //                              CLUSTER INITIALIZATION 					//
      //											//
      // ///////////////////////////////////////////////////////////////////////////////////////
      
      Clustering C(&lat, P);
      
      // ///////////////////////////////////////////////////////////////////////////////////////
      //											//
      //                              INTRA-WORD CLUSTERING 					//
      //											//
        // ///////////////////////////////////////////////////////////////////////////////////////
      
      
      assert(method_intraword == "max" || method_intraword == "avg");
      unsigned max_avg = 0;
      if (method_intraword == "avg")
	max_avg = 1;
      
      C.go_cluster(P,1,max_avg,constrain_intra);
      
      if (logfile != "" && first){
	flog << "Intra-word Clustering method: " << method_intraword;
	if (method_intraword == "max")
		flog << " (single link clustering method)" <<  endl;
	else if (method_intraword == "avg")
	  flog << " (complete link clustering method)" <<  endl;
	flog << "Constrain intra-word clustering: " << constrain_intra << endl << endl;
      }
      
      // compute the word posteriors from link posteriors in each class
      C.fill_words();
	
      
      // ///////////////////////////////////////////////////////////////////////////////////////
      //											//
      //                              INTER-WORD CLUSTERING 					//
      //											//
      // ///////////////////////////////////////////////////////////////////////////////////////	
      
      assert(method_interword == "max" || method_interword == "avg");
      unsigned phon = 3;
      if (use_phon_info)
	phon = 2;
      max_avg = 0;
      if (method_interword == "avg")
	max_avg = 1;
      
      C.go_cluster(P, phon, max_avg, constrain_inter);
      
      if (logfile != "" && first){
	flog << "Inter-word Clustering method: " << method_interword;
	if (method_interword == "max")
	  flog << " (single link clustering method)" <<  endl;
	else if (method_interword == "avg")
	  flog << " (complete link clustering method)" <<  endl;
	flog << "Constrain inter-word clustering: " << constrain_inter << endl << endl;
      }
      
      // if you constrained the merging steps in the inter-word clustering, you have to continue 
      // merging until you get a total order
      
      if (constrain_inter == true)
	C.go_cluster(P, 4, 1, false);	
      
      // //////////////////////////////////////////////////////////////////////////////////////
      
      // put the remaining probability mass on the deletion; weight it differently if necessary
      C.add_EPS(delweight);

      // sort the clusters in the total order in topological order 
      C.TopSort();                           
      
      // output the consensus hyp and the confusion networks
      C.print_sausages_FSMformat(outdir, outfile, file_name, lowEpsT, highEpsT, lowWordT, highWordT, P); 
      first = false;
    }
    flist.close();
    return 0;
}	
