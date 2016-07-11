#include <RooRealVar.h>
#include <map>
#include "RooSimultaneous.h" 
#include <stdexcept>
#include <RooArgSet.h>
#include <RooMinimizer.h>
#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"
#include <Math/MinimizerOptions.h>
#include "TKey.h"
#include <vector>
#include <iomanip>
#include "PlotFunctions/RobustMinimize.h"
#include "math.h"
#include <boost/program_options.hpp>
#include "TIterator.h"
#include "RooCategory.h"
#include "TLatex.h"
#include "RooPlot.h"
#include "TCanvas.h"
#include "RooAbsData.h"
#include "RooDataSet.h"
#include <fstream>
#include "RooWorkspace.h"
#include <RooStats/ModelConfig.h>
using namespace RooStats;
#include "PlotFunctions/DrawPlot.h"

using std::fstream;
using std::cout;
using std::endl;
using std::vector;
using std::min;
using std::max;
using std::string;
using std::to_string;

using namespace RooFit;

namespace po = boost::program_options;


int  main(int argc, char *argv[]){
  //############################################################
  //Boost argument Menu
  //Check validity of Arguments
  po::options_description desc("LikelihoodProfile Usage");

  //define all options in the program
  //TString is not supported by boost::program_options
  //Have to use string
  //  double sigma, mean;
  int modif_scheme, strategy, numCores, silent, constraint;
  string data_type, snapshot, outfile;
  string infile;
  vector<string> var1, var2; 
  vector<string> var3;
  bool save_np, wsyst, justMin, saveCsv;
  string saveSnapshot; 

  desc.add_options()
    ( "help", "Display this help message")
    ( "infile", po::value<string>(&infile), "1 : Input file name")
    ( "outfile", po::value<string>(&outfile)->default_value(""), "Output file name")
    ( "var1", po::value<vector<string>>(&var1), "3-8 : Parameters for first variable")
    ( "var2", po::value<vector<string>>(&var2), "9-14 : Parameters for second variable")
    ( "profiled", po::value<vector<string>>(&var3), "Names of profiled parameters of interest")
    ( "data", po::value<string>(&data_type)->default_value("obsData_G"),"Data name")
    ( "save_np", po::value<bool>(&save_np)->default_value( false ), "Save nuisance parameters")
    ( "wsyst", po::value<bool>(&wsyst)->default_value( false ), "Do not profile on nuisance parameters")
    ( "scheme", po::value<int>(&modif_scheme)->default_value(0), "Modification scheme")
    ( "strategy", po::value<int>(&strategy)->default_value(0), "Minimization strategy")
    ( "verbose", po::value<int>(&silent)->default_value( 0 ), "Verbose mode for minimization")
    ( "snapshot",po::value<string>(&snapshot)->default_value("conditionalGlobs_muhat"), "Choose asimov snapshot")
    ( "numCores", po::value<int>(&numCores)->default_value(1), "Number of Cores used for minimization")
    ( "justMin", po::value<bool>(&justMin)->default_value(false), "")
    ( "saveCsv", po::value<bool>(&saveCsv)->default_value(false), "" )
    ( "saveSnapshot", po::value<string>(&saveSnapshot), "" )
    ( "constraint", po::value<int>( &constraint)->default_value(0), "" )
    ;

  //Define options gathered by position
  po::positional_options_description p;
  p.add( "infile", 1);
  p.add( "var1", 4);
  p.add( "var2", 4);


  // create a map vm that contains options and all arguments of options           
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).style(po::command_line_style::unix_style ^ po::command_line_style::allow_short).run(), vm);
  po::notify(vm);
  
  if(vm.count("help")) {cout << desc; return 0;}
  if (strategy>2 || strategy<0) {cout << "Wrong strategy number" << endl; return 2;}
  //########################################
  //Deals with the input name
  string prefix = outfile=="" ? infile.substr( 0, infile.find_last_of( "/" )+1 ) : outfile;
  //Check the coherence on the inputs regarding variables
  if ( !vm.count("var1") && var1.size()!=4 ) {
    cout << "Incomplete arguments for variable 1" << endl; 
    return 1;
  }
  if ( vm.count("var2") && var2.size()!=4 )  {
    cout << "Incomplete arguments for variable 2" << endl; 
    return 1;
  }

  //make a list of profiled variables
  vector<TString> variables;
  if (vm.count("profiled")) {
    for (unsigned int i=0; i<var3.size(); i++) {
      variables.push_back( var3[i] );
    }}
  
  //Parametrization of verbal mode of minimizer
  //ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Migrad");
  ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
  ROOT::Math::MinimizerOptions::SetDefaultStrategy(strategy);
  ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(0);
  if(silent<=1){    //vire les warnings
    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  }

  //Find a workspace into the root file and assign it
  cout << "Getting Workspace " << endl;
  TFile *file_input=0;
  if (vm.count("infile")) file_input = TFile::Open(infile.c_str(), vm.count("saveSnapshot") ? "update" : "" );
  else {
    cout << "No input file" << endl; 
    return 1;
  }
  RooWorkspace* combWS =0;
  TIter nextkey(file_input->GetListOfKeys());
  TKey *key=0;
  while ((key = (TKey*)nextkey())) {
    if (strcmp("RooWorkspace",key->GetClassName())) continue;
    cout << "workspace found" << endl;
    cout << file_input << endl;
    cout << key << endl;
    cout << key->GetName() << endl;
    file_input->ls();
    combWS=(RooWorkspace *) file_input->Get(key->GetName());
    cout << "got" << endl;
    cout << combWS << endl;
  }
  combWS->Print();

  //Test that the variables actually exist into the workspace
  if ( !combWS->var( var1[0].c_str() ) 
       || ( vm.count( "var2" ) && !combWS->var( var2[0].c_str() ) ) ) {
    cout << var1[0] << " : " << combWS->var( var1[0].c_str() ) << endl;
    cout << "Variable do not exist in workspace" << endl;
  }

  RooRealVar *mu1 = combWS->var( var1[0].c_str() );
  mu1->setConstant(0);
  RooRealVar *mu2 = ( vm.count( "var2" ) ) ? combWS->var( var2[0].c_str() ) : 0;
  if ( mu2 ) mu2->setConstant( 0 );
  vector<RooRealVar*> mu3;
  for ( unsigned int iVar = 0; iVar < variables.size(); iVar++ ) {
    if ( combWS->var( variables[iVar] ) )  {
      mu3.push_back((RooRealVar*) combWS->var(variables[iVar]));
      mu3.back()->setConstant(0);
    }
    else {
      cout << "Profiled variable " << variables[iVar] << " do not exists in workspace" << endl;
      return 2;
    }
  }



  //########################## END_MODIF ##################################

  //Gather the model config and poi
  ModelConfig* const mc = (ModelConfig*) combWS->obj("ModelConfig") ? (ModelConfig*) combWS->obj("ModelConfig") : (ModelConfig*) combWS->obj("mconfig");
  cout << "mc : " << mc << endl;
  const  RooArgSet *poi = (RooArgSet*) mc->GetParametersOfInterest();
  poi->Print("v");  
  TIterator* poi_itr = poi->createIterator();
  const  RooArgSet* np_ptr = mc->GetNuisanceParameters();
  //  np_ptr->Print("v");
  RooAbsData* data=combWS->data(data_type.c_str());
  if (!data) {
    cout << "wrong data name" << endl; 
    return 1;
  }
  cout << "data : " << data << endl;
  cout << "dataName : " << data->GetName() << endl;
  data->Print();

  //When dataset is asimov, we need to load a snapshot
  if (data_type.find("asimovData")!=string::npos) {
    cout << "load snapshot : " << snapshot << endl;
    if (!combWS->loadSnapshot(snapshot.c_str())) { 
      cout << "Error loading the snapshot" << endl; 
      return 6;}
    combWS->loadSnapshot( "conditionalNuis_muhat" );
  }

  // Get the likelihood of the workspace
  RooSimultaneous*  pdf = (RooSimultaneous*) mc->GetPdf();

  //change nll depending on gaussian constraint
  cout<<"Building NLL"<<endl; 
  RooAbsReal* nll = 0 ;
  if ( constraint == 1 )  nll = pdf->createNLL(*data, CloneData(false), Constrain( *pdf->getParameters( *data ))); 
  else if ( constraint == 2 ) nll = pdf->createNLL(*data, CloneData(false), Constrain( *mc->GetNuisanceParameters() )); 
  else if ( constraint == 3 ) nll = pdf->createNLL(*data, CloneData(false), Constrain(*combWS->var("nui_QCDscale_bbH"))); 
  else if ( constraint == 4 ) nll = pdf->createNLL(*data, CloneData(false), Constrained()); 
  else nll = pdf->createNLL(*data, CloneData(false) ); 

  nll->enableOffsetting( true );
  RooMinimizer *_minuit = new  RooMinimizer(*nll);
  cout << "nll built" << endl;
  cout << var1.size() << " " << var1[0] << " " << var1[1] << endl;
  //Create the limits of the fits
  unsigned int nXBins = ( justMin ) ? 1 : (unsigned int) strtod( var1[3].c_str(), 0 );
  double xMin = (double) strtod( var1[1].c_str(), 0 );
  double xMax = (double) strtod( var1[2].c_str(), 0 );
  double yMin = ( vm.count( "var2" ) ) ? (double) strtod( var2[1].c_str(), 0 ) : 0;
  double yMax = ( vm.count( "var2" ) ) ? (double) strtod( var2[2].c_str(), 0 ) : 0;
  unsigned int nYBins = ( !vm.count( "var2" ) || justMin ) ? 1 : (unsigned int) strtod( var2[3].c_str(), 0 );

  for ( unsigned int iVar1 = 0; iVar1 < nXBins; iVar1++ ) {
    for ( unsigned int iVar2 = 0; iVar2 < nYBins; iVar2++ ) {

      std::map<std::string, double> muMap;
      std::map<std::string, int> muConstMap;
      std::map<std::string, double> muErrorMap;
      
      // Deals with the np
      //Make a tree for np and link it to the map
      cout << "np tree" << endl;
      TTree *t_np_af=new TTree("np","np");

      RooRealVar* np_var;
      TIterator* np_itr = np_ptr->createIterator();
      while ((np_var = (RooRealVar*)np_itr->Next())) {  
	np_var->setConstant( wsyst );

	if(save_np) {
	  //Fill the map
	  muMap[np_var->GetName()] = np_var->getVal(); 
	  muErrorMap[std::string(np_var->GetName())+"_Error"] = np_var->getError();
	  muConstMap[std::string(np_var->GetName())+"_isConst"] = np_var->isConstant() ? 1 : 0;
	  //Create the Branch
	  t_np_af->Branch( np_var->GetName(), &(muMap[np_var->GetName()]));
	  t_np_af->Branch( (std::string(np_var->GetName())+"_isConst").c_str(), &(muConstMap[std::string(np_var->GetName())+"_isConst"]));
	  t_np_af->Branch( (std::string(np_var->GetName())+"_Error").c_str(), &(muErrorMap[std::string(np_var->GetName())+"_Error"]));
	}}
      
      // combWS->var( "nui_PER_ATLAS_Hgg_mass" )->setVal(0);
      // combWS->var( "nui_PER_ATLAS_Hgg_mass" )->setConstant(1);
      //      combWS->var( "slope_VH_dileptons_13TeV")->setVal(-0.01);      
      //      combWS->var( "slope_VH_dileptons_13TeV")->setConstant(1);
      
	    // In Stefan Workspace, some variables need to be put constant to 1 for the fit to give
      // the same results as in the note
      // Still unexplained
      // Treatment temporary
      if (combWS->var("dummy")) {
	combWS->var("mu_ggH_llll1112")->setVal(1);
	combWS->var("mu_ZH_llll1112")->setVal(1);
	combWS->var("mu_WH_llll1112")->setVal(1);
	combWS->var("mu_VBF_llll1112")->setVal(1);
	combWS->var("dummy")->setVal(1);

	combWS->var("mu_ggH_llll1112")->setConstant(1);
	combWS->var("mu_ZH_llll1112")->setConstant(1);
	combWS->var("mu_WH_llll1112")->setConstant(1);
	combWS->var("mu_VBF_llll1112")->setConstant(1);
	combWS->var("dummy")->setConstant(1);
      }


      //############################ MODIF WS ######################################
      cout << "modification" << endl;
      vector<RooRealVar*> tmpvar;
      cout << "Modif scheme : " << modif_scheme << endl;
      switch (modif_scheme % 100) {
      case 1 : {
	RooRealVar *dumVar = combWS->var( "nui_pdf_gg_bbH" );
	if ( !dumVar ) { cout << "scheme failed" << endl; exit(0); }
	dumVar->setVal(0);
	dumVar->setConstant(1);
	break;
      }
      case 2 : {
	RooRealVar *dumVar = combWS->var( "nui_QCDscale_bbH" );
	if ( !dumVar ) { cout << "scheme failed" << endl; exit(0); }
	dumVar->setVal(0);
	dumVar->setConstant(1);
	break;
      }
      case 3 : {
	RooRealVar *dumVar1 = combWS->var( "nui_pdf_gg_bbH" );
	RooRealVar *dumVar2 = combWS->var( "nui_QCDscale_bbH" );
	if ( !dumVar1 || !dumVar2 ) { cout << "scheme failed" << endl; exit(0); }
	dumVar1->setVal(0);
	dumVar1->setConstant(1);
	dumVar2->setVal(0);
	dumVar2->setConstant(1);
	break;
      }
      case 4 : {
	TIterator* iter = pdf->getParameters( *data )->createIterator();
	RooAbsPdf* parg;
	cout << "importing constraint" << endl;
	while ( (parg=(RooAbsPdf*)iter->Next()) ) {
	  TString name = parg->GetName();
	  if ( ( !name.Contains( "nui_" ) && !name.Contains( "mu_XS_" ) ) || name.Contains("glob_") ) continue;
	  combWS->var( name )->setVal(0);
	  combWS->var( name )->setConstant(1);

	}
	combWS->var("mu_XS_bbH")->setVal(1);
	combWS->var("mu_XS_tHjb")->setVal(1);
	combWS->var("mu_XS_tWH")->setVal(1);
	//	combWS->var("mu_XS_ttH")->setVal(2);
	break;
      }
      case 5 : {
	combWS->var("m_yy")->setRange(105, 160);
      }
      default : break;
      }
      
      switch ( modif_scheme / 100 ) {
      default : break;
      }

      cout << "end modification" << endl;

      //  Tree for the poi with one branch per value of poi, error, and constant status
      cout << "poi tree" << endl;
      double NLL_value = -99.99, C2H = -99;
      int status = -99, Npoint = -99;
      int idata = (data_type=="combData") ? 1 : 0;
      TTree *t = new TTree("nll","nll");
      t->Branch("Npoint", &Npoint, "Npoint/I");
      t->Branch("Data", &idata, "Data/I");
      t->Branch("status",&status,"status/I");
      t->Branch("NLL",&NLL_value);
      if ( combWS->function("C2H") ) t->Branch( "C2H", &C2H, "C2H/D" );
      
      poi_itr->Reset();
      for ( RooRealVar* v = (RooRealVar* ) poi_itr->Next(); v!=0; v = (RooRealVar*)poi_itr->Next() ) {
	v->setConstant(1);
	//Fill the map 
	muMap[v->GetName()] = v->getVal();
	muErrorMap[std::string(v->GetName())+"_Error"] = v->getError();
	muConstMap[std::string(v->GetName())+"_isConst"] = v->isConstant() ? 1 : 0;
	//Create the Branch
	t->Branch( v->GetName(), &(muMap[v->GetName()]));
	t->Branch( (std::string(v->GetName())+"_isConst").c_str(), &(muConstMap[std::string(v->GetName())+"_isConst"]));
	t->Branch( (std::string(v->GetName())+"_Error").c_str(), &(muErrorMap[std::string(v->GetName())+"_Error"]));
      }

      cout << "poi values" << endl;
      //Set the constantness of poi and values
      double x = ( justMin ) ? mu1->getVal() : ( nXBins == 1 ) ? xMin : xMin+(xMax-xMin)*iVar1/(nXBins-1);
      double y = 0;
      mu1->setConstant( ( justMin ) ? 0 : 1 );
      mu1->setVal( x );
      if ( vm.count( "var2" ) ) {
	y = ( justMin ) ? mu2->getVal() : ( nYBins == 1 ) ? yMin : yMin+(yMax-yMin)*iVar2/(nYBins-1);
	mu2->setConstant( ( justMin ) ? 0 : 1 );
	mu2->setVal( y );
      }
      for ( unsigned int iVar = 0; iVar < mu3.size(); iVar++ ) {
	mu3[iVar]->setConstant(0);
      }

      //Minimization procedure
      cout << " Minimize  : "  << endl;
      poi->Print("v");
      cout << mu1->GetName() << " " <<  iVar1+1 << "/" << nXBins << " " << mu1->getVal() << endl;
      if ( vm.count( "var2" ) )      cout << mu2->GetName() << " " <<  iVar2+1 << "/" << nYBins << " " << mu2->getVal() << endl;
      status = robustMinimize(*nll, *_minuit) ;
      cout << "second minimization" << endl;
      status = robustMinimize(*nll, *_minuit) ;
      cout << "third minimization" << endl;
      status = robustMinimize(*nll, *_minuit) ;
      cout << "End Minimize" << endl;
      poi->Print("v");      
      if (!std::isfinite(NLL_value)) continue;
      //      nll->Minos();    
      //Save the poi et the nll value
      NLL_value = nll->getVal();
      cout.precision(10);
      cout << "nll_value :  " << NLL_value << endl;

      if ( combWS->function( "C2H" ) ) C2H = combWS->function( "C2H" )->getVal();
      poi_itr->Reset();
      for ( RooRealVar* v = (RooRealVar*)poi_itr->Next(); v!=0; v = (RooRealVar*)poi_itr->Next() ) {
  	muMap[v->GetName()] = v->getVal();
  	muConstMap[std::string(v->GetName())+"_isConst"] = v->isConstant() ? 1 : 0;
  	muErrorMap[std::string(v->GetName())+"_Error"] = v->getError();
      }
      t->Fill();

      if( save_np ) {
  	np_itr->Reset();
  	while ((np_var = (RooRealVar*)np_itr->Next())) { 
  	  muMap[np_var->GetName()] = np_var->getVal();
  	  muConstMap[std::string(np_var->GetName())+"_isConst"] = np_var->isConstant() ? 1 : 0;
  	  muErrorMap[std::string(np_var->GetName())+"_Error"] = np_var->getError();
	}  	
	t_np_af->Fill();
      }

      cout << "saving tree " << t->GetEntries() << endl;
      //Save one root file per point 
      if (t->GetEntries()) {
	outfile = prefix + var1[0] + string( ( vm.count( "var2" ) ) ? var2[0] : "" ) + string("_")
	  +  ( modif_scheme / 100 == 1  ? TString::Format( "%d", (modif_scheme / 100) ) : "" )
	  + string( ( justMin ) ? "" :
		    "_" + TString::Format( "%d", (int) floor(x*1e4)) 
		    + string( ( vm.count( "var2" ) && !justMin ) ? 
			      "_" + TString::Format( "%d", (int) floor(y*1e4)) : "" ) )
	  + string(".root");
	
	
	TFile *file_output = new TFile( outfile.c_str(), "RECREATE" );
	t->Write("",TObject::kOverwrite);
	if (save_np) t_np_af->Write("",TObject::kOverwrite);
	file_output->Close();
	cout << "tree written : " << outfile << endl;
	

	if ( vm.count("saveSnapshot") ) {
	  RooArgSet saveParameters;
	  saveParameters.add( *mc->GetParametersOfInterest() );
	  saveParameters.add( *mc->GetNuisanceParameters() );
	  combWS->saveSnapshot( saveSnapshot.c_str(), saveParameters );
	  file_input->cd();
	  combWS->Write( "", TObject::kOverwrite );
	}

	if ( saveCsv ) {
	  fstream stream;
	  TString csvFile = outfile;
	  csvFile.ReplaceAll(".root", ".csv" );
	  stream.open( csvFile, fstream::out | fstream::trunc );
	  cout << "csvFile : " << csvFile << endl;

	  RooArgSet vars = *mc->GetParametersOfInterest();
	  vars.add( *mc->GetNuisanceParameters() );
	  TIterator* iter = vars.createIterator();
	  while ( RooRealVar* v = (RooRealVar* ) iter->Next() ) {//; v!=0; v = (RooRealVar*)iter->Next() ) {
	    stream << v->GetName() << " " << v->getVal() << " " << v->getError() << endl;
	  }
	  stream.close();

  //############################################
	  csvFile.ReplaceAll(".csv","" );
	  //PlotPerCategory( m_mapVar["invMass"],{ data, pdf }, (RooCategory*)&pdf->indexCat(), string(csvFile), { "legend=" + string(data->GetName()), "legend=" + string(pdf->GetName()), "nComparedEvents=50" } );
	  PlotPerCategory( { data, pdf }, (RooCategory*)&pdf->indexCat(), string(csvFile), { "legend=" + string(data->GetName()), "legend=" + string(pdf->GetName()), "nComparedEvents=50" } );
	  
  //##########################################
	 
	}//end csv
      }      

    }  }// End loop over points


  cout << "Went up to the end" << endl;
  return 0;
}
//#################################################################################################"



