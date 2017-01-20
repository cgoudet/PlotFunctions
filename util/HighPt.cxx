#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/SideFunctionsTpp.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
using std::fstream;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using namespace ChrisLib;
int main() {

  // vector<string> MCFileNames, DataFileNames, MCObjNames, DataObjNames;
  // string MCFileLine = "/sps/atlas/c/cgoudet/Calibration/DataxAOD/MC_13TeV_Zee_25ns_Lkh1_scaled/MC_13TeV_Zee_25ns_Lkh1_scaled_0.root /sps/atlas/c/cgoudet/Calibration/DataxAOD/MC_13TeV_Zee_25ns_Lkh1_scaled/MC_13TeV_Zee_25ns_Lkh1_scaled_1.root"; 
  // string MCObjLine = "MC_13TeV_Zee_25ns_Lkh1_scaled_0_selectionTree MC_13TeV_Zee_25ns_Lkh1_scaled_1_selectionTree";
  // string DataFileLine = "/sps/atlas/c/cgoudet/Calibration/DataxAOD/Data_13TeV_Zee_25ns_Lkh1_scaled/Data_13TeV_Zee_25ns_Lkh1_scaled_0.root";
  // string DataObjLine = "Data_13TeV_Zee_25ns_Lkh1_scaled_0_selectionTree";
  // ParseVector( MCFileLine , MCFileNames );
  // ParseVector( MCObjLine, MCOjName );
  // ParseVector( DataFileLine , DataFileNames );
  // ParseVector( DataObjLine, DataOjName );

  vector<int> ptCuts = { 0, 100, 200, 300, 500 };
  vector<string> etaCut = { "centralBarrel", "forwardBarrel", "crack", "endcap" };
  vector<string> plotNames;

  string outFile = "/sps/atlas/c/cgoudet/Plots/HighPT_doc.tex";
  fstream stream;
  stream.open( outFile.c_str(), fstream::out | fstream::trunc );
  WriteLatexHeader( stream, "High PTZ" );


  stream << "\\section{Eta Study}" << endl;
  plotNames.clear();
  for ( unsigned int iCut = 0; iCut <= ptCuts.size(); iCut++ ) {
    for ( unsigned int iEta = 0; iEta <= etaCut.size(); iEta++ ) {
      string dumName = string( TString::Format( "/sps/atlas/c/cgoudet/Plots/ZMassPtCut_%spt_1%s_m12", iEta ? string( etaCut[iEta-1] + "_" ).c_str() : "" , !iCut ? "" : string( "_" + std::to_string(ptCuts[iCut-1])).c_str() ) );
      bool isLocal = system( ("ls " + dumName + ".pdf").c_str() );    
      plotNames.push_back( !isLocal ? dumName : ""  );
    }
  }
  WriteLatexMinipage( stream, plotNames, 2 );

  stream  << endl;
  stream << "\\section{Gain Study}" << endl;
  plotNames.clear();
  for ( unsigned int iCut = 0; iCut <= ptCuts.size(); iCut++ ) {
    for ( unsigned int iEta = 0; iEta <= etaCut.size(); iEta++ ) {
      string dumName = string( TString::Format( "/sps/atlas/c/cgoudet/Plots/ZMassPtCut_gain_%spt_1%s_m12", !iEta ? "" : string(etaCut[iEta-1] + "_").c_str(), !iCut ? "" : string( "_" + std::to_string(ptCuts[iCut-1])).c_str() ) );
      bool isLocal = system( ("ls " + dumName + ".pdf").c_str() );    
      plotNames.push_back( !isLocal ? dumName : "" );
    }
  }
  WriteLatexMinipage( stream, plotNames, 2 );
  stream  << endl;
  plotNames.clear();
  for ( unsigned int iCut = 0; iCut <= ptCuts.size(); iCut++ ) {
    for ( unsigned int iGain = 0; iGain < 3; iGain++ ) {
      for ( unsigned int iEta = 0; iEta <= etaCut.size(); iEta++ ) {
	string dumName = string( TString::Format( "/sps/atlas/c/cgoudet/Plots/ZMassPtCut_gain%d_%spt_1%s_m12", iGain, !iEta ? "" : string(etaCut[iEta-1] + "_").c_str(), !iCut ? "" : string( "_" + std::to_string(ptCuts[iCut-1])).c_str() ) );
	bool isLocal = system( ("ls " + dumName + ".pdf" ).c_str() );    
	plotNames.push_back( !isLocal ? dumName : "" );
      }
    }
  }
  WriteLatexMinipage( stream, plotNames, 3 );

  
  stream << "\\end{document}" << endl;

  string commandLine = "pdflatex  -interaction=batchmode " + outFile;
  //
  cout << "latexFileName : " << commandLine << endl;
  int err = system( commandLine.c_str() );
  err = system( commandLine.c_str() );
  err = system( commandLine.c_str() );
  cout << "IsDocument compiled : " << err << endl;


  return 0;
}
