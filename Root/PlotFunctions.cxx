#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/PlotFunctions.h"
#include "PlotFunctions/MapBranches.h"

#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"
#include "TROOT.h"
#include "TProfile.h"

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

#define DEBUG 1
//=====================================================
void ChrisLib::PlotHist( const InputCompare &inputCompare, vector<vector<TH1*>> &vectHist ) {
  
  const vector<vector<string>> &inputObjName = inputCompare.GetObjName();
  const vector<vector<string>> &rootFilesName = inputCompare.GetRootFilesName();

  vector<TH1*> drawVect(rootFilesName.size(), 0 );
  
  for ( unsigned int iPlot = 0; iPlot < rootFilesName.size(); ++iPlot ) {
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
  
  vectHist = { drawVect };
  
}//end PlotHist


//=====================================================
void ChrisLib::PlotTree( const InputCompare &inputCompare, vector<vector<TH1*>> &vectHist,vector<vector<TGraphErrors*>> &vectGraph ) { 
  const vector<vector<string>> &inputObjName = inputCompare.GetObjName();
  const vector<vector<string>> &rootFilesName = inputCompare.GetRootFilesName();

  const vector< vector<string> > &varName = inputCompare.GetVarName();
  if ( varName.empty() || varName[0].empty() ) throw invalid_argument( "PlotTree : empty varName." );


  const vector< vector< double > > &xBinning = inputCompare.GetXBinning();
  const vector< double > &varMin  = inputCompare.GetVarMin();
  const vector< double > &varMax  = inputCompare.GetVarMax();
  if ( xBinning.empty() && varMin.empty() && varMax.empty() ) throw invalid_argument( "PlotTree : No information for binning provided" );

  const vector< vector<string> > &varWeight = inputCompare.GetVarWeight();
  const vector< vector<string> > &varErrX = inputCompare.GetVarErrX();
  const vector< vector<string> > &varErrY = inputCompare.GetVarErrY();
  const vector<string> &selectionCut = inputCompare.GetSelectionCut();
  const vector< string > &eventID = inputCompare.GetEventID();

  vectHist = vector<vector<TH1*>>( varName[0].size(), vector<TH1*>(rootFilesName.size(), 0) );
  vectGraph = vector<vector<TGraphErrors*>>( varName[0].size(), vector<TGraphErrors*>(rootFilesName.size(), 0) );
  unsigned nEvents = atoi(inputCompare.GetOption("nEvents").c_str());
  unsigned countEvent=1;

  vector< double > varVal( varName[0].size(), 0 );
  vector< long long > eventIDVal( eventID.size(), 0 );
  int nCols = rootFilesName.size()*varName[0].size();
  multi_array<double,2> varValues;
  multi_array<long long, 2> IDValues;

  if ( !eventID.empty() ) {
    if ( !nEvents ) throw invalid_argument( "PlotTree : Mandatory non null nEvents for event comparison" );
    varValues.resize( extents[nEvents][nCols] );
    IDValues.resize( extents[nEvents][nCols] );
  }

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
      if ( !eventID.empty() && iPlot ) nEvents = 0;//Read all second container for comparison at event level

      //create a vector to store all branches names to be linked
      list<string> linkedVariables;
      copy( varName[iPlot].begin(), varName[iPlot].end(), back_inserter(linkedVariables) );
      copy( varWeight[iPlot].begin(), varWeight[iPlot].end(), back_inserter(linkedVariables) );
      copy( varErrX[iPlot].begin(), varErrX[iPlot].end(), back_inserter(linkedVariables) );
      copy( varErrY[iPlot].begin(), varErrY[iPlot].end(), back_inserter(linkedVariables) );
      MapBranches mapBranch;
      mapBranch.LinkTreeBranches( inTree, 0, linkedVariables );
	
      cout << "linkTree OK " << endl;
      for ( int iEvent = 0; iEvent < nEntries; ++iEvent ) {
	if ( nEvents && countEvent==nEvents ) break;
	inTree->GetEntry( iEvent );
	cout << "iEvent" << endl;	
	double totWeight=1;
	for_each( varWeight[iPlot].begin(), varWeight[iPlot].end(), [&totWeight, &mapBranch]( const string &s ) { totWeight*=mapBranch.GetVal(s);} );
	cout << "weigth : " << totWeight << endl;
	int foundIndex=-1;
	if ( !eventID.empty() ) foundIndex = FillCompareEvent( inputCompare, IDValues, mapBranch, iPlot, iEvent );
	cout << "foundEvent : " << foundIndex << endl;
	int outMode = atoi(inputCompare.GetOption("inputType").c_str())-1;
	cout << "outMode : " << outMode << endl;
	for ( unsigned int iHist = 0; iHist < varName[iPlot].size(); iHist++ ) {

	  if ( !eventID.empty() && ( !iPlot || foundIndex != -1 ) ) varValues[foundIndex][iHist*rootFilesName.size()+iPlot] = mapBranch.GetVal( varName[iPlot][iHist] );

	  cout << "before init" << endl;
	  if ( outMode == 2 && !vectGraph[iHist][iPlot] ) vectGraph[iHist][iPlot] = static_cast<TGraphErrors*>( InitHist( inputCompare, outMode, iPlot, iHist ) );
	else if ( !vectHist[iHist][iPlot] ) vectHist[iHist][iPlot] = static_cast<TH1*>(InitHist( inputCompare, outMode, iPlot, iHist ));
	cout << "after init" << endl;
	if ( outMode < 2 && totWeight ) vectHist[iHist][iPlot]->Fill( mapBranch.GetVal(varName[iPlot][iHist] ) , totWeight );
	  else if ( outMode == 2 ) {
	    vectGraph[iHist][iPlot]->SetPoint( iEvent, mapBranch.GetVal(varName[iPlot][iHist]), totWeight );
	    double errX = varErrX.size()>iHist ?  mapBranch.GetVal(varErrX[iPlot][iHist]) : 0;
	    double errY = varErrY.size()>iHist ?  mapBranch.GetVal(varErrY[iPlot][iHist]) : 0;
	    vectGraph[iHist][iPlot]->SetPointError( iEvent, errX, errY );
	  }
	}// End iHist
	++countEvent;
      }//end iEvent
	
      delete inTree; 
      inFile.Close( "R" ); 
    }
  }//end iPlot

  if ( !eventID.empty() ) {
    string outName = inputCompare.GetOption( "plotDirectory" ) + inputCompare.GetOutName() + "_compareEvents";
    PrintOutputCompareEvents( varValues, IDValues, eventID, vectHist, outName );
  }

}


