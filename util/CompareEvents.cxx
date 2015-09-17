#include <iostream>
#include <string>
#include "TFile.h"
#include "TTree.h"
#include <fstream>
#include <algorithm> 
#include <string>
#include "TH1D.h"
#include "TString.h"
#include "SideFunctions/DrawPlot.h"


using std::iostream;
using std::cout;
using std::vector;
using std::max;
using std::endl;
using std::string;
using std::fstream;

int main() {

  vector<string> inFileNames = {     
    "/sps/atlas/c/cgoudet/Calibration/DataxAOD/DRAW_Cut.root",
   "/sps/atlas/c/cgoudet/Calibration/DataxAOD/EGAM6_Cut.root"

  };
  vector<TFile*> inFiles;
  vector<TTree*> inTrees;
  vector<double> m12(inFileNames.size(),0);;
  vector<long long int > evtNumber(inFileNames.size(),0);
  vector<long long int > runNumber(inFileNames.size(),0);
  vector<double> energy(inFileNames.size(), 0);
  vector<double> e1(inFileNames.size(), 0);

  TH1D* histDiffMass = new TH1D( "histDiffMass", "histDiffMass", 100, -0.5, 0.5 );
  histDiffMass->GetXaxis()->SetTitle( "#Delta Mee (GeV)" );
  histDiffMass->GetYaxis()->SetTitle( "events /" + TString::Format("%2.0fMeV", (histDiffMass->GetXaxis()->GetXmax()-histDiffMass->GetXaxis()->GetXmin())/histDiffMass->GetNbinsX()*1000));

  TH1D* histDiffEnergy = new TH1D( "histDiffEnergy", "histDiffEnergy", 100, -0.1, 0.1 );
  histDiffEnergy->GetXaxis()->SetTitle( "#Delta E (GeV)" );
  histDiffEnergy->GetYaxis()->SetTitle( "events /" + TString::Format("%2.0fMeV", (histDiffEnergy->GetXaxis()->GetXmax()-histDiffEnergy->GetXaxis()->GetXmin())/histDiffEnergy->GetNbinsX()*1000));

  TH1D* histDiffE1 = new TH1D( "histDiffE1", "histDiffE1", 100, -0.01, 0.01 );
  histDiffE1->GetXaxis()->SetTitle( "#Delta E_{1} (GeV)" );
  histDiffE1->GetYaxis()->SetTitle( "events /" + TString::Format("%2.0fMeV", (histDiffE1->GetXaxis()->GetXmax()-histDiffE1->GetXaxis()->GetXmin())/histDiffE1->GetNbinsX()*1000));

  for ( unsigned int iFile = 0; iFile < inFileNames.size(); iFile++ ) {
    inFiles.push_back(0);
    inFiles.back() = new TFile( inFileNames[iFile].c_str() );
    cout << "inFile : " << inFiles.back() << endl;
    if ( inFiles.back() ) inFiles.back()->ls();
    string dumString = inFileNames[iFile];
    dumString = dumString.substr( dumString.find_last_of( "/" )+1);
    dumString = dumString.substr( 0, dumString.find_last_of( "." ) );
    dumString+="_selectionTree";
    cout << "dumString : " << dumString << endl;
    inTrees.push_back(0);
    inTrees.back() = (TTree*) inFiles.back()->Get( "Analysis_selectionTree" );
    cout << "inTree : " << inTrees.back() << endl;
    inTrees.back()->SetBranchAddress( "m12", &m12[iFile] );
    inTrees.back()->SetBranchAddress( "eventNumber", &evtNumber[iFile] );
    inTrees.back()->SetBranchAddress( "runNumber", &runNumber[iFile] );
    inTrees.back()->SetBranchAddress( "energy_1", &energy[iFile] );
    inTrees.back()->SetBranchAddress( "e1_1", &e1[iFile] );
  }

  std::fstream stream;
  stream.open( "Run368EventCompare2.csv", fstream::out | fstream::trunc );
  stream << "eventNumber,EGAM6_m12,DRAW_m12" << endl; 
  for( unsigned int iEvent = 0; iEvent < inTrees.front()->GetEntries(); iEvent++ ) {
    inTrees.front()->GetEntry(iEvent);
    //    stream << evtNumber.front() << "," << m12.front() << ",";

    for ( unsigned int iFile = 1; iFile < inFileNames.size(); iFile++ ) {
      bool found=false;
      for( unsigned int iSearch = 0; iSearch < inTrees[iFile]->GetEntries(); iSearch++ ) {
	inTrees[iFile]->GetEntry(iSearch);
	if ( evtNumber[iFile] != evtNumber.front() ) continue;
	found=true;
	break;
      }
      
      if ( !found ) {
	cout << "Not found events : " << evtNumber.front() << endl;
	continue; 
      }
      histDiffMass->Fill( m12.front()-m12[iFile] );
      histDiffEnergy->Fill( energy.front()-energy[iFile] );
      histDiffE1->Fill( e1.front()-e1[iFile] );
      stream << evtNumber.front() << "," << m12.front() << "," <<  m12[iFile] << endl;

      // if ( fabs(m12.front()-m12[iFile] ) > 0 ) cout << "Mass overflow : " << evtNumber.front() << " " << m12.front()-m12[iFile] << endl;
      // if ( fabs(energy.front()-energy[iFile] ) > histDiffEnergy->GetXaxis()->GetXmax() ) cout << "Energy overflow : " << evtNumber.front() << " " << energy.front()-energy[iFile] << endl;
      // if ( fabs(e1.front()-e1[iFile] ) > histDiffE1->GetXaxis()->GetXmax() ) cout << "E1 overflow : " << evtNumber.front() << " " << e1.front()-e1[iFile] << endl;
    }
    //    stream << endl;
  }

  DrawPlot( vector<TH1*>(1, histDiffMass), "plot/MassDifferenceDRAW", vector<string>(), 0, 0, 0);
  DrawPlot( vector<TH1*>(1, histDiffEnergy), "plot/EnergyDifferenceDRAW", vector<string>(), 0, 0, 0);
  DrawPlot( vector<TH1*>(1, histDiffE1), "plot/E1DifferenceDRAW", vector<string>(), 0, 0, 0);
  return 0;
}
