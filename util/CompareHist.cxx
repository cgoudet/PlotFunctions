#include <iostream>
#include <vector>
#include "TFile.h"
#include "TH1D.h"
#include <string>
#include <fstream>
#include <math.h>
#include "TTree.h"
#include "PlotFunctions/InputCompare.h"
#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/MapBranches.h"
#include <TROOT.h>
#include "TMatrixD.h"
#include "TProfile.h"
using std::string;
using std::cout;
using std::endl;
using std::vector;
#include "TGraphErrors.h"

#include "PlotFunctions/DrawPlot.h"
#include <boost/program_options.hpp>
#include <boost/multi_array.hpp>
using boost::multi_array;
using boost::extents;

namespace po = boost::program_options;

#define DEBUG 1

/**
   \param inFiles Name of the input configuration files

   Description of general attributes from InputCompare : \

   inputType : choose the type of object to draw
   rootFileName : 

   Description of the use of attributes from InputCompare on each case : \

   inputType : \n
    0 : Plot TH1
    1 : TTree
    used variables : varName, (varMin + varMax) || xBinning
    2 : comparison event by event
    eventID : names of branches that identify an event
    varName : names of the branches to compare
    nComparedEvents : number of events to be compared
    lengend : name of the column in csv file

    3 : plot text files
    4 : merging some histograms into 1 root file
    5 : separate a set of tree into two subTree according to a selection
    6 : cmopare all bins of histograms ( of the same size )  as a function of the legend (as label)
    7 : Compare TMatrixD
    8 : TProfile



 */
