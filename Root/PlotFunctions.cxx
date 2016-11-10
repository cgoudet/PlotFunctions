#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/PlotFunctions.h"
#include "PlotFunctions/MapBranches.h"

#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"

#include <iostream>
#include <stdexcept>
#include <iterator>
#include <list>
#include <algorithm>

using namespace ChrisLib;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::exception;
using std::runtime_error;
using std::back_inserter;
using std::list;
using std::invalid_argument;
using std::copy;
using std::for_each;

//=====================================================
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
    if ( drawVect.empty() ) throw runtime_error( "PlotHist : No histogram to draw." );

    string outName = inputCompare.GetOption("plotDirectory") + inputCompare.GetOutName();
    DrawPlot( drawVect, outName, inputCompare.CreateVectorOptions() );

    int doTabular = atoi(inputCompare.GetOption("doTabular").c_str());
    if ( doTabular ) PrintHist( drawVect, outName, doTabular );

    if ( atoi(inputCompare.GetOption( "saveRoot" ).c_str()) ) WriteVectHist( drawVect, outName );
  }//end try
  catch( const exception e ) {
    cout << e.what() << endl;
  }
  DeleteContainer( drawVect );  

}//end PlotHist


//=====================================================
void ChrisLib::PlotTree( const InputCompare &inputCompare ) {

  const vector<vector<string>> inputObjName = inputCompare.GetObjName();
  const vector<vector<string>> rootFilesName = inputCompare.GetRootFilesName();
  if ( rootFilesName.empty() ) throw invalid_argument( "PlotTree : No input file." );

  const vector< vector<string> > varName = inputCompare.GetVarName();
  if ( varName.empty() || varName[0].empty() ) throw invalid_argument( "PlotTree : empty varName." );



  const vector< double > varMin  = inputCompare.GetVarMin();
  const vector< double > varMax  = inputCompare.GetVarMax();
  const vector< vector<string> > varWeight = inputCompare.GetVarWeight();
  const vector< vector< double > > xBinning = inputCompare.GetXBinning();
  const vector<string> selectionCut = inputCompare.GetSelectionCut();

  vector<vector<TH1*>> vectHist( varName[0].size(), vector<TH1*>(rootFilesName.size(), 0) );

  vector< double > varVal( varName[0].size(), 0 );
  unsigned int nBins = atoi(inputCompare.GetOption("nComparedEvents").c_str());
  if ( !nBins ) nBins = 100;
	     
  try {   
    for ( unsigned int iPlot = 0; iPlot < rootFilesName.size(); ++iPlot ) {
      for ( unsigned int iAdd = 0; iAdd < rootFilesName[iPlot].size(); ++iAdd ) {

  	string inFileName = rootFilesName[iPlot][iAdd];
  	TFile inFile( inFileName.c_str() );

  	string inTreeName = ( inputObjName.size()>iPlot && inputObjName[iPlot].size()>iAdd ) ? inputObjName[iPlot][iAdd] : FindDefaultTree( &inFile, "TTree" );
  	TTree *inTree = static_cast<TTree*>(inFile.Get( inTreeName.c_str() ) );
  	if ( !inTree ) throw invalid_argument( "PlotTree : " + inTreeName + " dnot found in " + string(inFile.GetName()) );
  	inTree->SetDirectory(0);

  	if ( selectionCut.size()>iPlot && selectionCut[iPlot]!="" ) CopyTreeSelection( inTree, selectionCut[iPlot] );
	
  	int nEntries = inTree->GetEntries();

  	//create a vector to store all branches names to be linked
  	list<string> linkedVariables;
	copy( varName[iPlot].begin(), varName[iPlot].end(), back_inserter(linkedVariables) );
	copy( varWeight[iPlot].begin(), varWeight[iPlot].end(), back_inserter(linkedVariables) );
	MapBranches mapBranch;
  	mapBranch.LinkTreeBranches( inTree, 0, linkedVariables );
	
	for ( int iEvent = 0; iEvent < nEntries; ++iEvent ) {

	  inTree->GetEntry( iEvent );

	  double totWeight=1;
	  for_each( varWeight[iPlot].begin(), varWeight[iPlot].end(), [&totWeight, &mapBranch]( const string &s ) { totWeight*=mapBranch.GetVal(s);} );
	  
	  for ( unsigned int iHist = 0; iHist < varName[iPlot].size(); iHist++ ) {
	    if ( !vectHist[iHist][iPlot] ) {
	      string dumName = string( TString::Format( "%s_%s_%d", inputObjName[iPlot][iAdd].c_str(), varName[iPlot][iHist].c_str(), iPlot ) );
	      if ( xBinning[iHist].empty() ) vectHist[iHist][iPlot] = new TH1D( dumName.c_str(), dumName.c_str(), nBins, varMin[iHist], varMax[iHist] );
	      else vectHist[iHist][iPlot] = new TH1D( dumName.c_str(), dumName.c_str(), (int) xBinning[iHist].size()-1, &xBinning[iHist][0] );
	      vectHist[iHist][iPlot]->GetXaxis()->SetTitle( varName[iPlot][iHist].c_str() );
	      vectHist[iHist][iPlot]->GetYaxis()->SetTitle( "# Events" );
	      vectHist[iHist][iPlot]->SetDirectory( 0 );
	      vectHist[iHist][iPlot]->Sumw2();
	    }
		
	    if ( totWeight ) vectHist[iHist][iPlot]->Fill( mapBranch.GetVal(varName[iPlot][iHist] ) , totWeight );
  	    }// End iHist

	  }// end iEvent

	delete inTree; 
	inFile.Close( "R" ); 
      }
    }//end iPlot

    string plotPath = inputCompare.GetOption( "plotDirectory" ) + inputCompare.GetOutName();
    int doTabular = atoi(inputCompare.GetOption("doTabular").c_str());
    int saveRoot = atoi(inputCompare.GetOption( "saveRoot" ).c_str());
    const vector<string> vectorOptions = inputCompare.CreateVectorOptions();
    for ( unsigned iPlot=0; iPlot<varName[0].size(); ++iPlot ) {
      string outPlotName = plotPath + "_" + varName[0][iPlot];
      DrawPlot( vectHist[iPlot], outPlotName, vectorOptions );
      if ( doTabular ) PrintHist( vectHist[iPlot], outPlotName, doTabular );
      if ( saveRoot ) WriteVectHist( vectHist[iPlot], outPlotName );
    }
  }
  catch( const exception e ) {
    cout << e.what() << endl;
  }
  for ( auto it=vectHist.begin(); it!=vectHist.end(); ++it ) DeleteContainer( *it );  
}


//====================================================================
