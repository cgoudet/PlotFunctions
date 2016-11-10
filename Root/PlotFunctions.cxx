#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/SideFunctions.h"
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
#include <sstream>

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
using std::stringstream;
using boost::extents;
using boost::multi_array;
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
  const vector< string > &eventID = inputCompare.GetEventID();


  vector<vector<TH1*>> vectHist( varName[0].size(), vector<TH1*>(rootFilesName.size(), 0) );


  unsigned int nBins = atoi(inputCompare.GetOption("nComparedEvents").c_str());
  if ( !nBins ) nBins = 100;

  vector< double > varVal( varName[0].size(), 0 );
  vector< long long > eventIDVal( eventID.size(), 0 );
  int nCols = rootFilesName.size()*varName[0].size();
  multi_array<double,2> varValues;
  multi_array<long long, 2> IDValues;
  if ( !eventID.empty() ) {
    varValues.resize( extents[nBins][nCols] );
    IDValues.resize( extents[nBins][nCols] );
  }

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
	if ( !eventID.empty() && !iPlot ) nEntries = nBins;

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

	  int foundIndex=-1;
	  if ( !eventID.empty() && !iPlot ) {
	    for ( unsigned i=0; i<eventID.size(); ++i ) IDValues[iEvent][i] = mapBranch.GetVal( eventID[i] );
	    foundIndex=iEvent;
	  }
	  else if ( !eventID.empty() )  {
	    for ( unsigned int iSavedEvent = 0; iSavedEvent < nBins; ++iSavedEvent ) {
	      bool foundEvent =true;
	      for ( unsigned int iID = 0; iID < eventID.size(); ++iID ) {
		if ( IDValues[iSavedEvent][iID] == mapBranch.GetVal( eventID[iPlot] ) ) continue;
		foundEvent = false;
		break;
	      }
	      if ( !foundEvent ) continue;
	      foundIndex = iSavedEvent;
	      break;
	    }
	  }
	  
	  for ( unsigned int iHist = 0; iHist < varName[iPlot].size(); iHist++ ) {

	    if ( !eventID.empty() && ( !iPlot || foundIndex != -1 ) ) varValues[foundIndex][iHist*rootFilesName.size()+iPlot] = mapBranch.GetVal( varName[iPlot][iHist] );
	      
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
	}
	
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

    if ( !eventID.empty() ) {
      string outName = plotPath + "_compareEvents";
      vector<string> linesTitle(nBins, "" );
      vector<string> colsTitle(eventID.size()*rootFilesName.size()+1, "" );
      for ( unsigned iLine=0; iLine<nBins; ++iLine ) {
	stringstream ss;
	for ( unsigned iID = 0; iID<eventID.size(); ++iID ) {
	  ss << IDValues[iLine][iID] << " ";
	  if ( !iID ) colsTitle[0]+= eventID[iID] + " ";
	}
	linesTitle[iLine] = ss.str();
      }

      vector<unsigned> levelsSizes;
      levelsSizes.push_back(rootFilesName.size());
      levelsSizes.push_back(eventID.size());
      for ( unsigned iCol=1; iCol<colsTitle.size(); ++iCol ) {
	//varValues[foundIndex][iHist*rootFilesName.size()+iPlot] = mapBranch.GetVal( varName[iPlot][iHist] );	
	vector<unsigned> coords;
	GetCoordFromLinear( levelsSizes, iCol-1, coords );
	colsTitle[iCol] = vectHist[coords[0]][coords[1]]->GetTitle();
      }

      PrintArray( outName, varValues, linesTitle, colsTitle );
    }
  }
  catch( const exception e ) {
    cout << e.what() << endl;
  }
  for ( auto it=vectHist.begin(); it!=vectHist.end(); ++it ) DeleteContainer( *it );  
}


//====================================================================
void PrintOutputCompareEvents( const multi_array<double,2> &varValues, const multi_array<long long,2> &IDValues, const vector<string> &eventID, const vector<vector<TH1*>> &vectHist, const string &outName ) {
  if ( vectHist.empty() ) return;

  unsigned nBins = varValues.size();
  vector<string> linesTitle(nBins, "" );
  vector<string> colsTitle(eventID.size()*vectHist[0].size()+1, "" );
  for ( unsigned iLine=0; iLine<nBins; ++iLine ) {
    stringstream ss;
    for ( unsigned iID = 0; iID<eventID.size(); ++iID ) {
      ss << IDValues[iLine][iID] << " ";
      if ( !iID ) colsTitle[0]+= eventID[iID] + " ";
    }
    linesTitle[iLine] = ss.str();
  }
  
  vector<unsigned> levelsSizes;
  levelsSizes.push_back(vectHist[0].size());
  levelsSizes.push_back(eventID.size());
  for ( unsigned iCol=1; iCol<colsTitle.size(); ++iCol ) {
    //varValues[foundIndex][iHist*rootFilesName.size()+iPlot] = mapBranch.GetVal( varName[iPlot][iHist] );	
    vector<unsigned> coords;
    GetCoordFromLinear( levelsSizes, iCol-1, coords );
    colsTitle[iCol] = vectHist[coords[0]][coords[1]]->GetTitle();
  }
  
  PrintArray( outName, varValues, linesTitle, colsTitle );
}