//====================================================================
void ChrisLib::PrintOutputCompareEvents( const multi_array<double,2> &varValues, const multi_array<long long,2> &IDValues, const vector<string> &eventID, const vector<vector<TH1*>> &vectHist, const string &outName ) {
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

//==================================================================
void ChrisLib::PlotTextFile( const InputCompare &inputCompare, vector<vector<TH1*>> &vectHist ) {

  const vector<vector<string>> inputObjName = inputCompare.GetObjName();
  const vector<vector<string>> rootFilesName = inputCompare.GetRootFilesName();
  if ( rootFilesName.empty() ) throw invalid_argument( "PlotTextFile : No input file." );
  const vector< vector<string> > varName = inputCompare.GetVarName();
  if ( varName.empty() || varName[0].empty() ) throw invalid_argument( "PlotTextFile : empty varName." );
  const vector< double > varMin = inputCompare.GetVarMin();
  const vector< double > varMax = inputCompare.GetVarMax();
  const vector< vector<string> > varWeight = inputCompare.GetVarWeight();
  int nBins = atoi( inputCompare.GetOption( "nComparedEvents" ).c_str() );
  if ( !nBins ) nBins=100;

  vectHist = vector<vector<TH1*>>( rootFilesName.size(), vector<TH1*>(varName[0].size(), 0 ) );;
  vector< unsigned > varIndex( varName[0].size(), 0 );

  for ( unsigned int iPlot = 0; iPlot < rootFilesName.size(); ++iPlot ) {
    for ( unsigned int iAdd = 0; iAdd < rootFilesName[iPlot].size(); ++iAdd ) {
      
      fstream inputStream;
      inputStream.open( rootFilesName[iPlot][iAdd].c_str(), fstream::in );
      string dumString;
      getline( inputStream, dumString );
      vector< string > titleVect;
      ParseVector(  dumString , titleVect );
      vector< double > varVal( titleVect.size(), 0 );

      for ( unsigned int iVar = 0; iVar < varName[iPlot].size(); ++iVar ) {
	varIndex[iVar] = SearchVectorBin( varName[iPlot][iVar], titleVect );
	if ( varIndex[iVar] == titleVect.size() ) throw invalid_argument( "PlotTestFile : varName not found in header : " + varName[iPlot][iVar] );

	vector<unsigned> weightIndices( varWeight[iPlot].size() );
	for ( unsigned int iWeight = 0; iWeight < varWeight[iPlot].size(); iWeight++ ) weightIndices[iWeight] = SearchVectorBin( varWeight[iPlot][iWeight], titleVect );

	while ( true ) {
	  for ( unsigned int iTxtVar = 0; iTxtVar < titleVect.size(); iTxtVar++ ) inputStream >> varVal[iTxtVar];
	  if ( inputStream.eof() ) break;

	  double weight = 1;
	  for_each( weightIndices.begin(), weightIndices.end(), [&weight, &varVal, &weightIndices]( const unsigned &i ) { weight*=varVal[weightIndices[i]];} );

	  for ( unsigned int iVar = 0; iVar < varName[iPlot].size(); ++iVar ) {
	    if ( vectHist.size() == iVar ) vectHist.push_back( vector<TH1*>() );
	    if ( vectHist[iVar].size() == iPlot ) {
	      string dumString = inputCompare.GetOutName() + string( TString::Format( "_%s_%d", varName[iPlot][iVar].c_str(), iPlot ));
	      vectHist[iVar].push_back(0);
	      vectHist[iVar].back() = new TH1D( dumString.c_str(), dumString.c_str(), nBins, varMin[iVar], varMax[iVar] );
	      vectHist[iVar].back()->SetDirectory(0);
	      vectHist[iVar].back()->GetXaxis()->SetTitle( varName[iPlot][iVar].c_str() );
	      vectHist[iVar].back()->GetYaxis()->SetTitle( "#events" );
	      vectHist[iVar].back()->Sumw2();
	    }
	      
	    vectHist[iVar][iPlot]->Fill( varVal[varIndex[iVar]], weight );
	  }//end for iVar
	}//end while
      }
    }
  }//end iPlot
}

//==============================================================
void ChrisLib::DrawVect( vector<vector<TH1*>> &vectHist, const InputCompare &inputCompare ) {

  const string plotPath = inputCompare.GetOption( "plotDirectory" ) + inputCompare.GetOutName();
  const int doTabular = atoi(inputCompare.GetOption("doTabular").c_str());
  const int saveRoot = atoi(inputCompare.GetOption( "saveRoot" ).c_str());
  const vector< vector<string> > varName = inputCompare.GetVarName();
  const vector<string> vectorOptions = inputCompare.CreateVectorOptions();
  for ( unsigned iPlot=0; iPlot<vectHist.size(); ++iPlot ) {
    string outPlotName = plotPath;
    if ( !varName.empty() ) outPlotName += "_" + varName[0][iPlot];
    DrawPlot( vectHist[iPlot], outPlotName, vectorOptions );
    if ( doTabular ) PrintHist( vectHist[iPlot], outPlotName, doTabular );
    if ( saveRoot ) WriteVectHist( vectHist[iPlot], outPlotName );
  }
}

//==================================================================
void ChrisLib::SplitTree( const InputCompare &inputCompare ) {

  const vector<vector<string>> inputObjName = inputCompare.GetObjName();
  const vector<vector<string>> rootFilesName = inputCompare.GetRootFilesName();
  const vector<string> selectionCut = inputCompare.GetSelectionCut();


  string plotDirectory = inputCompare.GetOption( "plotDirectory" );
  for ( unsigned int iPlot = 0; iPlot < rootFilesName.size(); ++iPlot ) {
    TTree *treeRejSel=0, *treePassSel=0;
    for ( unsigned int iAdd = 0; iAdd < rootFilesName[iPlot].size(); ++iAdd ) {
      
      for ( unsigned int iPass = 0; iPass < 2; iPass++ ) {
	TTree *selTree = iPass ? treeRejSel : treePassSel;
	    if ( !iAdd && selTree ) SaveTree( selTree, plotDirectory );

	    TFile inFile( rootFilesName[iPlot][iAdd].c_str() );	    
	    string treeName = ( inputObjName.size()>iPlot && inputObjName[iPlot].size()>iAdd ) ? inputObjName[iPlot][iAdd] : FindDefaultTree( &inFile );
	    TTree *inTree = (TTree*) inFile.Get( treeName.c_str() );
	    if ( !inTree ) throw invalid_argument( "SplitTree : Unknown Tree." );
	    inTree->SetDirectory(0);
	    
	    gROOT->cd();
	    string dumString = inFile.GetName();
	    treeName = StripString( dumString ) + "_" + inputCompare.GetOutName() + ( iPass ? "_RejSel" : "_PassSel" );	  
	    string selection = selectionCut.size()>iPlot ? selectionCut[iPlot] : "";
	    if ( selection == "" ) throw invalid_argument( "SplitTree : Selection is empty." );
	    if ( iPass ) selection = "!(" + selection + ")";
	    TTree *dumTree = inTree->CopyTree( selection.c_str() );
	    dumTree->SetDirectory(0);

	    if ( selTree ) {
	      AddTree( selTree, dumTree  );
	      delete dumTree; dumTree=0;
	    }
	    else {
	      dumTree->SetName( treeName.c_str() );
	      dumTree->SetTitle( treeName.c_str() );
	      iPass ? (treeRejSel = dumTree) : (treePassSel= dumTree) ;
	    }

	    delete inTree; inTree = 0;

	    if ( iAdd == rootFilesName.back().size()-1 && selTree ) SaveTree( selTree, plotDirectory );
	  }//end iPass
    }
    treePassSel->Print();
    treeRejSel->Print();
  }
}
//============================================================
TObject* ChrisLib::InitHist( const InputCompare &inputCompare, int outMode, int iPlot, int iHist ) {
  cout << "initHist" << endl;
  const vector< vector<string> > &varErrX = inputCompare.GetVarErrX();
  const vector< vector<string> > &varErrY = inputCompare.GetVarErrY();
  const vector<vector<string>> &rootFilesName = inputCompare.GetRootFilesName();
  const vector< vector< double > > &xBinning = inputCompare.GetXBinning();
  const vector< vector<string> > &varName = inputCompare.GetVarName();
  const vector< double > &varMin  = inputCompare.GetVarMin();
  const vector< double > &varMax  = inputCompare.GetVarMax();
  const vector< vector<string> > &varWeight = inputCompare.GetVarWeight();
  unsigned int nBins = atoi(inputCompare.GetOption("nBins").c_str());
  if ( !nBins ) nBins = 100;

  TH1* outHist=0;
  TGraphErrors *outGraph=0;
  stringstream name;
  name << StripString(rootFilesName[iPlot][0]) << "_"  <<  varName[iPlot][iHist] << "_" << iPlot;
  if ( outMode == 0 ) {
    if ( xBinning[iHist].empty() ) outHist = new TH1D( name.str().c_str(), name.str().c_str(), nBins, varMin[iHist], varMax[iHist] );
    else outHist = new TH1D( name.str().c_str(), name.str().c_str(), (int) xBinning[iHist].size()-1, &xBinning[iHist][0] );
  }
  else if ( outMode == 1 ) {
    if ( !xBinning.size() ) outHist = new TProfile( name.str().c_str(), name.str().c_str(), nBins, varMin[iHist], varMax[iHist] );
    else outHist = new TProfile( name.str().c_str(), name.str().c_str(), (int) xBinning[iPlot].size()-1, &xBinning[iPlot][0] );
  }
  else if (outMode == 2 ) {
    outGraph = new TGraphErrors(1);
    outGraph->SetName( TString::Format( "%s_%s_%s_%s_%d", varName[iPlot][iHist].c_str(), varWeight[iPlot][iHist].c_str(), varErrX[iPlot][iHist].c_str(), varErrY[iPlot][iHist].c_str(), iPlot ) );
  }
  cout << "created" << endl;
  if ( outHist ) {
    outHist->GetXaxis()->SetTitle( varName[iPlot][iHist].c_str() );
    outHist->GetYaxis()->SetTitle( outMode==0 ? "# Events" : varWeight[iPlot][iHist].c_str() );
    outHist->SetDirectory( 0 );
    outHist->Sumw2();
  }
  else {
    outGraph->GetXaxis()->SetTitle( varName[iPlot][iHist].c_str() );
    outGraph->GetYaxis()->SetTitle( outMode==0 ? "# Events" : varWeight[iPlot][iHist].c_str() );
  }
  cout << "endInit" << endl;
  return ( outGraph ? static_cast<TObject*>(outGraph) : static_cast<TObject*>(outHist) );
}
//=======================================================
int ChrisLib::FillCompareEvent( const InputCompare &inputCompare, multi_array<long long,2> &IDValues, const MapBranches &mapBranch, const int iPlot, const int iEvent ) {

  const vector< string > &eventID = inputCompare.GetEventID();
  
  int foundIndex=-1;
  if ( !iPlot ) {
    for ( unsigned i=0; i<eventID.size(); ++i ) IDValues[iEvent][i] = mapBranch.GetVal( eventID[i] );
    foundIndex=iEvent;
  }
  else {
    unsigned nBins = IDValues.size();
    for ( unsigned int iSavedEvent = 0; iSavedEvent < nBins; ++iSavedEvent ) {
      bool foundEvent=true;
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
  return foundIndex;  
}
