#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/PlotFunctions.h"

#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"

#include <iostream>
#include <stdexcept>


using namespace ChrisLib;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::exception;
using std::runtime_error;

void ChrisLib::PlotHist( const InputCompare &inputCompare ) {
  vector<TH1*> drawVect;;

  const vector<vector<string>> inputObjName = inputCompare.GetObjName();
  const vector<vector<string>> rootFilesName = inputCompare.GetRootFilesName();
  try {   
    for ( unsigned int iPlot = 0; iPlot < rootFilesName.size(); ++iPlot ) {
      drawVect.push_back(0);
      for ( unsigned int iAdd = 0; iAdd < rootFilesName[iPlot].size(); ++iAdd ) {
	
	string inFileName = rootFilesName[iPlot][iAdd];
	TFile inFile( inFileName.c_str() );	
	
	string inHistName = inputObjName[iPlot][iAdd];
	TH1D* currHist = static_cast<TH1D*>(inFile.Get( inHistName.c_str() ) );
	if ( !currHist ) throw runtime_error( "PlotHist : Unknown histogram " + inputObjName[iPlot][iAdd] + " in " + inFile.GetName() );
	
	if ( !drawVect.back() ) {
	  drawVect.back() = currHist;
	  currHist->SetName( TString::Format( "%s_%d", inHistName.c_str(), iPlot ) );
	  currHist->SetDirectory( 0 );  
	}
	else drawVect.back()->Add( currHist );
	
      }
    }
    RemoveNullPointers( drawVect );
    if ( drawVect.empty() ) return;

    string outName = inputCompare.GetOption("plotDirectory") + inputCompare.GetOutName();
    DrawPlot( drawVect, outName, inputCompare.CreateVectorOptions() );

  }//end try
  catch( const exception e ) {
    cout << e.what() << endl;
  }
  DeleteContainer( drawVect );  

}//end PlotHist
