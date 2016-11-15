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
#include <bitset>

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
using std::remove;
using std::unique;
using std::bitset;
using std::ifstream;
using std::ostream_iterator;

#define DEBUG 1
//=====================================================
void ChrisLib::PlotHist( const InputCompare &inputCompare, vector<vector<TH1*>> &vectHist ) {
  if ( DEBUG ) cout << "ChrisLib::PlotHist" << endl;  

  const vector<vector<string>> &inputObjName = inputCompare.GetObjName();
  const vector<vector<string>> &rootFilesName = inputCompare.GetRootFilesName();

  vector<TH1*> drawVect(rootFilesName.size(), 0 );
  cout << rootFilesName.size() << endl;  
  for ( unsigned int iPlot = 0; iPlot < rootFilesName.size(); ++iPlot ) {
    for ( unsigned int iAdd = 0; iAdd < rootFilesName[iPlot].size(); ++iAdd ) {
      cout << "indices : " << iPlot << " " << iAdd << endl;
      string inFileName = rootFilesName[iPlot][iAdd];
      TFile inFile( inFileName.c_str() );	
      if ( inputObjName.size() <= iPlot || inputObjName[iPlot].size()<=iAdd ) throw invalid_argument( "ChrisLib::PlotHist : Histograms names mandatory." );
      string inHistName = inputObjName[iPlot][iAdd];
      TH1D* currHist = static_cast<TH1D*>(inFile.Get( inHistName.c_str() ) );
      if ( !currHist ) throw runtime_error( "PlotHist : Unknown histogram " + inputObjName[iPlot][iAdd] + " in " + inFile.GetName() );
      if ( !drawVect[iPlot] ) {
	drawVect[iPlot] = currHist;
	currHist->SetName( TString::Format( "%s_%d", inHistName.c_str(), iPlot ) );
	currHist->SetDirectory( 0 );  
      }
      else drawVect[iPlot]->Add( currHist );
      
    }
  }
  
  vectHist = { drawVect };
  if ( DEBUG ) cout << "ChrisLib::PlotHist end" << endl;    
}//end PlotHist

//====================================================================
void ChrisLib::PrintOutputCompareEvents( const multi_array<double,2> &varValues, const multi_array<long long,2> &IDValues, const vector<string> &eventID, const vector<vector<TObject*>> &vectHist, const string &outName ) {
  if ( vectHist.empty() ) return;

  unsigned nBins = varValues.size();
  vector<string> linesTitle(nBins, "" );
  vector<string> colsTitle(eventID.size()*vectHist[0].size()+1, "" );
  cout << "colsTitle : " << eventID.size() << "*" << vectHist[0].size() << "+1=" << colsTitle.size() << endl;
  for ( unsigned iLine=0; iLine<nBins; ++iLine ) {
    stringstream ss;
    for ( unsigned iID = 0; iID<eventID.size(); ++iID ) {
      ss << IDValues[iLine][iID] << " ";
      if ( !iID ) colsTitle[0]+= eventID[iID] + " ";
    }
    linesTitle[iLine] = ss.str();
  }
  cout << "line" << endl;  
  vector<unsigned> levelsSizes;
  levelsSizes.push_back(vectHist[0].size());
  levelsSizes.push_back(vectHist.size());
  for ( unsigned iCol=1; iCol<colsTitle.size(); ++iCol ) {
    if ( iCol < eventID.size()+1 ) colsTitle[iCol] = eventID[iCol-1];
	 else {
    //varValues[foundIndex][iHist*rootFilesName.size()+iPlot] = mapBranch.GetVal( varName[iPlot][iHist] );	
    vector<unsigned> coords;
    cout << "iCol : " << iCol << " " << eventID.size() << endl;
    unsigned renormCol = iCol-eventID.size()-1;
    cout << "renormCol : " << renormCol << endl;
    GetCoordFromLinear( levelsSizes, iCol-1-eventID.size(), coords );
    cout << "FromLinear : " << iCol-1-eventID.size() << " " << coords[0] << " " << coords[1] << endl;
    cout << "coords : " << coords.size() << " " << vectHist.size() << " " << coords[0]<< endl;
    cout << vectHist[coords[0]].size() << " " << coords[1] << endl;
    cout << vectHist[coords[0]][coords[1]] << endl;
    colsTitle[iCol] = static_cast<TNamed*>(vectHist[coords[0]][coords[1]])->GetTitle();
	 }
  }
  cout << varValues[0].size() << " " << linesTitle.size() << " " << colsTitle.size() << endl;
  PrintArray( outName, varValues, linesTitle, colsTitle );
}

