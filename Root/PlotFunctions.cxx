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
#include "TMatrixD.h"

#include <iostream>
#include <stdexcept>
#include <iterator>
#include <list>
#include <algorithm>
#include <sstream>
#include <bitset>
#include <iomanip>

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

#define DEBUG 0
//=====================================================
void ChrisLib::PlotHist( const InputCompare &inputCompare, vector<vector<TObject*>> &vectHist ) {
  if ( DEBUG ) cout << "ChrisLib::PlotHist" << endl;  

  const vector<vector<string>> &inputObjName = inputCompare.GetObjName();
  const vector<vector<string>> &rootFilesName = inputCompare.GetRootFilesName();

  vector<TObject*> drawVect(rootFilesName.size(), 0 );
  for ( unsigned int iPlot = 0; iPlot < rootFilesName.size(); ++iPlot ) {
    for ( unsigned int iAdd = 0; iAdd < rootFilesName[iPlot].size(); ++iAdd ) {
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
      else static_cast<TH1D*>(drawVect[iPlot])->Add( currHist );
      
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
  levelsSizes.push_back(vectHist.size());
  for ( unsigned iCol=1; iCol<colsTitle.size(); ++iCol ) {
    if ( iCol < eventID.size()+1 ) colsTitle[iCol] = eventID[iCol-1];
    else {
      //varValues[foundIndex][iHist*rootFilesName.size()+iPlot] = mapBranch.GetVal( varName[iPlot][iHist] );	
      vector<unsigned> coords;
      unsigned renormCol = iCol-eventID.size()-1;
      GetCoordFromLinear( levelsSizes, renormCol, coords );
      colsTitle[iCol] = static_cast<TNamed*>(vectHist[coords[0]][coords[1]])->GetTitle();
    }
  }
  PrintArray( outName, varValues, linesTitle, colsTitle );
}

//==============================================================
void ChrisLib::DrawVect( vector<vector<TObject*>> &vectObj, const InputCompare &inputCompare ) {
  if ( DEBUG ) cout << "ChrisLib::DrawVect" << endl;

  const string plotPath = inputCompare.GetOption( "plotDirectory" ) + inputCompare.GetOutName();
  const vector< vector<string> > varName = inputCompare.GetVarName();
  DrawOptions drawOpt = inputCompare.CreateDrawOptions();

  for ( unsigned iHist=0; iHist<vectObj.size(); ++iHist ) {
    string outPlotName = plotPath;
    if ( !varName.empty() && varName.size()>iHist ) outPlotName += "_" + varName[0][iHist];
    drawOpt.AddOption( "outName", outPlotName );
    drawOpt.Draw( vectObj[iHist] );

    const int doTabular = atoi(inputCompare.GetOption("doTabular").c_str());
    if ( doTabular ) PrintHist( vectObj[iHist], outPlotName, doTabular );
    const int saveRoot = atoi(inputCompare.GetOption( "saveRoot" ).c_str());
    if ( saveRoot ) WriteVect( vectObj[iHist], outPlotName );
  }
  if ( DEBUG ) cout << "ChrisLib::DrawVect end" << endl;
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
  const unsigned doLabels = atoi(inputCompare.GetOption("doLabels").c_str());
  unsigned int nBins = atoi(inputCompare.GetOption("nBins").c_str());
  if ( !nBins ) nBins = 100;

  TObject* object=0;
  stringstream name;
  name << StripString(rootFilesName[iPlot][0]) << "_"  <<  varName[iPlot][iHist] << "_" << iPlot;

  if (outMode == OutMode::graphErrors ) object = new TGraphErrors();
  else if ( outMode == OutMode::profile ) {
    name << "_" << varYName[iPlot][iHist];
    if ( doLabels ) object = new TProfile( name.str().c_str(), name.str().c_str(), 1, -0.5, 0.5 );
    else if ( xBinning.empty() || xBinning[iHist].empty() ) object = new TProfile( name.str().c_str(), name.str().c_str(), nBins, varMin[iHist], varMax[iHist] );
    else object = new TProfile( name.str().c_str(), name.str().c_str(), static_cast<int>(xBinning[iPlot].size())-1, &xBinning[iPlot][0] );
  }
  else {
    if ( doLabels ) object = new TH1D( name.str().c_str(), name.str().c_str(), 1, -0.5, 0.5 );
    else if ( xBinning.empty() || xBinning[iHist].empty() ) object = new TH1D( name.str().c_str(), name.str().c_str(), nBins, varMin[iHist], varMax[iHist] );
    else object = new TH1D( name.str().c_str(), name.str().c_str(), static_cast<int>(xBinning[iHist].size())-1, &xBinning[iHist][0] );
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
    for ( unsigned i=0; i<eventID.size(); ++i ) IDValues[iEvent][i] = mapBranch.GetLongLong( eventID[i] );
    foundIndex=iEvent;
  }
  else {
    unsigned nBins = IDValues.size();
    for ( unsigned int iSavedEvent = 0; iSavedEvent < nBins; ++iSavedEvent ) {
      bool foundEvent=true;
      for ( unsigned int iID = 0; iID < eventID.size(); ++iID ) {
	if ( IDValues[iSavedEvent][iID] == mapBranch.GetLongLong(eventID[iID]) ) continue;
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
  const unsigned doLabels = atoi(inputCompare.GetOption("doLabels").c_str());
  if ( rootFilesName.empty() || rootFilesName[0].empty() ) throw invalid_argument( "TestInput : No input file." );


  if ( outMode==OutMode::none ) return;
  bitset<6> errors;
  if ( outMode==OutMode::hist || outMode==OutMode::histEvent ) errors.set(0);
  if ( outMode==OutMode::histEvent ) { errors.set(1); errors.set(5); }
  if ( outMode==OutMode::profile ) { errors.set(0); errors.set(2); errors.set(3);}
  if ( outMode==OutMode::graphErrors ) { errors.set(2); errors.set(3); }
  if ( outMode!=OutMode::none ) errors.set(4);
  if ( doLabels && IsTH1( outMode ) ) errors.set(0,0);

  if ( errors.test(0) && xBinning.empty() && ( varMin.empty() || varMax.empty()) ) throw invalid_argument( "TestInput : No information for binning provided" );
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

  //  if ( DEBUG ) cout << "ChrisLib::FillObject" << endl;
  const vector< vector<string> > &varName = inputCompare.GetVarName();
  const vector< vector<string> > &varYName = inputCompare.GetVarYName();
  const OutMode outMode = GetOutMode( inputCompare );
  const vector< vector<string> > &varErrX = inputCompare.GetVarErrX();
  const vector< vector<string> > &varErrY = inputCompare.GetVarErrY();
  const vector< vector<string> > &varWeight = inputCompare.GetVarWeight();
  const unsigned doLabels = atoi(inputCompare.GetOption("doLabels").c_str());

  double totWeight=1;
  if ( outMode!=OutMode::graphErrors ) for_each( varWeight[iPlot].begin(), varWeight[iPlot].end(), [&totWeight, &mapBranch]( const string &s ) { totWeight*=stod(mapBranch.GetLabel(s));} );
  int foundIndex=-1;
  if ( outMode==OutMode::histEvent ) foundIndex = FillCompareEvent( inputCompare, IDValues, mapBranch, iPlot, iEntry );

  for ( unsigned int iHist = 0; iHist < varName[iPlot].size(); iHist++ ) {
    string label;
    if ( doLabels ) label = ReplaceString( "\\_", "_" )(mapBranch.GetLabel( varName[iPlot][iHist] ));
    if ( outMode==OutMode::histEvent && ( !iPlot || foundIndex != -1 ) ) varValues[foundIndex][iHist*varName.size()+iPlot] = stod(mapBranch.GetLabel( varName[iPlot][iHist] ));
    if ( !vectObject[iHist][iPlot] ) {
      vectObject[iHist][iPlot]=InitHist( inputCompare, iPlot, iHist );
      TH1 *hist = static_cast<TH1*>(vectObject[iHist][iPlot]);
      if ( doLabels && IsTH1(outMode) ) {
	hist->GetXaxis()->SetBinLabel(1, label.c_str());
	hist->GetXaxis()->LabelsOption("u");
	hist->GetXaxis()->SetLabelSize( 0.025 );
      }
    }

    double yVal = varYName.size() ? stod(mapBranch.GetLabel(varYName[iPlot][iHist])) : 0;
    double xVal = !doLabels ? stod(mapBranch.GetLabel(varName[iPlot][iHist] )) : 0;

    int iBin = -1;
    if ( doLabels && IsTH1( outMode ) ) {
      TH1* hist = static_cast< TH1* >(vectObject[iHist][iPlot]);
      iBin = hist->GetXaxis()->FindBin( label.c_str() );
      if ( iBin==-1 ) throw runtime_error( "FillObject : FindBin error." );
    }
    if ( outMode == OutMode::graphErrors ) {
      TGraphErrors *graph = static_cast<TGraphErrors*>(vectObject[iHist][iPlot]);
      string xTitle = graph->GetXaxis()->GetTitle();
      string yTitle = graph->GetYaxis()->GetTitle();
      int nPoints = graph->GetN();
      graph->Set( nPoints+1);
      graph->SetPoint( nPoints, xVal, yVal );
      double errX = varErrX.size()>iPlot && varErrY[iPlot].size()>iHist ?  mapBranch.GetDouble(varErrX[iPlot][iHist]) : 0;
      double errY = varErrY.size()>iPlot && varErrY[iPlot].size()>iHist ?  mapBranch.GetDouble(varErrY[iPlot][iHist]) : 0;
      graph->SetPointError( nPoints, errX, errY );
      graph->GetXaxis()->SetTitle(xTitle.c_str() );
      graph->GetYaxis()->SetTitle(yTitle.c_str());
    }
    else if ( outMode==OutMode::profile && totWeight ) {
      if ( doLabels ) static_cast<TProfile*>(vectObject[iHist][iPlot])->Fill( iBin, yVal , totWeight );
      else static_cast<TProfile*>(vectObject[iHist][iPlot])->Fill( xVal, yVal, totWeight );
    }
    else if ( ( outMode==OutMode::hist && totWeight )
	      || ( outMode==OutMode::histEvent && foundIndex!=-1 ) 
	      ) {
      if ( doLabels ) static_cast<TH1D*>(vectObject[iHist][iPlot])->Fill( iBin-1, totWeight );
      else static_cast<TH1D*>(vectObject[iHist][iPlot])->Fill( xVal , totWeight );
    }

  }// End iHist
//  if ( DEBUG ) cout << "ChrisLib::FillObject end" << endl;
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
    unsigned countEvent=0;

    for ( unsigned int iAdd = 0; iAdd < rootFilesName[iPlot].size(); ++iAdd ) {

      string inFileName = rootFilesName[iPlot][iAdd];
      cout << "iPlot : " << iPlot << " " << iAdd << " " << inFileName << endl;
      TTree *inTree=0;
      TFile *inFile=0;
      MapBranches mapBranch;
      ifstream inputStream;
      int nEntries=1;
      if ( outMode==OutMode::histEvent && iPlot ) nEvents = 0;//Read all second container for comparison at event level
      bool isRoot = inFileName.find(".root")!=string::npos;
      if ( isRoot ) {
	
	inFile = new TFile( inFileName.c_str() );
      if ( inFile->IsZombie() ) throw invalid_argument( "ChrisLib::PlotTree : Input file does not exist " + inFileName );
      string inTreeName = ( inputObjName.size()>iPlot && inputObjName[iPlot].size()>iAdd ) ? inputObjName[iPlot][iAdd] : FindDefaultTree( inFile, "TTree" );
      inTree = static_cast<TTree*>(inFile->Get( inTreeName.c_str() ) );
      if ( !inTree ) throw invalid_argument( "PlotTree : " + inTreeName + " not found in " + string(inFile->GetName()) );
      inTree->SetDirectory(0);
      if ( selectionCut.size()>iPlot && selectionCut[iPlot]!="" ) CopyTreeSelection( &inTree, selectionCut[iPlot] );
      nEntries = inTree->GetEntries();
      //create a vector to store all branches names to be linked
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
      mapBranch.LinkTreeBranches( inTree, 0, linkedVariables );
      }
      else { 
	inputStream.open( inFileName );
	if ( !inputStream.good() ) throw invalid_argument( "PlotTestFile : Wrong input file " + inFileName );
	if ( outMode==OutMode::histEvent && iPlot ) nEvents = 0;//Read all second container for comparison at event level
	mapBranch.LinkCSVFile( inputStream );
      }
      for ( int iEvent = 0; iEvent < nEntries; ++iEvent ) {
	if ( nEvents && countEvent==nEvents ) break;
	if ( isRoot ) inTree->GetEntry( iEvent );
	else {
	  mapBranch.ReadCSVEntry( inputStream );
	  if ( inputStream.eof() ) break;
	  ++nEntries;
	}
	FillObject( inputCompare, mapBranch, vectHist, IDValues, varValues, iPlot, iEvent );
	++countEvent;
      }//end iEvent

      if ( !isRoot ) inputStream.close();
      else {
	delete inTree; 
	inFile->Close( "R" );
	delete inFile;
      }
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
  else if ( inputType==10 ) return OutMode::none;
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

//=========================================
void ChrisLib::PlotMatrix( const InputCompare &inputCompare, vector<vector<TObject*>> &vectObj ) {
  
  const vector<vector<string>> &inputObjName = inputCompare.GetObjName();
  const vector<vector<string>> &rootFilesName = inputCompare.GetRootFilesName();
  if ( rootFilesName.empty() ) throw invalid_argument( "PlotMatrix : No input file." );
  if ( inputObjName.empty() ) throw invalid_argument( "PlotMatrix : matrix name required." );

  vector<TObject*> drawVect(rootFilesName.size(), 0 );

  for ( unsigned int iPlot = 0; iPlot < rootFilesName.size(); ++iPlot ) {
    if ( rootFilesName[iPlot].empty() ) throw invalid_argument( "PlotMatrix : No input file.");
    string mFile = rootFilesName[iPlot][0];
    TFile inFile( mFile.c_str() );
    if ( inputObjName[iPlot].empty() ) throw invalid_argument( "PlotMatrix : object name required." );
    string mObj = inputObjName[iPlot][0];
    TMatrixD *matrix = static_cast<TMatrixD*>(inFile.Get( mObj.c_str() ));
    if ( !matrix ) throw invalid_argument( "PlotMatrix : matrix not found." );

    unsigned int nLine = matrix->GetNrows();
    unsigned int nCol = matrix->GetNcols();

    TString histTitle;
    bool doTriangular = ( nCol==nLine && inputCompare.GetOption( "triangular" ) != "" && atoi( inputCompare.GetOption( "triangular" ).c_str() ) );
    unsigned int nBins = doTriangular ? nLine*(nLine+1)/2 : nLine*nCol;
    drawVect[iPlot] = new TH1D( histTitle, histTitle, nBins, 0.5, nBins+0.5 );
    TH1D* hist = static_cast<TH1D*>( drawVect[iPlot] );
    hist->SetDirectory(0);
    int bin =1;
    for ( unsigned int iLine=0; iLine<nLine; ++iLine ) {
      unsigned int iColMax =  doTriangular ? iLine+1 : nCol;
      for ( unsigned int iCol=0; iCol<iColMax; iCol++ ) {
	if ( (*matrix)(iLine, iCol) != 100 ) hist->SetBinContent( bin, (*matrix)(iLine, iCol) );
	hist->SetBinError( bin, 0 );
	hist->GetXaxis()->SetBinLabel( bin, TString::Format( "%d_%d", iLine, iCol ) );
	bin++;
      }
    }
    hist->SetLineWidth( 1 );
    hist->LabelsOption("v" );
    hist->GetXaxis()->SetTitle( "Line_Column" );
  }
  vectObj.push_back( drawVect );  
}

//=========================================================
 void ChrisLib::IncreaseHist( TH1** hist, const string &label ) {

  unsigned nBins = (*hist)->GetNbinsX()+1;
  TH1* outHist = 0;
  if ( string( (*hist)->ClassName()) == "TH1D" ) outHist = new TH1D( "dum", "dum", nBins, -0.5, -0.5+nBins );
  else if ( string( (*hist)->ClassName()) == "TProfile" ) outHist = new TProfile( "dum", "dum", nBins, -0.5, -0.5+nBins );
  else throw runtime_error( "IncreaseHist : Invalid input type." );

  for ( unsigned iBin=1; iBin<nBins; ++iBin ) {
    outHist->SetBinContent( iBin, (*hist)->GetBinContent(iBin) );
    outHist->SetBinError( iBin, (*hist)->GetBinError(iBin) );
    outHist->GetXaxis()->SetBinLabel( iBin, (*hist)->GetXaxis()->GetBinLabel(iBin) );
  }
  outHist->GetXaxis()->SetBinLabel( nBins, label.c_str() );

  string name = (*hist)->GetName();
  string title = (*hist)->GetTitle();
  delete (*hist);
  *hist = outHist;
}

//================================================
 bool ChrisLib::IsTH1( OutMode outMode ) {
   if ( outMode == OutMode::hist 
	|| outMode == OutMode::histEvent
	|| outMode == OutMode::profile
	) return true;
   else return false;
 }

 //==============================================

