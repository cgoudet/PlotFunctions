#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include <vector>
#include "TH2F.h"
#include "TH1F.h"
#include "TString.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "PlotFunctions/DrawPlot.h"
#include <vector>
#include <string>
#include "boost/multi_array.hpp"
using boost::multi_array;
using boost::extents;
using std::string;
using std::vector;
using std::cout;
using std::endl;

void Style_Christophe();


int main( ) {
  Style_Christophe();

  unsigned int doRatio = 1;
  bool defaultTreeName = true;
  string outNamePrefix;

  string path = "/sps/atlas/c/cgoudet/Calibration/DataxAOD/";
  vector< vector< string > > filesNames;
  vector<string> legends;
  vector< double > legendPos;
  unsigned int whichplot = 0;
  unsigned int selection = 0;
  bool printMean = 1;
  vector< string > latexNames;
  multi_array< double, 2> latexPos;

  switch ( 1 ) {
  case 0 :
    outNamePrefix = "CompareZee13TeV_";
    filesNames =  { 
      { "MC_13TeV_Zee_smeared_0.root", "MC_13TeV_Zee_smeared_1.root" },     
      { "Data_13TeV_Zee_Lkh1_0.root"},
      { "Data_13TeV_Zee_Lkh1_scaled_0.root"},
      { "../Test/CorrectedData.root" }
    };
    legends = { "MC 13TeV, full correction", "Data 13TeV, no scale", "Data 13TeV, Tool corrected", "Data 13TeV, Tool + new scales" };
    whichplot = 1;
    break;

  case 1 :
    outNamePrefix = "CompareZee13TeVPeriod_";
    filesNames =  { 
      { "EGAM1_DATA15/Data_13TeV_PA_Zee_Lkh1_scaled.root"},
      { "EGAM1_DATA15/Data_13TeV_PC_Zee_Lkh1_scaled.root"}
    };
    legends = { "Period A", "Period C" };
    legendPos = {0.7, 0.8, 0.99, 0.99};
    whichplot = 1;
    selection=2;
    latexNames = { "1+ electron with 1.37<|#eta|<1.55" };
    latexPos.resize( extents[latexNames.size()][2] );
    latexPos[0][0] = 0.1; latexPos[0][1] = 0.9;

    break;

  }

  vector< string > varNames;
  vector< double > varMin;
  vector< double > varMax;
  vector<string> varWeightName;
  vector<double> meanVect;
  vector<double> sigmaVect;
  switch ( whichplot  ) {
  case 0 :
    varNames = {"pt_1", "pt_2", "eta_1", "eta_2", "phi_1", "phi_2", "eta_cl_1",  "eta_cl_2", "eta_calo_1", "eta_calo_2", "m12", "e_raw_sampl1_1", "e_raw_sampl1_2", "energy_1", "energy_2"};
    varMin = { 27000, 27000, -2.5, -2.5, -3.15, -3.15, -2.5, -2.5, -2.5, -2.5, 70, 1000, 1000, 25000, 25000};
    varMax = { 100000, 100000, 2.5, 2.5, 3.15, 3.15, 2.5, 2.5, 2.5, 2.5, 110, 50000, 50000, 100000, 100000};
    break;
  case 1 :
    varNames = { "m12" };
    varMin = { 80 };
    varMax = { 100 };
    break;
  }

  vector< double > varVal(varNames.size(), 0);;
  vector< vector< TH1D* >>  varHist;
  vector< TH1D* >  varRatio( varNames.size()-1, 0 );

  TFile* inFile = 0;
  TTree * inTree = 0;
  vector<string> names;
  unsigned int nentries;
  double eta1, eta2;
  //Loop over trees to compare
  for( unsigned int iFile = 0; iFile < filesNames.size(); iFile++ ) {
    //loop over trees to add
    meanVect.push_back(0);
    sigmaVect.push_back(0);
    unsigned int totEvents=0;
    for ( unsigned int iAddedFile = 0; iAddedFile < filesNames[iFile].size(); iAddedFile++ ) {

      //Put in names the name of the last TFile without path nor .root
      //To be used in the legends
      if ( iAddedFile )  names.back() =  filesNames[iFile][iAddedFile];
      else names.push_back( filesNames[iFile][iAddedFile] );
      names.back() = names.back().substr( 0, names.back().find_last_of( "." ) );
      names.back() = names.back().substr( names.back().find_last_of( "/" )+1 );

      //Get the trees
      inFile = new TFile( TString( path ) + filesNames[iFile][iAddedFile] );
      inTree = (TTree*) inFile->Get( defaultTreeName ? "Analysis_selectionTree" : TString( names.back() ) + "_selectionTree" );
      if ( !inTree ) inTree = (TTree*) inFile->Get( "correctedData" );
      //Create one histogram per variable and link tree to varables
      inTree->SetBranchStatus( "*", 0 );
      if ( !iAddedFile )  varHist.push_back( vector<TH1D*>( varNames.size(), 0 ) );
      double weight = 1;
      for ( unsigned int hist = 0; hist < varNames.size(); hist++ ) {
  	if ( !iAddedFile ) {
  	  varHist.back()[hist] = new TH1D(names[iFile] + TString("_") + TString(varNames[hist]), varNames[hist].c_str(), 40, varMin[hist], varMax[hist] );
  	  varHist.back()[hist]->SetDirectory(0);
  	  varHist.back()[hist]->Sumw2();
  	  varHist.back()[hist]->GetXaxis()->SetTitle( varNames[hist].c_str() );
  	  varHist.back()[hist]->GetYaxis()->SetTitle( "# Events" );
  	}
  	inTree->SetBranchStatus( varNames[hist].c_str(), 1);
  	inTree->SetBranchAddress( varNames[hist].c_str(), &varVal[hist]);
      }
      if ( iFile < varWeightName.size() ) {
  	inTree->SetBranchStatus( varWeightName[iFile].c_str(), 1 );
  	inTree->SetBranchAddress( varWeightName[iFile].c_str(), &weight );
      }

      
      inTree->SetBranchStatus( "eta_calo_1", 1 );
      inTree->SetBranchAddress( "eta_calo_1", &eta1 );
      inTree->SetBranchStatus( "eta_calo_2", 1 );
      inTree->SetBranchAddress( "eta_calo_2", &eta2 );

      //Loop over all entries in the tree      
      nentries = inTree->GetEntries();
      for ( unsigned int event = 0; event < nentries; event ++ ) {
  	inTree->GetEntry( event );
	switch ( selection ) {
	case 1 :
	  if ( varVal[10] > 99 || varVal[10] < 93 ) continue;
	  break;

	case 2 : 
	  if ( ( fabs( eta1 ) > 1.55 || fabs( eta1 ) < 1.37 ) && ( fabs( eta2 ) > 1.55 || fabs( eta2 ) < 1.37 ) ) continue;
	  break;
	default : 
	  break;
	}
	
  	if ( varVal[0] < varMax[0] && varVal[0] > varMin[0] ) {
  	totEvents++;
  	meanVect.back() += varVal[0];
	sigmaVect.back() += varVal[0]*varVal[0];
  	}
  	for ( unsigned int hist = 0; hist < varNames.size(); hist++ ){
  	  varHist.back()[hist]->Fill( varVal[hist], weight );
  	}
      }
      cout << "file " << filesNames[iFile][iAddedFile] << " dealt with" << endl;
      delete inTree; inTree = 0;
      inFile->Close();
      delete inFile; inFile = 0;
    }
      meanVect.back() /= totEvents;
      sigmaVect.back() = sqrt( sigmaVect.back()/totEvents -meanVect.back()*meanVect.back() );
      if ( printMean ) legends[iFile] += TString::Format( ", m=%2.3f, s=%2.3f", meanVect.back(), sigmaVect.back() ); 

  }

  cout << "files done" << endl;
  for ( unsigned int hist = 0; hist < varNames.size(); hist++ ){
    vector<TH1*> dumVect;
    for( unsigned int iFile = 0; iFile < filesNames.size(); iFile++ ) {
      cout << varHist[iFile][hist]->GetName() << " " << varHist[iFile][hist]->GetEntries()  << endl;
      dumVect.push_back( varHist[iFile][hist] );
    }
    cout << "drawing " << dumVect.size() << " histograms" << endl;
    DrawPlot( dumVect, 
	      "plot/" + outNamePrefix + varNames[hist], 
	      legends, 
	      doRatio, 
	      1, 
	      0,
	      0, 
	      vector<double>(),
	      legendPos
	      );
  }

  return 0;
}