//==================================================================
void ChrisLib::PlotTextFile( const InputCompare &inputCompare, vector<vector<TObject*>> &vectHist ) {
  TestInputs( inputCompare );

  const vector<vector<string>> inputObjName = inputCompare.GetObjName();
  const vector<vector<string>> rootFilesName = inputCompare.GetRootFilesName();
  const vector< vector<string> > varName = inputCompare.GetVarName();
  const vector< vector<string> > varYName = inputCompare.GetVarYName();
  const vector< double > varMin = inputCompare.GetVarMin();
  const vector< double > varMax = inputCompare.GetVarMax();
  const vector< vector<string> > varWeight = inputCompare.GetVarWeight();

  vectHist = vector<vector<TObject*>>( varName[0].size(), vector<TObject*>(rootFilesName.size(), 0) );
  unsigned nEvents = atoi(inputCompare.GetOption("nEvents").c_str());

  int nCols = rootFilesName.size()*varName[0].size();
  multi_array<double,2> varValues;
  multi_array<long long, 2> IDValues;

  const OutMode outMode = GetOutMode( inputCompare );
  if ( outMode==OutMode::histEvent ) {
    varValues.resize( extents[nEvents][nCols] );
    IDValues.resize( extents[nEvents][nCols] );
  }

  for ( unsigned int iPlot = 0; iPlot < rootFilesName.size(); ++iPlot ) {
    unsigned countEvent=1;
    for ( unsigned int iAdd = 0; iAdd < rootFilesName[iPlot].size(); ++iAdd ) {
      cout << "iPlot : " << iPlot << " " << iAdd << endl;
      string inFileName = rootFilesName[iPlot][iAdd];
      ifstream inputStream( inFileName );

      if ( outMode==OutMode::histEvent && iPlot ) nEvents = 0;//Read all second container for comparison at event level

      MapBranches mapBranch;
      mapBranch.LinkCSVFile( inputStream );
      list<string> keys;
      mapBranch.GetKeys( keys );
      cout << "keys" << endl;
      copy( keys.begin(), keys.end(), ostream_iterator<string>(cout,"\n"));
      exit(0); 
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
void ChrisLib::DrawVect( vector<vector<TGraphErrors*>> &vectGraph, const InputCompare &inputCompare ) {

  const string plotPath = inputCompare.GetOption( "plotDirectory" ) + inputCompare.GetOutName();
  const int doTabular = atoi(inputCompare.GetOption("doTabular").c_str());
  const int saveRoot = atoi(inputCompare.GetOption( "saveRoot" ).c_str());
  const vector< vector<string> > varName = inputCompare.GetVarName();
  const vector<string> vectorOptions = inputCompare.CreateVectorOptions();
  cout << doTabular << " " << saveRoot << endl;
  for ( unsigned iPlot=0; iPlot<vectGraph.size(); ++iPlot ) {
    string outPlotName = plotPath;
    if ( !varName.empty() ) outPlotName += "_" + varName[0][iPlot];
    DrawPlot( vectGraph[iPlot], outPlotName, vectorOptions );
    // if ( doTabular ) PrintGraph( vectGraph[iPlot], outPlotName, doTabular );
    // if ( saveRoot ) WriteVectGraph( vectGraph[iPlot], outPlotName );
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
TObject* ChrisLib::InitHist( const InputCompare &inputCompare, unsigned iPlot, unsigned iHist ) {
  if ( DEBUG ) cout << "ChrisLib::InitHist" << endl;
  const OutMode outMode = GetOutMode( inputCompare );

  const vector< vector<string> > &varErrX = inputCompare.GetVarErrX();
  const vector< vector<string> > &varErrY = inputCompare.GetVarErrY();
  const vector<vector<string>> &rootFilesName = inputCompare.GetRootFilesName();
  const vector< vector< double > > &xBinning = inputCompare.GetXBinning();
  const vector< vector<string> > &varName = inputCompare.GetVarName();
  const vector< double > &varMin  = inputCompare.GetVarMin();
  const vector< double > &varMax  = inputCompare.GetVarMax();
  const vector< vector<string> > &varYName = inputCompare.GetVarYName();
  unsigned int nBins = atoi(inputCompare.GetOption("nBins").c_str());
  if ( !nBins ) nBins = 100;

  TObject* object=0;
  stringstream name;
  name << StripString(rootFilesName[iPlot][0]) << "_"  <<  varName[iPlot][iHist] << "_" << iPlot;

  if (outMode == OutMode::graphErrors ) object = new TGraphErrors();
  else if ( outMode == OutMode::profile ) {
    name << "_" << varYName[iPlot][iHist];
    if ( xBinning.empty() || xBinning[iHist].empty() ) object = new TProfile( name.str().c_str(), name.str().c_str(), nBins, varMin[iHist], varMax[iHist] );
    else object = new TProfile( name.str().c_str(), name.str().c_str(), (int) xBinning[iPlot].size()-1, &xBinning[iPlot][0] );
  }
  else {
    if ( xBinning.empty() || xBinning[iHist].empty() ) object = new TH1D( name.str().c_str(), name.str().c_str(), nBins, varMin[iHist], varMax[iHist] );
    else object = new TH1D( name.str().c_str(), name.str().c_str(), (int) xBinning[iHist].size()-1, &xBinning[iHist][0] );
  }


  if ( DEBUG ) cout << "Object created" << endl;
  if ( outMode==OutMode::graphErrors ) {
    TGraphErrors *outGraph=static_cast<TGraphErrors*>(object);
    outGraph->GetXaxis()->SetTitle( varName[iPlot][iHist].c_str() );
    outGraph->GetYaxis()->SetTitle( varYName[iPlot][iHist].c_str() );
    name.flush();
    name << varName[iPlot][iHist] << "_" << varYName[iPlot][iHist];
    if ( varErrX.size()>iPlot && varErrX[iPlot].size()>iHist ) name << "_" << varErrX[iPlot][iHist];
    if ( varErrY.size()>iPlot && varErrY[iPlot].size()>iHist ) name << "_" << varErrY[iPlot][iHist];
    name << "_" << iPlot;
    outGraph->SetName( name.str().c_str() );
  }
  else {
    TH1* outHist=static_cast<TH1D*>(object);
    outHist->GetXaxis()->SetTitle( varName[iPlot][iHist].c_str() );
    outHist->GetYaxis()->SetTitle( outMode==OutMode::profile ?  varYName[iPlot][iHist].c_str() : "#Events" );
    outHist->SetDirectory( 0 );
    outHist->Sumw2();
  }

  if ( DEBUG ) cout << "ChrisLib::InitHist end" << endl;
  return object;
}

//=======================================================
int ChrisLib::FillCompareEvent( const InputCompare &inputCompare, multi_array<long long,2> &IDValues, const MapBranches &mapBranch, const int iPlot, const int iEvent ) {

  const vector< string > &eventID = inputCompare.GetEventID();
  
  int foundIndex=-1;
  if ( !iPlot ) {
    for ( unsigned i=0; i<eventID.size(); ++i ) IDValues[iEvent][i] = *static_cast<const long long*>(mapBranch.GetVal( eventID[i] ));
    foundIndex=iEvent;
  }
  else {
    unsigned nBins = IDValues.size();
    for ( unsigned int iSavedEvent = 0; iSavedEvent < nBins; ++iSavedEvent ) {
      bool foundEvent=true;
      for ( unsigned int iID = 0; iID < eventID.size(); ++iID ) {
	if ( IDValues[iSavedEvent][iID] == *static_cast<const long long*>(mapBranch.GetVal(eventID[iID]) ) ) continue;
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

//=======================================================
void ChrisLib::TestInputs( const InputCompare &inputCompare ) {
  if ( DEBUG ) cout << "ChrisLib::TestInputs" << endl;
  const OutMode outMode = GetOutMode( inputCompare );

  const vector<vector<string>> &rootFilesName = inputCompare.GetRootFilesName();
  const vector< string > &eventID = inputCompare.GetEventID();
  const vector< vector<string> > &varName = inputCompare.GetVarName();
  const vector< vector<string> > &varYName = inputCompare.GetVarYName();
  const vector< vector< double > > &xBinning = inputCompare.GetXBinning();
  const vector< double > &varMin  = inputCompare.GetVarMin();
  const vector< double > &varMax  = inputCompare.GetVarMax();

  if ( rootFilesName.empty() || rootFilesName[0].empty() ) throw invalid_argument( "TestInput : No input file." );


  if ( outMode==OutMode::none ) return;
  bitset<6> errors;
  if ( outMode==OutMode::hist || outMode==OutMode::histEvent ) errors.set(0);
  if ( outMode==OutMode::histEvent ) { errors.set(1); errors.set(5); }
  if ( outMode==OutMode::profile ) { errors.set(0); errors.set(2); errors.set(3);}
  if ( outMode==OutMode::graphErrors ) { errors.set(2); errors.set(3); }
  if ( outMode!=OutMode::none ) errors.set(4);

  if ( errors.test(0) && xBinning.empty() && varMin.empty() && varMax.empty() ) throw invalid_argument( "TestInput : No information for binning provided" );
  if ( errors.test(4) && (varName.empty() || varName[0].empty()) ) throw invalid_argument( "TestInput : Require varName." );
  if ( errors.test(1) && eventID.empty() ) throw invalid_argument( "TestInput : Require eventID." );
  if ( errors.test(2) && (varYName.empty()||varYName[0].empty()) ) throw invalid_argument( "TestInput : Require varYName." );
  if ( errors.test(3) && varYName[0].size() != varName[0].size() ) throw invalid_argument( "TestInput : varName and varYName must have same size." );
  if ( errors.test(5) && !atoi(inputCompare.GetOption("nEvents").c_str()) ) throw invalid_argument( "TestInput : Require non null nEvents." );

  if ( DEBUG ) cout << "ChrisLib::TestInputs end" << endl;
}
//=========================================================
void ChrisLib::SetTGraphsTitle( const InputCompare &inputCompare, const unsigned iPlot, vector<vector<TGraphErrors*>> &vectGraph ) {
  if ( DEBUG ) cout << "ChrisLib::SetTGraphsTitle" << endl;
  const vector< vector<string> > &varName = inputCompare.GetVarName();
  const vector< vector<string> > &varWeight = inputCompare.GetVarWeight();
  
  for ( unsigned int iHist = 0; iHist < varName[iPlot].size(); iHist++ ) {
    if ( !vectGraph[iHist][iPlot] ) continue;
    vectGraph[iHist][iPlot]->GetXaxis()->SetTitle( varName[iPlot][iHist].c_str() );
    string titleY = ( varWeight[iPlot].size() > iHist ? varWeight[iPlot][iHist] : varWeight[0][iHist] );
    vectGraph[iHist][iPlot]->GetYaxis()->SetTitle( titleY.c_str() );
  }
  if ( DEBUG ) cout << "ChrisLib::SetTGraphsTitle end" << endl;
}

//==================================================
void ChrisLib::FillObject( const InputCompare &inputCompare, 
		 const MapBranches &mapBranch, 
		 vector<vector<TObject*>> &vectObject,
		 multi_array<long long,2> &IDValues,
		 multi_array<double,2> &varValues,
		 const unsigned iPlot,
		 const unsigned iEntry
		 ) {

  const vector< vector<string> > &varName = inputCompare.GetVarName();
  const vector< vector<string> > &varYName = inputCompare.GetVarYName();
  const OutMode outMode = GetOutMode( inputCompare );
  const vector< vector<string> > &varErrX = inputCompare.GetVarErrX();
  const vector< vector<string> > &varErrY = inputCompare.GetVarErrY();
  const vector< vector<string> > &varWeight = inputCompare.GetVarWeight();

  double totWeight=1;
  if ( outMode!=OutMode::graphErrors ) for_each( varWeight[iPlot].begin(), varWeight[iPlot].end(), [&totWeight, &mapBranch]( const string &s ) { totWeight*=*static_cast<const double*>(mapBranch.GetVal(s));} );

  int foundIndex=-1;
  if ( outMode==OutMode::histEvent ) foundIndex = FillCompareEvent( inputCompare, IDValues, mapBranch, iPlot, iEntry );


  for ( unsigned int iHist = 0; iHist < varName[iPlot].size(); iHist++ ) {
    if ( outMode==OutMode::histEvent && ( !iPlot || foundIndex != -1 ) ) varValues[foundIndex][iHist*varName.size()+iPlot] = *static_cast<const double*>(mapBranch.GetVal( varName[iPlot][iHist] ));
    
    if ( !vectObject[iHist][iPlot] ) vectObject[iHist][iPlot]=InitHist( inputCompare, iPlot, iHist );
    if ( outMode == OutMode::graphErrors ) {
      TGraphErrors *graph = static_cast<TGraphErrors*>(vectObject[iHist][iPlot]);
    string xTitle = graph->GetXaxis()->GetTitle();
    string yTitle = graph->GetYaxis()->GetTitle();
    int nPoints = graph->GetN();
    graph->Set( nPoints+1);
    graph->SetPoint( nPoints, *static_cast<const double*>(mapBranch.GetVal(varName[iPlot][iHist])), *static_cast<const double*>(mapBranch.GetVal( varYName[iPlot][iHist] )) );
    double errX = varErrX.size()>iPlot && varErrY[iPlot].size()>iHist ?  *static_cast<const double*>(mapBranch.GetVal(varErrX[iPlot][iHist])) : 0;
    double errY = varErrY.size()>iPlot && varErrY[iPlot].size()>iHist ?  *static_cast<const double*>(mapBranch.GetVal(varErrY[iPlot][iHist])) : 0;
    graph->SetPointError( nPoints, errX, errY );
    graph->GetXaxis()->SetTitle(xTitle.c_str() );
    graph->GetYaxis()->SetTitle(yTitle.c_str());
    }
    else if ( outMode==OutMode::profile && totWeight ) static_cast<TProfile*>(vectObject[iHist][iPlot])->Fill( *static_cast<const double*>(mapBranch.GetVal(varName[iPlot][iHist] )), *static_cast<const double*>(mapBranch.GetVal(varYName[iPlot][iHist]) ) , totWeight );
    else if ( ( outMode==OutMode::hist && totWeight )
	      || ( outMode==OutMode::histEvent && foundIndex!=-1 ) 
	      ) static_cast<TH1D*>(vectObject[iHist][iPlot])->Fill( *static_cast<const double*>(mapBranch.GetVal(varName[iPlot][iHist] )) , totWeight );

  }// End iHist
}
//==================================================
void ChrisLib::PlotTree( const InputCompare &inputCompare, vector<vector<TObject*>> &vectHist ) {
  if ( DEBUG ) cout << "ChrisLib::PlotTree" << endl;
  TestInputs( inputCompare );

  const vector<vector<string>> &inputObjName = inputCompare.GetObjName();
  const vector<vector<string>> &rootFilesName = inputCompare.GetRootFilesName();
  const vector< vector<string> > &varName = inputCompare.GetVarName();
  const vector< vector<string> > &varYName = inputCompare.GetVarYName();
  const vector< string > &eventID = inputCompare.GetEventID();
  const vector< vector<string> > &varErrX = inputCompare.GetVarErrX();
  const vector< vector<string> > &varErrY = inputCompare.GetVarErrY();
  const vector< vector<string> > &varWeight = inputCompare.GetVarWeight();
  const vector<string> &selectionCut = inputCompare.GetSelectionCut();


  vectHist = vector<vector<TObject*>>( varName[0].size(), vector<TObject*>(rootFilesName.size(), 0) );
  unsigned nEvents = atoi(inputCompare.GetOption("nEvents").c_str());

  int nCols = rootFilesName.size()*varName[0].size();
  multi_array<double,2> varValues;
  multi_array<long long, 2> IDValues;

  const OutMode outMode = GetOutMode( inputCompare );
  if ( outMode==OutMode::histEvent ) {
    if ( !nEvents ) throw invalid_argument( "PlotTree : Mandatory non null nEvents for event comparison" );
    varValues.resize( extents[nEvents][nCols] );
    IDValues.resize( extents[nEvents][nCols] );
  }

  for ( unsigned int iPlot = 0; iPlot < rootFilesName.size(); ++iPlot ) {
    unsigned countEvent=1;

    for ( unsigned int iAdd = 0; iAdd < rootFilesName[iPlot].size(); ++iAdd ) {
      cout << "iPlot : " << iPlot << " " << iAdd << endl;
      string inFileName = rootFilesName[iPlot][iAdd];
      TFile inFile( inFileName.c_str() );

      string inTreeName = ( inputObjName.size()>iPlot && inputObjName[iPlot].size()>iAdd ) ? inputObjName[iPlot][iAdd] : FindDefaultTree( &inFile, "TTree" );
      TTree *inTree = static_cast<TTree*>(inFile.Get( inTreeName.c_str() ) );
      if ( !inTree ) throw invalid_argument( "PlotTree : " + inTreeName + " dnot found in " + string(inFile.GetName()) );
      inTree->SetDirectory(0);

      if ( selectionCut.size()>iPlot && selectionCut[iPlot]!="" ) CopyTreeSelection( inTree, selectionCut[iPlot] );
	
      int nEntries = inTree->GetEntries();
      if ( outMode==OutMode::histEvent && iPlot ) nEvents = 0;//Read all second container for comparison at event level

      //create a vector to store all branches names to be linked
      cout << "linking" << endl;
      list<string> linkedVariables;
      copy( varName[iPlot].begin(), varName[iPlot].end(), back_inserter(linkedVariables) );
      if ( !varYName.empty() ) copy( varYName[iPlot].begin(), varYName[iPlot].end(), back_inserter(linkedVariables) );
      copy( varErrX[iPlot].begin(), varErrX[iPlot].end(), back_inserter(linkedVariables) );
      copy( varErrY[iPlot].begin(), varErrY[iPlot].end(), back_inserter(linkedVariables) );
      copy( eventID.begin(), eventID.end(), back_inserter(linkedVariables) );
      copy( varWeight[iPlot].begin(), varWeight[iPlot].end(), back_inserter(linkedVariables) );
      copy( varWeight[0].begin(), varWeight[0].end(), back_inserter(linkedVariables) );
      linkedVariables.sort();
      linkedVariables.erase( unique(linkedVariables.begin(), linkedVariables.end() ), linkedVariables.end() );
      MapBranches mapBranch;
      mapBranch.LinkTreeBranches( inTree, 0, linkedVariables );
      cout << "loopTree" << endl;
      for ( int iEvent = 0; iEvent < nEntries; ++iEvent ) {
	if ( nEvents && countEvent==nEvents ) break;
	inTree->GetEntry( iEvent );
	FillObject( inputCompare, mapBranch, vectHist, IDValues, varValues, iPlot, iEvent );
	++countEvent;
      }//end iEvent
      
      delete inTree; 
      inFile.Close( "R" ); 
    }//end iAdd
    
  }//end iPlot

  if ( outMode==OutMode::histEvent ) {
    string outName = inputCompare.GetOption( "plotDirectory" ) + inputCompare.GetOutName() + "_compareEvents";
    PrintOutputCompareEvents( varValues, IDValues, eventID, vectHist, outName );
  }
  if ( DEBUG ) cout << "ChrisLib::PlotTree end" << endl;
}


//=======================================================
OutMode ChrisLib::GetOutMode( const InputCompare &inputCompare ) {

  int inputType = atoi(inputCompare.GetOption("inputType").c_str());
  if ( inputType==0 ) return OutMode::none;
  else if ( inputType < 5 ) return static_cast<OutMode>(inputType);
  else if ( inputType < 9 ) return static_cast<OutMode>(inputType-4);
  else if ( inputType==9 ) return OutMode::none;
  else throw runtime_error( "GetOutMode : OutMode not planned for inputType above 5" );
}

//=========================================
string ChrisLib::WriteOutMode ( const OutMode outMode ) {
  switch( outMode ) {
  case OutMode::none : return "none";
  case OutMode::hist : return "hist";
  case OutMode::histEvent : return "histEvent";
  case OutMode::profile : return "profile";
  case OutMode::graphErrors : return "graphErrors";
  }
  throw invalid_argument( "WriteOutMode : No OutMode corresponds to the input." );
}
