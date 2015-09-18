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
#include <TROOT.h>
using std::string;
using std::cout;
using std::endl;
using std::vector;

#include "PlotFunctions/DrawPlot.h"
#include <boost/program_options.hpp>
#include <boost/multi_array.hpp>
using boost::multi_array;
using boost::extents;
namespace po = boost::program_options;


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
  string plotPath = "/afs/in2p3.fr/home/c/cgoudet/private/Codes/PlotFunctions/Plots/";

  for ( unsigned int iFile = 0; iFile < inFiles.size(); iFile++ ) {
    cout << "iFile : " << iFile << " " << inFiles[iFile] << endl;
    InputCompare input( inFiles[iFile] );
    vector< vector< TH1* > > vectHist;
    multi_array< double, 2 > eventVarVect;
    multi_array< long long int, 2> eventIDVect;
    
    vector< vector< string > > inputRootFile = input.GetRootFileName();
    vector< vector< string > > inputObjName = input.GetObjName();

    for ( unsigned int iPlot = 0; iPlot < inputRootFile.size(); iPlot++ ) {
      for ( unsigned int iAdd = 0; iAdd < inputRootFile[iPlot].size(); iAdd ++ ) {
	TFile inFile( inputRootFile[iPlot][iAdd].c_str() );	

	switch( input.GetInputType() ) {
	case 0 : //histograms
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

	case 1 : {//TTree plotting
	  vector< vector<string> > varName = input.GetVarName();
	  vector< double > varMin  = input.GetVarMin();
	  vector< double > varMax  = input.GetVarMax();
	  vector< double > varVal( varName[iPlot].size(), 0 );
	  cout << inFile.GetName() << endl;
	  TTree *inTree = (TTree*) inFile.Get( inputObjName[iPlot][iAdd].c_str() );
	  inTree->SetDirectory( 0 );
	  cout << "inTree : " << inTree << endl;

	  if ( input.GetSelectionCut().size() ){
	    TTree *dumTree = inTree;
	    inTree = dumTree->CopyTree( input.GetSelectionCut()[iPlot].c_str() );
	    inTree->SetDirectory(0);
	    delete dumTree;
	  }
	  unsigned int nEntries = (unsigned int) inTree->GetEntries();
	  
	  for ( unsigned int iEvent = 0; iEvent < nEntries; iEvent++ ) {
	    for ( unsigned int iHist = 0; iHist < varName[iPlot].size(); iHist++ ) {

	      if ( !iEvent ) {
		//Link tree branches to local variables
		inTree->SetBranchAddress( varName[iPlot][iHist].c_str(), &varVal[iHist] );
		if ( !iPlot && !iAdd )  vectHist.push_back( vector<TH1*>() );
		if ( !iAdd )  vectHist[iHist].push_back( 0 );
	      }

	      //Read the tree entry. As we run over all plotted variables, the entry need not to be read several times
	      if ( !iHist ) inTree->GetEntry( iEvent );

	      if ( !vectHist[iHist][iPlot] ) {
		//Create correspondig histogram
		string dumName = string( TString::Format( "%s_%s_%d", input.GetObjName()[iPlot][iAdd].c_str(), varName[iPlot][iHist].c_str(), iPlot ) );
		vectHist[iHist][iPlot] = new TH1D( dumName.c_str(), dumName.c_str(), 100, varMin[iHist], varMax[iHist] );
		vectHist[iHist][iPlot]->GetXaxis()->SetTitle( varName[iPlot][iHist].c_str() );
		vectHist[iHist][iPlot]->GetYaxis()->SetTitle( TString::Format( "# Events / %2.2f", (varMax[iHist]-varMin[iHist])/vectHist[iHist][iPlot]->GetNbinsX()) );
		vectHist[iHist][iPlot]->SetDirectory( 0 );
	      }
	      //if created fill it
	      else vectHist[iHist][iPlot]->Fill( varVal[iHist] );
	    }// end iHist
	  }// end iEvent

	  delete inTree;
	  break;
	}//end case TTree

	case 2 : { //event/event comparison
	  cout << "case2" << endl;
	  TTree *inTree = (TTree*) inFile.Get( inputObjName[iPlot][iAdd].c_str() );
	  if ( input.GetSelectionCut().size() ){
	    TTree *dumTree = inTree;
	    gROOT->cd();
	    inTree = dumTree->CopyTree( input.GetSelectionCut()[iPlot].c_str() );
	    inTree->SetDirectory(0);
	    delete dumTree;
	  }

	  vector< string > eventID = input.GetEventID();
	  vector< long long int > eventIDVal( eventID.size(), 0 );
	  for ( unsigned int i = 0; i<eventID.size(); i++ ) inTree->SetBranchAddress( eventID[i].c_str(), &eventIDVal[i] );

	  vector< vector<string> > eventVar = input.GetVarName();
	  vector< double > eventVarVal(eventVar[iPlot].size(), 0 );
	  for ( unsigned int i = 0; i<eventVar[iPlot].size(); i++ ) inTree->SetBranchAddress( eventVar[iPlot][i].c_str(), &eventVarVal[i] );
	  //resize the 2D vector
	  if ( !iPlot && !iAdd ) {
	    eventVarVect.resize( extents[eventVar[iPlot].size()*inputRootFile.size()][0] );
	    eventIDVect.resize( extents[eventID.size()][0] );
	  }

	  unsigned int nComparedEvents = input.GetNComparedEvents();
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
	    }
	  }

	  //Run over all events
	  for ( unsigned int iEvent = 0; iEvent < nEntries; iEvent++ ) {	  
	    
	    if ( !iPlot && nComparedEvents && eventVarVect[0].size() >= nComparedEvents ) break;
	    inTree->GetEntry( iEvent );
	    //	    cout << iEvent << " " << eventVarVect[0].size() << endl;
	      
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
	  
	default : 
	  cout << "inputType=" << input.GetInputType() << " is not known." << endl;
	  exit(0);
	}//end switch inputType
	inFile.Close("R");	
      }// end iAdd
    }//end iPlot

    for ( unsigned int iHist = 0; iHist < vectHist.size(); iHist++ ) {
      cout << "drawing : " << iHist << endl;
      DrawPlot( vectHist[iHist], 
		plotPath + input.GetOutName()+ ( input.GetVarName().size() && input.GetVarName().front().size() ? "_" + input.GetVarName().front()[iHist] : "" ),
		input.GetLegend(),
		input.GetDoRatio(),
		input.GetNormalize(),
		input.GetDoChi2(),
		input.GetCenterZoom(),
		input.GetRangeUser(),
		input.GetLegendPos(),
		input.GetLatex(),
		input.GetLatexOpt(),
		input.GetDrawStyle()
		);
      cout << "drawn : " << iHist << endl;
    }

    if ( input.GetInputType() == 2 ) { //print csvFile
      fstream csvStream;
      cout << string( plotPath+input.GetOutName() + ".csv" ).c_str() << endl;
      csvStream.open( string( plotPath+input.GetOutName() + ".csv" ).c_str(), fstream::out | fstream::trunc );
      vector< string > eventID = input.GetEventID();
      vector< vector<string> > eventVar = input.GetVarName();      
      for ( unsigned int iVar = 0; iVar < eventIDVect.size(); iVar++ ) csvStream << eventID[iVar] << ",";
      for ( unsigned int iVar = 0; iVar < eventVar.front().size()*eventVar.size(); iVar++ ) csvStream << eventVar[iVar%inputRootFile.size()][iVar/inputRootFile.size()] << ",";
      csvStream << endl;
      for ( unsigned int iEvent = 0; iEvent< eventIDVect[0].size(); iEvent++ ) {
	for ( unsigned int iVar = 0; iVar < eventIDVect.size(); iVar++ ) csvStream << eventIDVect[iVar][iEvent] << ",";
	for ( unsigned int iVar = 0; iVar < eventVar.front().size()*eventVar.size(); iVar++ ) csvStream << eventVarVect[iVar%inputRootFile.size()][iVar/inputRootFile.size()] << ",";
	csvStream << endl;
      }
      csvStream.close();
    }

    //cleaning vectors of pointers
    for ( unsigned int i = 0; i < vectHist.size(); i++ ) {
      for ( unsigned int j = 0; j < vectHist[i].size(); j++ ) {
	if ( vectHist[i][j] ) delete vectHist[i][j];
	vectHist[i].pop_back();
      }
      vectHist.pop_back();
    }
    
  }// end iFile
  return 0;  
}