int main( int argc, char* argv[] ) {

  po::options_description desc("LikelihoodProfiel Usage");

  vector<string> inFiles;
  //define all options in the program
  desc.add_options()
    ("help", "Display this help message")
    ("inFiles", po::value<vector <string> >(&inFiles), "" )
    ;
  
  //Define options gathered by position                                                          
  po::positional_options_description p;
  p.add("inFiles", -1);

  // create a map vm that contains options and all arguments of options       
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).style(po::command_line_style::unix_style ^ po::command_line_style::allow_short).run(), vm);
  po::notify(vm);
  
  if (vm.count("help")) {cout << desc; return 0;}
  //=============================================
  string plotPath;// = "/sps/atlas/c/cgoudet/Plots/";

  for ( unsigned int iFile = 0; iFile < inFiles.size(); iFile++ ) {
    cout << "iFile : " << iFile << " " << inFiles[iFile] << endl;
    InputCompare input( inFiles[iFile] );
    if ( DEBUG ) cout << "config file loaded" << endl;
    plotPath = input.GetOption("plotDirectory");
    vector< vector< TH1* > > vectHist;
    vector< vector< TGraphErrors* > > vectGraph;
    multi_array< double, 2 > eventVarVect;
    multi_array< long long int, 2> eventIDVect;

    TFile *outFile = 0;    
    vector< vector< string > > inputRootFile = input.GetRootFileName();
    vector< vector< string > > inputObjName = input.GetObjName();
    TTree *treePassSel=0, *treeRejSel=0;
    MapBranches mapBranch;

    for ( unsigned int iPlot = 0; iPlot < inputRootFile.size(); iPlot++ ) {
      for ( unsigned int iAdd = 0; iAdd < inputRootFile[iPlot].size(); iAdd ++ ) {
	string dumString = inputRootFile[iPlot][iAdd];
	
	if ( !TString( inputRootFile[iPlot][iAdd] ).Contains( ".root" ) )
	  cout << system( ("ls " + dumString).c_str() ) << endl;

	TFile inFile( inputRootFile[iPlot][iAdd].c_str() );	
	if ( DEBUG ) cout << iPlot << " " << iAdd << endl;

	switch( atoi(input.GetOption("inputType").c_str()) ) {
	case 0 : {//histograms
	  if ( !iPlot && !iAdd ) vectHist.push_back( vector< TH1* >() );
	  if ( !iAdd ) vectHist.back().push_back( 0 );
	  if ( !vectHist.back().back() ) {
	    //Get the histograms from input file and rename it to avoid root overwritting
	    vectHist.front().back() = (TH1D*) inFile.Get( inputObjName[iPlot][iAdd].c_str() );
	    if ( !vectHist.front().back() ) {
	      cout << "histogram not found : " << inputObjName[iPlot][iAdd] << " in file " << inputRootFile[iPlot][iAdd] << endl;
	      return 1 ;
	    }
	    vectHist.front().back()->SetName( TString::Format( "%s_%d", inputObjName[iPlot][iAdd].c_str(), iPlot ) );
	    vectHist.front().back()->SetDirectory( 0 );  
	  }
	  //If the histogram have already been created, we have to add the new one.
	  else vectHist.front().back()->Add( (TH1D*) inFile.Get( inputObjName[iPlot][iAdd].c_str() ) );
	  break;
	}
	  
	  //############################################
	case 1 : {//TTree plotting
	  vector< vector<string> > &varName = input.GetVarName();
	  vector< double > &varMin  = input.GetVarMin();
	  vector< double > &varMax  = input.GetVarMax();
	  vector< vector<string> > &varWeight = input.GetVarWeight();
	  vector< double > varVal( varName[iPlot].size(), 0 );
	  vector< vector< double > > &xBinning = input.GetXBinning();

	  if ( !xBinning.size() && ( varMin.size() != varMax.size() || ( varName.size()>iPlot && varName[iPlot].size() != varMin.size() ) ) ) {
	    cout << "varMin and varMax sizes matching : " << varMin.size() << " " << varMax.size() << endl;
	    if ( varName.size()>iPlot ) cout << "varName[iPlot].size() : "  << varName[iPlot].size() << endl;
	    return 1;
	  }

	  //define the weight vector
	  vector<string> dumVect( 1, "X" );
	  while ( varWeight.size()<=iPlot ) varWeight.push_back( varWeight.size() ? varWeight.back() : dumVect );


	  TTree *inTree = (TTree*) inFile.Get( inputObjName[iPlot][iAdd].c_str() );
	  if ( !inTree ) { cout << inputObjName[iPlot][iAdd] << " does not exist in " << inFile.GetName() << endl; exit(0); }
	  inTree->SetDirectory(0);

	  //if requested perform a selection on the tree using copyTree
	  if ( input.GetSelectionCut().size() ){
	    TFile *dumFile = new TFile( "/tmp/cgoudet/dumFile", "RECREATE" );
	    gROOT->cd();
	    TTree* dumTree = inTree->CopyTree( input.GetSelectionCut()[iPlot].c_str() );
	    if ( dumTree ) {
	      delete inTree;
	      inTree= dumTree;
	      inTree->SetDirectory(0);
	    }
	    delete dumFile; dumFile=0;
	  }


	  unsigned int nEntries = (unsigned int) inTree->GetEntries();
	  if ( DEBUG ) cout << "nEntries : " << nEntries << endl;
	  //Make the number of legend and histogram match by adding 0 histograms (which will not be printed)
	  if ( !vectHist.size() ) vectHist = vector< vector< TH1* > >( varName[iPlot].size(), vector<TH1*>() );
	  for ( unsigned int iHist = 0; iHist < vectHist.size(); iHist++ ) {
	    while ( vectHist[iHist].size() <= iPlot )  vectHist[iHist].push_back( 0 );
	  }
	  if ( !nEntries ) { delete inTree; inTree = 0; inFile.Close( "R" ); }

	  //create a vector to store all branches names to be linked
	  vector<string> linkedVariables;
	  for ( unsigned int iWeight=0; iWeight<varWeight[iPlot].size(); iWeight++ ) 
	    if ( varWeight[iPlot][iWeight] != "X" ) linkedVariables.push_back( varWeight[iPlot][iWeight] );
	  for ( unsigned int iHist = 0; iHist < varName[iPlot].size(); iHist++ ) 
	    if ( varName[iPlot][iHist] != "X" ) linkedVariables.push_back( varName[iPlot][iHist] );
	  mapBranch.LinkTreeBranches( inTree, 0, linkedVariables );


	  for ( unsigned int iEvent = 0; iEvent < nEntries; iEvent++ ) {
	    //	    if ( vectHist.front()[iPlot] && vectHist.front()[iPlot]->GetEntries() > 20 ) continue;
	    double totWeight=1;
	    //Read the tree entry. As we run over all plotted variables, the entry need not to be read several times
	    inTree->GetEntry( iEvent );

	    for ( auto vWeightName : varWeight[iPlot] )  totWeight *= vWeightName == "X" ? 1 : mapBranch.GetVal( vWeightName );

	    for ( unsigned int iHist = 0; iHist < varName[iPlot].size(); iHist++ ) {
	      if ( !vectHist[iHist][iPlot] ) {
		//Create correspondig histogram
		unsigned int nBins = atoi(input.GetOption("nComparedEvents").c_str());
		if ( !nBins ) nBins = 100;
		
		string dumName = string( TString::Format( "%s_%s_%d", input.GetObjName()[iPlot][iAdd].c_str(), varName[iPlot][iHist].c_str(), iPlot ) );
		if ( xBinning.size() <= iHist || !xBinning[iHist].size() ) vectHist[iHist][iPlot] = new TH1D( dumName.c_str(), dumName.c_str(), nBins, varMin[iHist], varMax[iHist] );
		else vectHist[iHist][iPlot] = new TH1D( dumName.c_str(), dumName.c_str(), (int) xBinning[iHist].size()-1, &xBinning[iHist][0] );
		vectHist[iHist][iPlot]->GetXaxis()->SetTitle( varName[iPlot][iHist].c_str() );
		//		  vectHist[iHist][iPlot]->GetYaxis()->SetTitle( TString::Format( "# Events / %2.2f", (varMax[iHist]-varMin[iHist])/vectHist[iHist][iPlot]->GetNbinsX()) );
		vectHist[iHist][iPlot]->GetYaxis()->SetTitle( "# Events" );
		vectHist[iHist][iPlot]->SetDirectory( 0 );
		vectHist[iHist][iPlot]->Sumw2();
	      }
		
	      //if created fill it
	      vectHist[iHist][iPlot]->Fill( mapBranch.GetVal(varName[iPlot][iHist] ) , totWeight );
	    }// End iHist
	    if ( iEvent < 10 ) cout << totWeight << endl;
	  }// end iEvent


	  delete inTree; inTree = 0;
	  break;
	}//end case TTree

	case 2 : { //event/event comparison

	  TTree *inTree = (TTree*) inFile.Get( inputObjName[iPlot][iAdd].c_str() );
	  if ( !inTree ) {
	    cout << inputObjName[iPlot][iAdd] << " not found in " << inFile.GetName() << endl;
	    exit(0);
	  }
	  if ( input.GetSelectionCut().size() ){
	    TTree *dumTree = inTree;
	    gROOT->cd();
	    inTree = dumTree->CopyTree( input.GetSelectionCut()[iPlot].c_str() );
	    inTree->SetDirectory(0);
	    delete dumTree;
	  }

	  vector< string > &eventID = input.GetEventID();
	  vector< long long int > eventIDVal( eventID.size(), 0 );
	  for ( unsigned int i = 0; i<eventID.size(); i++ ) inTree->SetBranchAddress( eventID[i].c_str(), &eventIDVal[i] );

	  vector< vector<string> > &eventVar = input.GetVarName();
	  if ( !eventVar.size() ) {
	    cout << "no variable name given" << endl;
	    exit(0);
	  }
	  if ( eventVar.size() <= iPlot ) eventVar.push_back( eventVar.back() );
	  vector< double > eventVarVal(eventVar[iPlot].size(), 0 );
	  for ( unsigned int i = 0; i<eventVar[iPlot].size(); i++ ) inTree->SetBranchAddress( eventVar[iPlot][i].c_str(), &eventVarVal[i] );
	  //resize the 2D vector
	  if ( !iPlot && !iAdd ) {
	    eventVarVect.resize( extents[eventVar[iPlot].size()*inputRootFile.size()][0] );
	    eventIDVect.resize( extents[eventID.size()][0] );
	  }

	  unsigned int nComparedEvents = atoi(input.GetOption("nComparedEvents").c_str());
	  unsigned int nEntries = (unsigned int) inTree->GetEntries();
	  vector< double > varMin  = input.GetVarMin();
	  vector< double > varMax  = input.GetVarMax();
	  vector< double > varVal( eventVar.size(), 0 );

	  //Create the histograms for all plotted variables
	  for ( unsigned int iVar = 0; iVar < eventVar[iPlot].size(); iVar++ ) {
	    if ( !iAdd && !iPlot ) vectHist.push_back( vector<TH1*>() );
	    if ( !iAdd ) {
	      string dumString = input.GetOutName() + string( TString::Format( "_%s_%d", eventVar[iPlot][iVar].c_str(), iPlot ));
	      vectHist[iVar].push_back(0);
	      vectHist[iVar].back() = new TH1D( dumString.c_str(), dumString.c_str(), 100, varMin[iVar], varMax[iVar] );
	      vectHist[iVar].back()->SetDirectory(0);
	      vectHist[iVar].back()->GetXaxis()->SetTitle( eventVar[iPlot][iVar].c_str() );
	      vectHist[iVar].back()->GetYaxis()->SetTitle( "#events" );
	      vectHist[iVar].back()->Sumw2();
	    }
	  }

	  //Run over all events
	  for ( unsigned int iEvent = 0; iEvent < nEntries; iEvent++ ) {	  
	    if ( !iPlot && nComparedEvents && eventVarVect[0].size() >= nComparedEvents ) break;
	    inTree->GetEntry( iEvent );
	    unsigned int index = iEvent;
	    if ( !iPlot ) { //I save the informations of the event I want to compare
	      //Fill the event ID informations
	      eventIDVect.resize( extents[eventIDVect.size()][iEvent+1] );		
	      for ( unsigned int i = 0; i<eventID.size(); i++ )  eventIDVect[i][iEvent] = eventIDVal[i];		  
	      eventVarVect.resize( extents[eventVarVect.size()][iEvent+1] );
	    }
	    else { //look for the right index to place values
	      index = iEvent+1;

	      for ( unsigned int iSavedEvent = 0; iSavedEvent < eventVarVect[0].size(); iSavedEvent++ ) {
		bool foundEvent =true;
		for ( unsigned int iID = 0; iID < eventIDVect.size(); iID++ ) {
		  if ( eventIDVect[iID][iSavedEvent] == eventIDVal[iID] ) continue;
		  foundEvent = false;
		  break;
		}

		if ( !foundEvent ) continue;
		index = iSavedEvent;
		break;
	      }// end iSavedEvent
		
	    }//end else

	    // get to the next event if not found in selected events
	    if ( index == iEvent+1 ) continue;
	    for ( unsigned int i = 0; i<eventVar[iPlot].size(); i++ ) {
	      eventVarVect[i*inputRootFile.size()+iPlot][index] = eventVarVal[i];  
	      vectHist[i][iPlot]->Fill( eventVarVal[i] );

	      //		if ( !i ) cout << iPlot << " " << iAdd << " " << i << " " << eventVarVal[i] << endl;
	    }// end i

	  }// end iEvent

	  break;
	}//end case compare event

	case 3 : {
	  fstream inputStream;
	  inputStream.open( inputRootFile[iPlot][iAdd].c_str(), fstream::in );
	  string dumString;
	  getline( inputStream, dumString );
	  vector< string > titleVect;
	  ParseVector(  dumString , titleVect );

	  vector< vector<string> > varName = input.GetVarName();
	  vector< double > varVal( titleVect.size(), 0 );
	  vector< unsigned int > varIndex( varName[iPlot].size(), 0 );
	  for ( unsigned int iVar = 0; iVar < varName[iPlot].size(); iVar++ ) {
	    varIndex[iVar] = SearchVectorBin( varName[iPlot][iVar], titleVect );
	    if ( varIndex[iVar] == titleVect.size() ) {
	      cout << varName[iPlot][iVar] << " was not found in header" << endl;
	      exit(0);
	    }
	  }
	  vector< double > varMin  = input.GetVarMin();
	  vector< double > varMax  = input.GetVarMax();
	  vector< vector<string> > varWeight = input.GetVarWeight();
	  vector<unsigned int> weightIndices;
	  if ( varWeight.size() ) {
	    for ( unsigned int iWeight = 0; iWeight < varWeight.front().size(); iWeight++ ){
	      if ( varWeight[iPlot][iWeight] == "X" ) continue;	     
	      weightIndices.push_back( SearchVectorBin( varWeight[iPlot][iWeight], titleVect ) );
	    }
	  }
	  while ( true ) {
	    for ( unsigned int iTxtVar = 0; iTxtVar < titleVect.size(); iTxtVar++ ) {
	      inputStream >> varVal[iTxtVar];
	    }
	    if ( inputStream.eof() ) break;
	    //	    double weight = ( varWeight[iPlot] != "X" ) ? varVal[weightIndex] : 1;
	    double weight = 1;
	    for ( unsigned int iWeight = 0; iWeight < weightIndices.size(); iWeight++ ){
	      weight *= varVal[weightIndices[iWeight]];
	    }
	    for ( unsigned int iVar = 0; iVar < varName[iPlot].size(); iVar++ ) {
	      if ( vectHist.size() == iVar ) vectHist.push_back( vector<TH1*>() );
	      if ( vectHist[iVar].size() == iPlot ) {
		string dumString = input.GetOutName() + string( TString::Format( "_%s_%d", varName[iPlot][iVar].c_str(), iPlot ));
		vectHist[iVar].push_back(0);
		vectHist[iVar].back() = new TH1D( dumString.c_str(), dumString.c_str(), 100, varMin[iVar], varMax[iVar] );
		vectHist[iVar].back()->SetDirectory(0);
		vectHist[iVar].back()->GetXaxis()->SetTitle( varName[iPlot][iVar].c_str() );
		vectHist[iVar].back()->GetYaxis()->SetTitle( "#events" );
		vectHist[iVar].back()->Sumw2();
	      }
	      vectHist[iVar][iPlot]->Fill( varVal[varIndex[iVar]], weight );
	    }//end for iVar
	  }//end while
	  cout << "endwhile" << endl;
	  break;
	}

	case 4 : {

	  if ( !iPlot && !iAdd ) outFile = new TFile( string( plotPath + input.GetOutName()+ ".root").c_str(), "RECREATE"  );
	  outFile->cd();
	  inFile.Get( inputObjName[iPlot][0].c_str() )->Write( input.GetLegend()[iPlot].c_str(), TObject::kOverwrite );
	  break;
	}

	case 5 : {
	  for ( unsigned int iPass = 0; iPass < 2; iPass++ ) {
	    TTree *selTree = iPass ? treeRejSel : treePassSel;
	    if ( !iAdd && selTree ) SaveTree( selTree, plotPath );
	    
	    string treeName = inputObjName[iPlot].size() ?  ( inputObjName[iPlot].size()>iAdd ?  inputObjName[iPlot][iAdd].c_str()  : inputObjName[iPlot].back() ) : FindDefaultTree( &inFile ).c_str() ;
	    TTree *inTree = (TTree*) inFile.Get( treeName.c_str() );
	    if ( !inTree ) {
	      cout << treeName << " doesn't exist in " << inFile.GetName() << endl;
	      continue;
	    }
	    inTree->SetDirectory(0);
	    
	    gROOT->cd();
	    string dumString = inFile.GetName();
	    treeName = StripString( dumString ) + "_" + input.GetOutName() + ( iPass ? "_RejSel" : "_PassSel" );	  
	    string selection = input.GetSelectionCut()[iPlot];
	    if ( iPass ) selection = "!(" + selection + ")";
	    TTree *dumTree = inTree->CopyTree( selection.c_str() );
	    dumTree->SetDirectory(0);

	    if ( selTree ) selTree->Print();
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

	    if ( iPlot==inputRootFile.size()-1 && iAdd == inputRootFile.back().size()-1 && selTree ) SaveTree( selTree, plotPath );
	  }//end iPass
	  
	  break;
	}
	  
	case 6 : {
	  if ( iAdd ) continue;
	  vector<string> &labelVect = input.GetLegend();
	  if ( inputRootFile.size() != labelVect.size() ) {
	    cout << "legend size and total files do not have the same size." << endl;
	    exit(0);
	  }

	  TH1D* inHist = (TH1D*) inFile.Get(  inputObjName[iPlot][iAdd].c_str() );
	  if ( !inHist ) {
	    cout << inputObjName[iPlot][iAdd] << " does not exist within " << inFile.GetName() << endl;
	    exit(0);
	  }
	  if ( !iPlot )  vectHist.push_back( vector<TH1*>( inHist->GetNbinsX(), 0) );
	
	  for ( unsigned int iBin = 0; iBin < (unsigned int) inHist-> GetNbinsX(); iBin++ ) {
	    if ( !vectHist.front()[iBin] ) {
	      vectHist.front()[iBin] = new TH1D( input.GetOutName().c_str(), input.GetOutName().c_str(), labelVect.size(), -0.5, labelVect.size()-0.5 );
	      vectHist.front()[iBin]->SetDirectory(0);
	      vectHist.front()[iBin]->GetYaxis()->SetTitle( inHist->GetYaxis()->GetTitle() );
	    }
	    vectHist.front()[iBin]->GetXaxis()->SetBinLabel( iPlot+1, labelVect[iPlot].c_str() );
	    vectHist.front()[iBin]->SetBinContent( iPlot+1, inHist->GetBinContent( iBin+1 ) );
	    vectHist.front()[iBin]->SetBinError( iPlot+1, inHist->GetBinError( iBin+1 ) );

	  }//end for iBin
	  if ( iPlot == inputRootFile.size()-1 ) {
	    labelVect.clear();
	    for ( unsigned int iPlot = 0; iPlot < vectHist.front().size(); iPlot++ ) {
	      labelVect.push_back( string( TString::Format( "Bin %d", iPlot )) );
	      vectHist.front()[iPlot]->LabelsOption("v" );
	    }
	  }
	  delete inHist;
	  break;
	}//end case 6
	  
	case 7 : {
	  if ( iAdd )  continue;
	  TMatrixD *matrix = ( TMatrixD*) inFile.Get( inputObjName[iPlot][iAdd].c_str() );
	  if ( !matrix ) {
	    cout << inputObjName[iPlot][iAdd].c_str() << " not found in " << inFile.GetName() << endl;
	    exit(0);
	  }
	  //	  matrix->Print();
	  unsigned int nLine = matrix->GetNrows();
	  unsigned int nCol = matrix->GetNcols();
	  if ( !iPlot ) vectHist.push_back( vector<TH1*>() );
	  vectHist.front().push_back(0);
	  TString histTitle;
	  bool doDiagonalize = ( nCol==nLine && input.GetOption( "diagonalize" ) != "" && atoi( input.GetOption( "diagonalize" ).c_str() ) );
	  unsigned int nBins = doDiagonalize ? nLine*(nLine+1)/2 : nLine*nCol;
	  vectHist.front().back() = new TH1D( histTitle, histTitle, nBins, 0.5, nBins+0.5 );
	  vectHist.front().back()->SetDirectory(0);
	  int bin =1;
	  for ( unsigned int iLine=0; iLine<nLine; iLine++ ) {
	    unsigned int iColMax =  doDiagonalize ? iLine+1 : nCol;
	    for ( unsigned int iCol=0; iCol<iColMax; iCol++ ) {
	      if ( (*matrix)(iLine, iCol) != 100 ) vectHist.front().back()->SetBinContent( bin, (*matrix)(iLine, iCol) );
	      vectHist.front().back()->SetBinError( bin, 0 );
	      vectHist.front().back()->GetXaxis()->SetBinLabel( bin, TString::Format( "%d_%d", iLine, iCol ) );
	      bin++;
	    }
	  }
	  vectHist.front().back()->SetLineWidth( 1 );
	  vectHist.front().back()->LabelsOption("v" );
	  vectHist.front().back()->GetXaxis()->SetTitle( "Line_Column" );
	  break;
	}

	case 8 : {//TTree into TProfile
	  vector< vector<string> > &varName = input.GetVarName();
	  while ( iPlot && varName.size() <= iPlot ) varName.push_back( varName.back() );

	  vector< double > varMin  = input.GetVarMin();
	  vector< double > varMax  = input.GetVarMax();
	  vector< vector<string> > varWeight = input.GetVarWeight();
	  MapBranches mapB;
	  vector< vector<double> > xBinning = input.GetXBinning();
	  if ( xBinning.size() ) while ( xBinning.size() <= iPlot ) xBinning.push_back( xBinning.back() );

	  if ( !xBinning.size() && ( varMin.size() != varMax.size() || !varMin.size() ) ) {
	    cout << "varMin and varMax sizes matching : " << varMin.size() << " " << varMax.size() << endl;
	    if ( varName.size()>iPlot ) cout << "varName[iPlot].size() : "  << varName[iPlot].size() << endl;
	    return 1;
	  }

	  vector<string> dumVectWeight( 1, "X" );
	  if ( !varWeight.size() ) varWeight.push_back( vector<string>(1,"X" ) );
	  while ( iPlot>=varWeight.size() ) varWeight.push_back( varWeight.back() );

	  vector<double> weight;
	  TTree *inTree = (TTree*) inFile.Get( inputObjName[iPlot][iAdd].c_str() );
	  if ( !inTree ) { cout << inputObjName[iPlot][iAdd] << " does not exist in " << inFile.GetName() << endl; exit(0);}
	  //	  inTree->SetDirectory(0);

	  if ( input.GetSelectionCut().size() ){
	    //	    TFile *dumFile = new TFile( "/tmp/cgoudet/dumFile", "RECREATE" );
	    gROOT->cd();
	    TTree* dumTree = inTree->CopyTree( input.GetSelectionCut()[iPlot].c_str() );
	    if ( dumTree ) {
	      delete inTree;
	      inTree= dumTree;
	      inTree->SetDirectory(0);
	    }
	  }
	  mapB.LinkTreeBranches( inTree );
	  unsigned int nEntries = (unsigned int) inTree->GetEntries();

	  for ( unsigned int iEvent = 0; iEvent < nEntries; iEvent++ ) {
	    double totWeight = 1;
	    for ( unsigned int iHist = 0; iHist < varName[iPlot].size(); iHist++ ) {
	      if ( !iEvent ) {
		if ( !iPlot && !iAdd )  vectHist.push_back( vector<TH1*>() );
		if ( !iAdd )  vectHist[iHist].push_back( 0 );
	      }

	      //Read the tree entry. As we run over all plotted variables, the entry need not to be read several times
	      if ( !iHist ) {
		inTree->GetEntry( iEvent );
		for ( unsigned int iWeight=0; iWeight< varWeight[iPlot].size(); iWeight++ ) totWeight *= varWeight[iPlot][iWeight]=="X" ? 1 : mapB.GetVal( varWeight[iPlot][iWeight] );
	      }

	      if ( !vectHist[iHist][iPlot] ) {
		//Create correspondig histogram
		string dumName = string( TString::Format( "%s_%s_%d", input.GetObjName()[iPlot][iAdd].c_str(), varName[iPlot][iHist].c_str(), iPlot ) );
		if ( !xBinning.size() ) vectHist[iHist][iPlot] = new TProfile( dumName.c_str(), dumName.c_str(), 100, varMin[iHist], varMax[iHist] );
		else vectHist[iHist][iPlot] = new TProfile( dumName.c_str(), dumName.c_str(), (int) xBinning[iPlot].size()-1, &xBinning[iPlot][0] );
		vectHist[iHist][iPlot]->SetDirectory( 0 );
	      }
	      //if created fill it
	      else {
		((TProfile*) vectHist[iHist][iPlot])->Fill( mapB.GetVal( varName[iPlot].front() ), mapB.GetVal( varName[iPlot][iHist] ), totWeight );
	      }
	    }// End iHist
	  }// end iEvent

	  delete inTree; inTree = 0;
	  break;
	}//end case TProfile
	  
	  //--------------------------------
	case 9 : { //TGraphError
	  cout << "case 9" << endl;
	  vector< vector<string> > &varName = input.GetVarName();
	  vector< vector<string> > &varErrX = input.GetVarErrX();
	  vector< vector<string> > &varErrY = input.GetVarErrY();
	  vector< vector<string> > &varWeight = input.GetVarWeight();

	  map<string, double> mapVars;

	  //Test inputs
	  if ( !varName.size() ) { cout << "No variable to be printed in varName" << endl; exit(0); }
	  //By default, if less names of variables than root files are entered, the last entry will be compied
	  while ( varName.size() <= iPlot ) varName.push_back( varName.back() );
	  while ( varWeight.size() < varName.size() ) varWeight.push_back( varWeight.back() );
	  // if number of x and Y variables are different exit
	  if ( varName[iPlot].size() != varWeight[iPlot].size() ) { cout << "X (varName) and Y (varWeight) branches have different sizes." << endl; exit(0); }
	  if ( !varErrX.size() ) varErrX.push_back( vector<string>( varName[iPlot].size(), "X" ) );
	  while ( varErrX.size() < varName.size() ) varErrX.push_back( varErrX.back() );
	  if ( !varErrY.size() ) varErrY.push_back( vector<string>( varName[iPlot].size(), "X" ) );
	  while ( varErrY.size() < varName.size() ) varErrY.push_back( varErrX.back() );

	  cout << "varNameSize : " << varName.size() << endl;

	  TTree *inTree = (TTree*) inFile.Get( inputObjName[iPlot][iAdd].c_str() );
	  if ( !inTree ) { cout << "inTree " << inputObjName[iPlot][iAdd] << " not found in " << inFile.GetName() << endl; exit(0);}
	  //	  inTree->SetDirectory(0);
	  if ( input.GetSelectionCut().size() ){
	    TFile *dumFile = new TFile( "/tmp/cgoudet/dumFile", "RECREATE" );
	    gROOT->cd();
	    TTree* dumTree = inTree->CopyTree( input.GetSelectionCut()[iPlot].c_str() );
	    if ( dumTree ) {
	      delete inTree;
	      inTree= dumTree;
	      inTree->SetDirectory(0);
	    }
	    delete dumFile; dumFile=0;
	  }
	  //Setting the branches for all branches names
	  for ( auto vName : varName[iPlot] ) inTree->SetBranchAddress( vName.c_str(), &mapVars[vName] );
	  for ( auto vName : varWeight[iPlot] ) inTree->SetBranchAddress( vName.c_str(), &mapVars[vName] );
	  for ( auto vName : varErrX[iPlot] ) inTree->SetBranchAddress( vName.c_str(), &mapVars[vName] );
	  for ( auto vName : varErrY[iPlot] ) inTree->SetBranchAddress( vName.c_str(), &mapVars[vName] );
	  mapVars["X"]=0;


	  unsigned int nentries = inTree->GetEntries();
	  for ( unsigned int iEntry = 0; iEntry<nentries; iEntry++ ) {

	    inTree->GetEntry( iEntry );

	    for ( unsigned int iVar = 0; iVar<varName[iPlot].size(); iVar++ ) {
	      cout << "iVar : " << iVar << endl;
	      while ( vectGraph.size() <= iVar ) vectGraph.push_back( vector<TGraphErrors*>() );
	      while ( vectGraph[iVar].size() <= iPlot ) vectGraph[iVar].push_back(0);
	      cout << vectGraph.size() << endl;
	      cout << vectGraph[iVar].size() << endl;
	      cout << vectGraph[iVar][iPlot] << endl;
	      if ( !vectGraph[iVar][iPlot] ) {
		vectGraph[iVar][iPlot] = new TGraphErrors( nentries );
		vectGraph[iVar][iPlot]->SetName( TString::Format( "%s_%s_%s_%s_%d", varName[iPlot][iVar].c_str(), varWeight[iPlot][iVar].c_str(), varErrX[iPlot][iVar].c_str(), varErrY[iPlot][iVar].c_str(), iPlot ) );
		vectGraph[iVar][iPlot]->GetXaxis()->SetTitle( varName[iPlot][iVar].c_str() );
		vectGraph[iVar][iPlot]->GetYaxis()->SetTitle( varWeight[iPlot][iVar].c_str() );
	      }
	      cout << "created" << endl;
	      vectGraph[iVar][iPlot]->SetPoint( iEntry, mapVars[varName[iPlot][iVar]], mapVars[varWeight[iPlot][iVar]] );
	      cout << "error" << endl;
	      vectGraph[iVar][iPlot]->SetPointError( iEntry, mapVars[varErrX[iPlot][iVar]], mapVars[varErrY[iPlot][iVar]] );
	      cout.precision(10);
	      cout << "print" << endl;
	      cout << varName.size() << " " << iVar << endl;
	      cout << varName[iVar].size() << " " << iPlot << endl;
	      cout << varName[iPlot][iVar] << endl;
	      cout << mapVars[varName[iPlot][iVar]] << endl;
	      cout << iEntry << " " << mapVars[varName[iPlot][iVar]] << endl;
	      cout <<  mapVars[varWeight[iPlot][iVar]]  << endl;
	      cout << mapVars[varErrX[iPlot][iVar]] << endl;
	      cout << mapVars[varErrY[iPlot][iVar]] << endl;
	    }//end iVar
	  }//end foriEntry

	  delete inTree; inTree=0;
	  break;
	}//end case TGraphErr (9)
	  //---------------------------

	default : 
	  cout << "inputType=" << input.GetOption("inputType") << " is not known." << endl;
	  exit(0);
	}//end switch inputType
	inFile.Close("R");	
      }// end iAdd
    }//end iPlot

    if ( DEBUG ) cout << "nHist : " << vectHist.size() << endl;
    for ( unsigned int iHist = 0; iHist < vectHist.size(); iHist++ ) {
      DrawPlot( vectHist[iHist], 
		plotPath + input.GetOutName()+ ( input.GetVarName().size() && input.GetVarName().front().size() ? "_" + input.GetVarName().front()[iHist] : "" ),
		input.CreateVectorOptions()
		);
    }

    if ( vectHist.size() && atoi(input.GetOption("doTabular").c_str()) ) {
      fstream stream;
      stream.open( string( plotPath + input.GetOutName() + ( input.GetVarName().size() && input.GetVarName().front().size() ? "_" + input.GetVarName().front().front() : "") + ".csv"), fstream::out | fstream::trunc );
      for ( int iBin = 0; iBin <= vectHist.front().front()->GetNbinsX(); iBin++ ) {
	for ( unsigned int iPlot = 0; iPlot <= vectHist.front().size(); iPlot++ ) {
	  if ( !iBin ) {
	    if ( iPlot ) {
	      string name = string( input.GetLegend().size() ? input.GetLegend()[iPlot-1] : vectHist.front()[iPlot-1]->GetName() ) ;
	      stream << name;
	      if ( atoi(input.GetOption("doTabular").c_str()) ==2 ) stream << "," << name + " err";
	    }
	    else {
	      TString colName = vectHist.front().front()->GetXaxis()->GetTitle();
	      colName=colName.ReplaceAll("_", "" ).ReplaceAll("#", "" ) ;
	      stream << colName; 
	    }
	  }
	  else {
	    if ( iPlot ) { 
	      stream << vectHist.front()[iPlot-1]->GetBinContent( iBin );
	      if ( atoi(input.GetOption("doTabular").c_str()) ==2 ) stream << "," << vectHist.front()[iPlot-1]->GetBinError( iBin );
		}	 
	    else stream << ( strcmp( vectHist.front().front()->GetXaxis()->GetBinLabel(iBin), "" ) ? TString(vectHist.front().front()->GetXaxis()->GetBinLabel(iBin)) :  TString::Format( "] %2.2f : %2.2f]", vectHist.front().front()->GetXaxis()->GetBinLowEdge( iBin ), vectHist.front().front()->GetXaxis()->GetBinUpEdge( iBin ) ) );
	  } 
	  if ( iPlot != vectHist.front().size() ) stream << ",";
	}
	stream << endl;
      }
      stream.close();
    }

    if ( atoi(input.GetOption("inputType").c_str()) == 2 ) { //print csvFile
      fstream csvStream;
      cout << string( plotPath+input.GetOutName() + ".csv" ).c_str() << endl;
      csvStream.open( string( plotPath+input.GetOutName() + ".csv" ).c_str(), fstream::out | fstream::trunc );
      vector< string > eventID = input.GetEventID();
      vector< vector<string> > eventVar = input.GetVarName();      
      vector<string> legend = input.GetLegend();
      //Write column names
      for ( unsigned int iVar = 0; iVar < eventIDVect.size(); iVar++ ) csvStream << eventID[iVar] << ",";
      for ( unsigned int iVar = 0; iVar < eventVarVect.size(); iVar++ ) {
	TString columnTitle = eventVar[iVar%eventVar.size()][iVar/eventVar.size()] + "_" + ( legend.size() ? legend[iVar%eventVar.size()] : "" );
	columnTitle = columnTitle.ReplaceAll( "_", "\\_" );
	csvStream <<  columnTitle << ",";
      }
      csvStream << endl;


      for ( unsigned int iEvent = 0; iEvent< eventIDVect[0].size(); iEvent++ ) {
	for ( unsigned int iVar = 0; iVar < eventIDVect.size(); iVar++ ) csvStream << eventIDVect[iVar][iEvent] << ",";
	for ( unsigned int iVar = 0; iVar < eventVarVect.size(); iVar++ ) {
	  csvStream << eventVarVect[iVar][iEvent] << ",";
	}
	csvStream << endl;
      }
      csvStream.close();
      cout << "csv file saved" << endl;
    }

    //Plotting graphs
    cout << "nGraph : " << vectGraph.size() << endl;
    for ( unsigned int iHist = 0; iHist < vectGraph.size(); iHist++ ) {
      DrawPlot( vectGraph[iHist], 
		plotPath + input.GetOutName()+ ( input.GetVarName().size() && input.GetVarName().front().size() ? "_" + input.GetVarName().front()[iHist] : "" ),
		input.CreateVectorOptions()
		);
    }


    if ( outFile ) {
      outFile->Close("R");
      delete outFile;
    }


    TFile *dumFile=0;
    if ( atoi(input.GetOption("inputType").c_str()) == 1 ) dumFile = new TFile( "/sps/atlas/c/cgoudet/Calibration/Test/dumFile.cxx", "recreate" );

    //cleaning vectors of pointers
    while ( vectHist.size() ) {
      while ( vectHist.back().size() ) {
	if ( dumFile ) vectHist.back().back()->Write( "", TObject::kOverwrite );
	if ( vectHist.back().back() ) delete vectHist.back().back();
	vectHist.back().pop_back();
      }
      vectHist.pop_back();
    }
    if ( dumFile ) dumFile->Close();

    while ( vectGraph.size() ) {
      while ( vectGraph.back().size() ) {
	if ( vectGraph.back().back() ) delete vectGraph.back().back();
	vectGraph.back().pop_back();
      }
      vectGraph.pop_back();
    }
    
  }// end iFile
  cout << "All good" << endl;
  return 0;  
}

//===============================================


