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
  string plotPath = "/sps/atlas/c/cgoudet/Plots/";

  for ( unsigned int iFile = 0; iFile < inFiles.size(); iFile++ ) {
    cout << "iFile : " << iFile << " " << inFiles[iFile] << endl;
    InputCompare input( inFiles[iFile] );
    cout << "config file loaded" << endl;
    vector< vector< TH1* > > vectHist;
    multi_array< double, 2 > eventVarVect;
    multi_array< long long int, 2> eventIDVect;
    TFile *outFile = 0;    
    vector< vector< string > > inputRootFile = input.GetRootFileName();
    vector< vector< string > > inputObjName = input.GetObjName();
    TTree *treePassSel=0, *treeRejSel=0;

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
	    cout << "setName" << endl;
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
	  vector< string > varWeight = input.GetVarWeight();
	  if ( varMin.size() != varMax.size() || varName.size()<=iPlot || ( varName.size()>iPlot && varName[iPlot].size() != varMin.size() ) ) {
	    cout << "varMin and varMax sizes matching : " << varMin.size() << " " << varMax.size() << endl;
	    cout << "varName size and iPlot : " << varName.size() << " " << iPlot << endl;
	    if ( varName.size()>iPlot ) cout << "varName[iPlot].size() : "  << varName[iPlot].size() << endl;
	    return 1;
	  }
	  if ( !varWeight.size() ) varWeight = vector<string>( varName.size(), "X" );
	  double weight = 1;

	  TTree *inTree = (TTree*) inFile.Get( inputObjName[iPlot][iAdd].c_str() );
	  if ( !inTree ) {
	    cout << inputObjName[iPlot][iAdd] << " does not exist in " << inFile.GetName() << endl;
	    exit(0);
	  }
	  cout << inTree->GetName() <<  endl;
	  inTree->SetDirectory( 0 );

	  if ( input.GetSelectionCut().size() ){
	    TTree *dumTree = inTree;
	    inTree = dumTree->CopyTree( input.GetSelectionCut()[iPlot].c_str() );
	    inTree->SetDirectory(0);
	    delete dumTree;
	  }
	  unsigned int nEntries = (unsigned int) inTree->GetEntries();
	  if ( varWeight[iPlot] != "X" ) inTree->SetBranchAddress( varWeight[iPlot].c_str(), &weight );
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
		vectHist[iHist][iPlot]->Sumw2();
	      }
	      //if created fill it
	      else vectHist[iHist][iPlot]->Fill( varVal[iHist], weight );
	    }// end iHist
	  }// end iEvent

	  delete inTree;
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
	  cout << "case 3 :" << endl;
	  cout << "iPlot : " << iPlot << endl;
	  cout << "iAdd : " << iAdd << endl;
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
	  vector< string > varWeight = input.GetVarWeight();
	  unsigned int weightIndex = SearchVectorBin( varWeight[iPlot], titleVect );

	  while ( true ) {
	    for ( unsigned int iTxtVar = 0; iTxtVar < titleVect.size(); iTxtVar++ ) {
	    inputStream >> varVal[iTxtVar];
	    }
	    if ( inputStream.eof() ) break;
	  double weight = ( varWeight[iPlot] != "X" ) ? varVal[weightIndex] : 1;
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
		input.GetDrawStyle(),
		input.GetShiftColor()
		);
      cout << "drawn : " << iHist << endl;
    }


    if ( input.GetInputType() == 2 ) { //print csvFile
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

    if ( outFile ) {
      outFile->Close("R");
      delete outFile;
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
  cout << "All good" << endl;
  return 0;  
}

//===============================================


