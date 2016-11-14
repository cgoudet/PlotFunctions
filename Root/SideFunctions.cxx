#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/Foncteurs.h"

#include <TROOT.h>
#include "TRandom.h"
#include "TClass.h"
#include "TKey.h"
#include "TFile.h"
#include "THStack.h"
#include "TObject.h"
#include "TArrayD.h"
#include "RooWorkspace.h"
#include "RooArgSet.h"
#include "TIterator.h"
#include "RooRealVar.h"
#include <RooStats/ModelConfig.h>
#include "RooProduct.h"
#include "TXMLAttr.h"
#include "TList.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TObjArray.h"
#include "TArrayD.h"

#include <iostream>
#include <fstream>
#include "time.h"
#include <chrono>
#include <memory>
#include <set>
#include <algorithm> 
#include <stdio.h>
#include <string>
#include <iterator>

#define DEBUG 1
using namespace std::chrono;
using namespace std;
using RooStats::ModelConfig;
using boost::multi_array;
using boost::extents;
using namespace ChrisLib;
using std::swap;

//=====================================
/**\brief Create a name from levels of components
 */
void ChrisLib::CombineNames( const list<list<string>> &components, list<string> &outNames, string separator ) {
  list<string> tmpVect;
  outNames = list<string>(1,"");
  for( auto vLevel : components ) {
    tmpVect = outNames;
    outNames.clear();
    for( auto vCurrentName : tmpVect ) {
      for ( auto vString : vLevel ) {
	string name  = vCurrentName == "" ? vString : vCurrentName + separator + vString;
  	outNames.push_back( name );
      }
    }
  }
}

//============================================
unsigned int ChrisLib::GetLinearCoord( const vector<unsigned int> &levelsSize, const vector<unsigned int> &objCoords ) {
  //coords from the least to most nested
  if ( !levelsSize.size() && !objCoords.size() ) throw invalid_argument( "GetLinearCoord : vectors sizes are 0." );
  if ( levelsSize.size() != objCoords.size() ) throw invalid_argument( "GetLinearCoord : vectors sizes do not match." );

  unsigned int index = 0;
  unsigned int indexByStep = 1;
  for ( int iObjCoords = static_cast<int>(objCoords.size())-1; iObjCoords>=0; --iObjCoords ) {
    if ( objCoords[iObjCoords] >= levelsSize[iObjCoords] ) throw runtime_error( "GetLinearCoord : Coordinate above dimension size." );
    index += indexByStep*objCoords[iObjCoords];
    indexByStep *= levelsSize[iObjCoords];
  }
  return index;
}

  //==================================
void ChrisLib::GetCoordFromLinear( const vector<unsigned> &levelsSize, const unsigned objIndex, vector<unsigned> &coords ) {
  //coords from the least to most nested
  coords.clear();
  unsigned subLevelSize=1;
  
  for ( auto itLevel = levelsSize.rbegin(); itLevel!=levelsSize.rend(); ++itLevel ) {
    unsigned int dumSubLevelSize = subLevelSize;
    subLevelSize *= *itLevel;
    coords.push_back( (objIndex%subLevelSize)/dumSubLevelSize );
  }

  reverse( coords.begin(), coords.end() );
  if ( objIndex > subLevelSize ) throw runtime_error( "GetCoordFromLinear : Asked index above maximum." );

}

//==============================================================
string ChrisLib::PrintWorkspaceCorrelationModel(string inFileName, string outFileName, vector<vector<string>> inConfigurationsName, string NPPrefix, string inWSName, string inMCName ) {

  //Get the model config out of the fileName
  TFile *inFile = new TFile( inFileName.c_str() );
  if ( !inFile ) { cout << inFileName << " not found." << endl; exit(0); }
  if ( inWSName == "" ) inWSName = FindDefaultTree( inFile, "RooWorkspace" );
  RooWorkspace *ws = (RooWorkspace*) inFile->Get( inWSName.c_str() );
  if ( !ws ) { cout << "Workspace not found in " << inFileName << endl; exit(0); }
  ModelConfig* const mc = (ModelConfig*) ws->obj( inMCName.c_str() );

  //Get the names of all nuisance paramters
  vector<string> NPName;
  TIterator* iter = mc->GetNuisanceParameters()->createIterator();
  while ( RooRealVar* v = (RooRealVar* ) iter->Next() ) {
    if ( string(v->GetName()).find_first_of("ATLAS_") == 0 ) NPName.push_back( v->GetName() );
  }
  sort( NPName.begin(), NPName.end() );

  vector<unsigned int> configurationsDepth;
  GetLevelsSize( inConfigurationsName, configurationsDepth );

  unsigned int nConfig = GetNConfigurations( inConfigurationsName );
  cout << "nConfig : " << nConfig << endl;

  multi_array<bool, 2> correlations;
  correlations.resize( extents[nConfig][NPName.size()] );
  vector<string> configName;

  vector<unsigned int> iConfToSkip;

  for ( unsigned int iConf = 0; iConf< nConfig; ++iConf ) {
    vector<unsigned> coords;
    GetCoordFromLinear( configurationsDepth, iConf, coords );
    string name = "";
    for ( unsigned int iList = 0; iList<coords.size(); ++iList ) {
      name += inConfigurationsName[iList][coords[iList]] + "_";
    }
    configName.push_back( RemoveSeparator( name ) );
    RooProduct *var = (RooProduct*) ws->function( name.c_str() );
    if ( !var ) { 
      cout << name << " not found in " << ws->GetName() << endl; 
      iConfToSkip.push_back( iConf );
      continue;
    }

    list<string> removeList;
    copy( inConfigurationsName.front().begin(), inConfigurationsName.front().end(), back_inserter(removeList) );
    string nameWoVariable = RemoveSeparator( RemoveWords( name, removeList ) );

    iter = var->getComponents()->createIterator();
    while ( RooRealVar* v = (RooRealVar* ) iter->Next() ) {
      TString dumName = v->GetName();
      if ( !dumName.Contains( NPPrefix.c_str() ) ) continue;
      dumName.ReplaceAll( (NPPrefix+"_").c_str(), "" );
      unsigned int nStep = 0;
      unsigned int NPPos = NPName.size();
      vector<string>::iterator pos = find( NPName.begin(), NPName.end(), string(dumName) );
      //      while ( NPPos == NPName.size() && ++nStep!=5 ) {
      while ( pos == NPName.end() && ++nStep!=5 ) {
	if ( nStep==2 ) dumName.ReplaceAll( nameWoVariable, "" );
	else if ( nStep ==3 ) dumName.Resize( dumName.Last('_') );
	else if ( nStep ==4 ) dumName+="_"+nameWoVariable;
	dumName = RemoveSeparator(string(dumName));
	pos = find( NPName.begin(), NPName.end(), string(dumName) );
	//NPPos = SearchVectorBin( string(dumName), NPName );
	//	if ( dumName.Contains( "ATLAS_pdf_acc" ) ) cout << dumName << " " << NPPos << " " << NPName.size() << endl;
      }
      
      if ( pos == NPName.end() ) { 
	cout << dumName << " not found" << endl;
	continue;
      }
      else correlations[iConf][NPPos]=1;
    }
  }

  reverse( iConfToSkip.begin(), iConfToSkip.end() );
  vector<unsigned int> dumConfToSkip = iConfToSkip;
  PrintVector( dumConfToSkip );
  cout << dumConfToSkip.size() << endl;
  fstream stream;
  if ( outFileName == "" ) outFileName = "/sps/atlas/c/cgoudet/Plots/CorrelationModel.csv";
  cout << "Writing in : " << outFileName << endl;
  stream.open( outFileName.c_str(), fstream::out | fstream::trunc );
  for ( unsigned int iCat=0; iCat<correlations.size(); ++iCat ) {
    if ( dumConfToSkip.size() && iCat == dumConfToSkip.back() ) {
      cout << "remove " << dumConfToSkip.back() << " " << configName[iCat]<< endl;
      dumConfToSkip.pop_back();
      continue;
      }
    stream << "," << configName[iCat];
  }
  stream << endl;

  for ( unsigned int iNPName = 0; iNPName< NPName.size(); ++iNPName ) {
    stream << NPName[iNPName];
    dumConfToSkip = iConfToSkip;
    for ( unsigned int iCat=0; iCat<correlations.size(); ++iCat ) {
      if ( iCat == dumConfToSkip.back() ) {
	dumConfToSkip.pop_back();
	continue;
      }
      stream << "," << (correlations[iCat][iNPName] ? "1" : "0");
    }
    stream << endl;
  }
  stream.close();
  
  return outFileName;


  }

//==============================================================
string ChrisLib::PrintWorkspaceVariables( string inFileName, string outFileName, vector<string> inFunctionsName, string inWSName ) {
  
  TFile *inFile = new TFile( inFileName.c_str() );
  if ( !inFile ) { cout << inFileName << " not found." << endl; exit(0); }

  if ( inWSName == "" ) inWSName = FindDefaultTree( inFile, "RooWorkspace" );
  RooWorkspace *ws = (RooWorkspace*) inFile->Get( inWSName.c_str() );
  if ( !ws ) { cout << "Workspace not found in " << inFileName << endl; exit(0); }
  
  RooArgSet allVars = ws->allVars();
  allVars.sort();
  
  fstream stream;
  if ( outFileName == "" ) outFileName = "/sps/atlas/c/cgoudet/Plots/PrintWorkspaceVariables.csv";
  cout << "Writing in : " << outFileName << endl;
  stream.open( outFileName.c_str(), fstream::out | fstream::trunc );
  
  TIterator* iter = allVars.createIterator();
  while ( RooRealVar* v = (RooRealVar* ) iter->Next() ) {
    stream << v->GetName() << "," << v->getVal() << "," << v->getError() << endl;
  }
  
  for ( auto vName : inFunctionsName ) {
    RooAbsReal *var = ws->function( vName.c_str() );
    if ( var ) stream << vName << "," << var->getVal() << ",0"  << endl;
    else cout << vName << " not found in " << ws->GetName() << endl;
  }
  stream.close();
  
  return outFileName;
}

//==============================================================
void ChrisLib::RebinHist( vector<TH1*> &vectHist ) {

  for ( auto itHist = vectHist.begin(); itHist!=vectHist.end(); ++itHist ) {
    if ( *itHist  ) continue;
    vectHist.erase( itHist );
    --itHist;
  }
  if ( vectHist.empty() ) return;
  vector<double> axisLimits;//axisLimits;
  for ( auto itHist = vectHist.begin(); itHist!=vectHist.end(); ++itHist ) {
    auto array  = (*itHist)->GetXaxis()->GetXbins();
    if ( array->GetSize() ) {
      for ( int iBin = 0; iBin < array->GetSize(); iBin++ ) 
	axisLimits.push_back( array->At( iBin ) );
    }
    else FillDefaultFrontiers( axisLimits, (*itHist)->GetNbinsX(), (*itHist)->GetXaxis()->GetXmin(), (*itHist)->GetXaxis()->GetXmax() );
  }

  sort( axisLimits.begin(), axisLimits.end() );
  axisLimits.erase( unique( axisLimits.begin(), axisLimits.end() ), axisLimits.end() );

  for ( unsigned int iHist = 0; iHist < vectHist.size(); ++iHist ) {
    if ( static_cast<int>(axisLimits.size())-1 == vectHist[iHist]->GetNbinsX() ) continue;
    TH1* dumHist = vectHist[iHist];
    TString dumName = dumHist->GetName();
    vectHist[iHist] = new TH1D( dumName+"_dum", dumName+"_dum", axisLimits.size()-1, &axisLimits[0] );
    vectHist[iHist]->GetXaxis()->SetTitle( dumHist->GetXaxis()->GetTitle() );
    vectHist[iHist]->GetYaxis()->SetTitle( dumHist->GetYaxis()->GetTitle() );

    for ( int iBin =1; iBin <= vectHist[iHist]->GetNbinsX(); ++iBin ) {
      double centralValueBin = vectHist[iHist]->GetXaxis()->GetBinCenter( iBin );
      vectHist[iHist]->SetBinContent( iBin, dumHist->GetBinContent( dumHist->FindFixBin( centralValueBin ) ) );
      vectHist[iHist]->SetBinError( iBin, 0 );
    }
    delete dumHist; dumHist=0;
    vectHist[iHist]->SetName( dumName );
  }
}
//==============================================================

double ChrisLib::ComputeChi2( TH1 *MCHist, TH1 *DataHist ) {

  if ( !ComparableHists( MCHist, DataHist ) ) throw invalid_argument( "ComputeChi2 : Not comparable histograms." );

  double chi2 = 0;
  for ( int i = 1; i < MCHist->GetNbinsX()+1; ++i ) {
    double valdif = ( MCHist->GetBinError( i )==0 && DataHist->GetBinError( i )==0 ) ? 0 : MCHist->GetBinContent( i ) - DataHist->GetBinContent( i );
    //    cout << "valdif : " << valdif << " = " << MCHist->GetBinContent( i ) << " - " << DataHist->GetBinContent( i ) << endl;
    double sigma2 = ( valdif != 0 ) ? MCHist->GetBinError( i ) * MCHist->GetBinError( i ) + DataHist->GetBinError( i ) * DataHist->GetBinError( i ) : 1;
    //    cout << "sigma2 : " << sigma2 << " :  " << MCHist->GetBinError( i ) << " " << DataHist->GetBinError( i ) << endl;
    chi2 += valdif * valdif / sigma2; 
  }
    
  return chi2;
}

//=======================================================
/**\brief Find the range to fit between +/- 3sigma
   \return 0 OK

Lower and upper bins are put into binMin and binMax variables
*/
int ChrisLib::FindFitBestRange( TH1D *hist, int &binMin, int &binMax, double chiMinLow, double chiMinUp ) {


  double min = hist->GetMinimum();
  int centralBin = hist->GetMinimumBin();

  if ( chiMinLow == 0 ) binMin = 1; 
  else { 
  binMin = centralBin;
  while ( binMin > 1 && hist->GetBinContent( binMin ) - min < chiMinLow ) binMin--;
  }

  if ( chiMinUp == 0 ) binMax = hist->GetNbinsX();
  else {
    binMax = centralBin;
    while ( binMax < hist->GetNbinsX()  && hist->GetBinContent( binMax ) - min < chiMinUp ) binMax++;
  }

  return 0;
}

//=============================================

//=============================================
void ChrisLib::WriteLatexHeader( fstream &latexStream, string title, string author ) {

  latexStream << "\\documentclass[a4paper,12pt]{article}" << endl;
  latexStream << "\\usepackage{graphicx}" << endl;
  latexStream << "\\usepackage{csvsimple}" << endl;
  latexStream << "\\usepackage{adjustbox}" << endl;
  latexStream << "\\usepackage{xcolor}" << endl;
  latexStream << "\\usepackage[a4paper, textwidth=0.9\\paperwidth, textheight=0.9\\paperheight]{geometry}" << endl;
  latexStream << "\\usepackage[toc]{multitoc}" << endl;
  latexStream << "\\title{" << title << "}" << endl;
  latexStream << "\\author{" << author << "}" << endl;
  latexStream << "\\date{\\today}" <<endl;
  latexStream << "\\begin{document}\\maketitle" << endl;

}
  //======================================
string ChrisLib::StripString( const string &inString, bool doPrefix, bool doSuffix ) {
  string dumString = inString;
  if ( doPrefix ) dumString = dumString.substr( dumString.find_last_of( "/" )+1 );
  if ( doSuffix ) dumString = dumString.substr( 0, dumString.find_last_of( "." ) );
  return dumString;
}

//=======================================
void ChrisLib::RemoveExtremalEmptyBins( TH1 *hist ) {

  int lowBin = 1, upBin = hist->GetNbinsX();
  while ( hist->GetBinContent( lowBin ) == 0 ) lowBin++;
  while ( hist->GetBinContent( upBin ) == 0 ) upBin--;

  hist->GetXaxis()->SetRangeUser( hist->GetXaxis()->GetBinLowEdge( lowBin ), hist->GetXaxis()->GetBinUpEdge( upBin ) );
}

//========================================================
void ChrisLib::ParseLegend( TGraphErrors *graph, string &legend ) {
  TString dumString = legend;
  if ( graph ) { 
    dumString.ReplaceAll( "__ENTRIES", TString::Format( "%1.0d", graph->GetN() ) );
    dumString.ReplaceAll( "__MEAN", TString::Format( "%1.3e", graph->GetMean() ) );
    dumString.ReplaceAll( "__STDEV", TString::Format( "%1.3e", graph->GetRMS() ) );
  }

  legend = dumString;
  ParseLegend( legend );
}
//================================
void ChrisLib::ParseLegend( string &legend ) {
  TString dumString = legend;
  dumString.ReplaceAll( "__HASHTAG", "#" );
  dumString.ReplaceAll( "__FILL", "" );
  dumString.ReplaceAll("__NOPOINT", "" );
  dumString.ReplaceAll("__ATLAS", "" );
  dumString.ReplaceAll("__STACK", "" );
  dumString.ReplaceAll("ETA_CALO", "#eta_{CALO}" );
  legend = dumString;
}

//============================
void ChrisLib::ParseLegend( TH1* hist, string &legend ) {

  TString dumString = legend;
  if ( hist ) { 
    dumString.ReplaceAll( "__ENTRIES", TString::Format( "%1.0f", hist->GetEntries() ) );
    dumString.ReplaceAll( "__MEAN", TString::Format( "%1.3e", hist->GetMean() ) );
    dumString.ReplaceAll( "__STDEV", TString::Format( "%1.3e", hist->GetStdDev() ) );
    dumString.ReplaceAll( "__INTEGRAL", TString::Format( "%1.3e", hist->GetSumOfWeights() ) );
  }

  legend = dumString;
  ParseLegend( legend );
}

//============================================
TTree* ChrisLib::Bootstrap( vector< TTree* > inTrees, unsigned int nEvents, unsigned long int seed, int mode ) {
  cout << "Bootstrap" << endl;
  string outTreeName = inTrees.front()->GetName() + string( "_bootstrap" );
  TTree * outTree = new TTree ( outTreeName.c_str(), outTreeName.c_str() );
  outTree->SetDirectory(0);
  TRandom rand;
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  if ( seed ) {
    if ( seed == 1 ) seed = t1.time_since_epoch().count();
    rand.SetSeed( seed );
  }

  unsigned int totEntry = 0;
  for ( unsigned int iTree = 0; iTree < inTrees.size(); iTree++ ) totEntry += inTrees[iTree]->GetEntries();
  if ( !nEvents ) nEvents = totEntry;

  cout << "nEvents : " << nEvents << endl;

  vector<unsigned int > totEntriesIndex, selectedEventsIndex;
  for ( unsigned int i =0; i<totEntry; i++ ) totEntriesIndex.push_back( i );

  // TH1D *histTest =new TH1D("bootstrap", "", totEntry, 0, totEntry); 
  // TFile *outRootFile = new TFile ("bootstrapFile.root", "RECREATE");

  // cout<<"Create bootstrap histo, nEvents: "<<nEvents<<endl;

  for ( unsigned int iEvent=0; iEvent<nEvents; iEvent++ ) {
    unsigned int xEntry = floor( rand.Uniform( totEntriesIndex.size() ) );
    selectedEventsIndex.push_back( totEntriesIndex[xEntry] );

    if ( mode == 1 ) {
      totEntriesIndex[xEntry] = totEntriesIndex.back();
      totEntriesIndex.pop_back();
    }
   
    // histTest->Fill(totEntriesIndex[xEntry]);

  }
  sort( selectedEventsIndex.begin(), selectedEventsIndex.end() );
  reverse( selectedEventsIndex.begin(), selectedEventsIndex.end() );

  //histTest->Write();
  //outRootFile->Close();
  //delete outRootFile;

  MapBranches mapB;
  unsigned int counter=0;
  for ( auto vTree : inTrees ) {
    mapB.LinkTreeBranches( vTree, outTree );
    unsigned int nEntries = vTree->GetEntries();
    for ( unsigned int iEvent = 0; iEvent < nEntries; iEvent++ ) {
      if ( !selectedEventsIndex.size() ) break;
      vTree->GetEntry( iEvent );
      while ( selectedEventsIndex.back() < counter ) selectedEventsIndex.pop_back();
      while ( selectedEventsIndex.back() == counter ) {
	outTree->Fill();
	selectedEventsIndex.pop_back();
      }

      counter ++;
    }
  }
  return outTree;

}

//================================================
string ChrisLib::FindDefaultTree( const TFile* inFile, string type, string keyWord  ) { 
  if ( !inFile ) throw invalid_argument( "FindDefaultTree : Null inFile " );
  if ( type == "" ) type = "TTRee";

  string inFileName = inFile->GetName();
  StripString( inFileName );

  list<string> listTreeNames;
  
  TIter nextkey( inFile->GetListOfKeys());
  TKey *key=0;
  while ((key = (TKey*)nextkey())) {
    if (strcmp( type.c_str(),key->GetClassName())) continue;//Check wether the class name of the object is what is looked for
    listTreeNames.push_back( key->GetName() );
  }
  delete key; key=0;
  
  if ( !listTreeNames.size() ) throw runtime_error( "FindDefaultTree : No object of type " + type + " found." );
  else if ( listTreeNames.size() == 1 || keyWord == "" ) return *listTreeNames.begin();
  else {
    for ( auto it = listTreeNames.begin(); it!=listTreeNames.end(); ++it ) {
      if ( it->find(keyWord) != string::npos ) return *it;
    }
  }
  throw runtime_error( "FindDefaultTree : No object found of type " + type + " with name containing " + keyWord );
}

//===========================================
void ChrisLib::AddTree( TTree *treeAdd, TTree *treeAdded ) {
  TList *list = new TList();
  list->Add( treeAdd );
  list->Add( treeAdded );
  treeAdd->Merge( list );

}

//===================================
void ChrisLib::SaveTree( TTree *inTree, string prefix) {
  prefix += string( inTree->GetName()) + ".root";
  TFile *dumFile = new TFile( prefix.c_str(), "RECREATE" );
  inTree->Write();
  dumFile->Close("R");
  delete dumFile;
  delete inTree; inTree=0;
}

//================================
void ChrisLib::DiffSystematics( string inFileName, bool update ) {

  TFile *outFile = 0;
  TH1D *totSyst = 0;
  TH1D* baseValue = 0;
  unsigned int mode = 0;
  //Variables to create uncertainties
  fstream inStream;
  inStream.open( inFileName, fstream::in );
  if ( !inStream.is_open() ) {
    cout << inFileName << " does not exist." << endl;
    exit(0);
  }
  
  string rootFileName, histName, systName, outSystName;
  unsigned int counterSyst=0;
  string other;
  while ( inStream >> rootFileName >> histName >> systName >> mode ) {
    //    cout << rootFileName << endl;
    if ( TString( rootFileName).Contains("#") ) continue;
    if ( !outFile ) {
      outFile = new TFile( rootFileName.c_str(), update ? "UPDATE" : "RECREATE" );
      outSystName = histName;
      totSyst = (TH1D*) outFile->Get( histName.c_str() );
      continue; //counterSyst remains at 0 
    }
    unsigned int tempMode = mode;

    TFile *inFile = new TFile( rootFileName.c_str() );
    if ( !inFile ) { cout << rootFileName << " does not exist." << endl; exit(0); }
    inFile->cd();


    if ( !counterSyst && ( tempMode/10==0 || tempMode/10==2 ) ) {
	//Get nominal Value
	baseValue = (TH1D*) inFile->Get( histName.c_str() )->Clone();
	baseValue->SetDirectory(0);
	outFile->cd();
	if ( systName != "dum" ) baseValue->Write( systName.c_str(), TObject::kOverwrite );
       	if ( DEBUG ) cout << "baseValue defined : " << baseValue << endl;	
      }
    else {
      TH1D* inHist = (TH1D*) inFile->Get( histName.c_str() );
      if ( !inHist ) { cout << histName << " does not exist within " << inFile->GetName() << endl; exit(0); }
      //If stat appears in the name, use the error bars as the systematic
      if ( TString( systName ).Contains( "__STAT" ) ) {
	systName = string(TString( systName ).ReplaceAll( "__STAT", "" ));
	for ( int iBin=1; iBin<=inHist->GetNbinsX(); iBin++ ) inHist->SetBinContent( iBin, inHist->GetBinError( iBin ) );
	tempMode = 10 + ( mode % 10 );
      }


      systName = "syst_" + systName;
      inHist->SetName( systName.c_str() );
      inHist->SetTitle( inHist->GetName() );
      inHist->SetDirectory(0);
      if ( DEBUG ) cout << "inHist found : " << inFile->GetName() << " " << inHist->GetName() << endl;

      //Create the systematic histogram to add
      switch ( tempMode/10 ) {
      case 0 :
	inHist->Add( baseValue, -1 );
	break;
      case 2 : {
	for ( int iBin = 1; iBin<= inHist->GetNbinsX()/2; iBin++ ) {
	  double symDiff = ( inHist->GetBinContent(iBin) + inHist->GetBinContent( inHist->GetNbinsX() - iBin+1 ) - baseValue->GetBinContent(iBin) - baseValue->GetBinContent( baseValue->GetNbinsX() - iBin+1 ) )/2.;
	  //	  if ( DEBUG ) cout << "symDiff " << iBin << " : " << symDiff << " " << inHist->GetBinContent(iBin) << " " << inHist->GetBinContent( inHist->GetNbinsX() - iBin+1 ) << " "  << baseValue->GetBinContent(iBin) << " " << baseValue->GetBinContent( baseValue->GetNbinsX() - iBin+1 ) << endl;
	  inHist->SetBinContent( iBin, symDiff );
	  inHist->SetBinContent( inHist->GetNbinsX() - iBin+1, symDiff );
	}
	break;
      }//end case 2
      }//end switch

     
      for ( int iBin = 1; iBin<inHist->GetNbinsX()+1; iBin++ ) inHist->SetBinError(iBin, 0 );
      outFile->cd();
      //      inHist->Write( "", TObject::kOverwrite );

      if ( DEBUG ) cout << "Add systematic to the model" << endl;
      //in case totSyst does not exists in the output file, create it
      if ( !totSyst ) {
	totSyst = (TH1D*) inHist->Clone();
	for ( int iBin = 1; iBin<totSyst->GetNbinsX()+1; iBin++ ) totSyst->SetBinContent( iBin, 0 );
	totSyst->SetName( outSystName.c_str()) ;
	totSyst->SetTitle( totSyst->GetName() );
	totSyst->GetYaxis()->SetTitle( "#delta" + TString(inHist->GetYaxis()->GetTitle() ) );
      }
      vector<TH1*> hists = { totSyst, inHist };
      RebinHist( hists );
      inHist = (TH1D*) hists[1];
      totSyst = (TH1D*) hists[0];
      if ( !TString( inHist->GetYaxis()->GetTitle() ).Contains("#delta") ) inHist->GetYaxis()->SetTitle( "#delta" + TString( inHist->GetYaxis()->GetTitle() ) );
      inHist->Write( "", TObject::kOverwrite );
      for ( int iBin = 1; iBin<totSyst->GetNbinsX()+1; iBin++ ) {
	switch( tempMode % 10 ) {
	case 1 :
	  totSyst->SetBinContent( iBin,
				  totSyst->GetBinContent( iBin ) + fabs( inHist->GetBinContent( iBin ) ) );
	  break;
	default :
	  totSyst->SetBinContent( iBin,
				  sqrt(totSyst->GetBinContent( iBin )*totSyst->GetBinContent( iBin ) + inHist->GetBinContent( iBin )*inHist->GetBinContent( iBin ) ) );
	  break;

	}//end switch
	totSyst->SetBinError( iBin, 0 );
      }//end for iBin

      delete inHist; inHist=0;
    }//end else

    outFile->cd();
    if ( totSyst ) totSyst->Write( "", TObject::kOverwrite );
    if ( inFile ) delete inFile; inFile =0;
    counterSyst++;
  }//end while
  if ( totSyst ) delete totSyst; totSyst=0;
  if ( baseValue ) delete baseValue; baseValue=0;
  cout << "outFile : " << outFile->GetName() << endl;
  delete outFile;
}//EndDiffSystematic

//===================================================
void ChrisLib::VarOverTime( string inFileName, bool update) {

  fstream inStream;
  inStream.open( inFileName, fstream::in );
  if ( !inStream.is_open() ) {
    cout << inFileName << " does not exist." << endl;
    exit(0);
  }
  string rootFileName, histName, systName, outFileName, outHistName;
  double val, valMin=-99, valMax=-99;
  //  unsigned int counterSyst=0;
  string yaxis;

  vector<double> valVect;
  multi_array<double, 3> scales;

  while ( inStream >> rootFileName >> histName >> val  ) {

    if ( outFileName == "" ) {
      outFileName = rootFileName;
      outHistName = histName;
      continue;
    }

    unsigned int index = SearchVectorBin( val, valVect );
    if ( index == valVect.size() ) valVect.push_back( val );


    if ( valMax == -99 || val > valMax ) valMax = val;
    if ( valMin == -99 || val < valMin ) valMin = val;

    TFile *inFile = new TFile( rootFileName.c_str() );
    if ( !inFile ) {
      cout << rootFileName << " does not exists." << endl;
      exit(0);
    }

    TH1D* inHist = (TH1D*) inFile->Get( histName.c_str() );
    if ( !inHist ) {
      cout << histName << " does not exist within " << rootFileName << endl;
      exit(0);
    }
    yaxis = inHist->GetYaxis()->GetTitle();
    if ( !scales.size() ) scales.resize( extents[inHist->GetNbinsX()][2][0] );
    cout << "sizes : " << scales.size() << " " << inHist->GetNbinsX() << endl;
    cout << inFile->GetName() << " " << inHist->GetName() << endl;
    if ( scales.size() != ( unsigned int ) inHist->GetNbinsX() ) {
      cout << "all input scales are not from same binning" << endl;
      exit(0);
    }
    scales.resize( extents[scales.size()][2][valVect.size()] );

    for ( int iBin = 0; iBin < inHist->GetNbinsX(); iBin++ )  {
      scales[iBin][0][index] = inHist->GetBinContent( iBin+1 );
      scales[iBin][1][index] = inHist->GetBinError( iBin+1 );
    }

    delete inHist;
    delete inFile;
  }// end while

  cout << "outFileName : " << outFileName << " " << outHistName << endl;  
  TFile *outFile = new TFile( outFileName.c_str(), update ? "UPDATE" : "RECREATE" );
  vector< TH1D *> histVect( scales.size(), 0 );
  cout << "scale size : " << scales.size() << endl;
  for ( unsigned int iBin=0; iBin<scales.size(); iBin++ ) {
    if ( ! histVect[iBin] ) {
      string dumName = outHistName + string( TString::Format( "_bin%d", iBin ));
      histVect[iBin] = new TH1D( dumName.c_str(), dumName.c_str(), ceil( valMax ) - floor( valMin ) +1, floor( valMin ) -0.5, ceil( valMax ) +0.5 );
      histVect[iBin]->GetXaxis()->SetTitle( "PT" );
      histVect[iBin]->GetYaxis()->SetTitle( yaxis.c_str() );
    }
    
    for ( unsigned int iPt=0; iPt<scales[iBin][0].size(); iPt++ ) {
      unsigned int bin = histVect[iBin]->FindFixBin( valVect[iPt] );
      histVect[iBin]->SetBinContent( bin, scales[iBin][0][iPt] );
      histVect[iBin]->SetBinError( bin , scales[iBin][1][iPt] );
    }
    histVect[iBin]->Write( "" , TObject::kOverwrite );
  }



  for ( auto hist : histVect ) delete hist;
  delete outFile;
}
//=====================================
void ChrisLib::RescaleStack( THStack *stack, double integral ) {
  TIter iter(stack->GetHists());
  TH1 *hist = 0;
  double totIntegral=0;
  while ( hist ) {
    cout << "integral : " << hist->Integral() << endl;
    totIntegral += hist->Integral();
    hist = (TH1*) iter.Next();
  }
  cout << "totIntegral : " << totIntegral << endl;
  iter.Reset();
  while ( (hist = (TH1*) iter.Next()) ) {
    hist->Scale( 1./totIntegral * integral );
    hist = (TH1*) iter.Next();
  }

}
//=====================================
void ChrisLib::CleanHist( vector<TH1*> &vect, const double removeVal ) {

  for ( auto itHist = vect.begin(); itHist!=vect.end(); ++itHist ) {
    if ( *itHist == 0 ) {
      vect.erase( itHist );
      --itHist;
    }
    if ( !ComparableHists( *vect.begin(), *itHist ) ) throw invalid_argument( "CleanHist : Histograms not comparables." );
  }

  if ( vect.empty() ) return;
  int nBins = vect.front()->GetNbinsX();
  //Check which bins must be kept


  vector<int> keptBins;
  for ( int iBin = 1; iBin<=nBins; ++iBin ) {
    bool keepBin=false;
    for ( auto itHist = vect.begin(); itHist!=vect.end(); ++itHist ) {
      if ( (*itHist)->GetBinContent(iBin) == removeVal ) continue;
      keepBin = true;
      break;
    }
    if ( keepBin ) keptBins.push_back( iBin );     
    }


  if ( keptBins.empty() ) throw runtime_error( "CleanHist : All bins should be removed." );
  else if ( static_cast<int>(keptBins.size()) == nBins ) return;

  //Fill new histograms with old values
  for ( unsigned int iVect = 0; iVect<vect.size(); ++iVect ) {
    string tmpName = vect[iVect]->GetName();
    TH1D *dumVect = new TH1D( "tmp", vect[iVect]->GetTitle(), static_cast<int>(keptBins.size()), 0.5, keptBins.size()+0.5 );
    dumVect->SetDirectory(0);
    
    for ( unsigned int iBin=0; iBin<keptBins.size(); ++iBin ) {
      dumVect->SetBinContent( iBin+1, vect[iVect]->GetBinContent(keptBins[iBin]) );
      dumVect->SetBinError( iBin+1, vect[iVect]->GetBinError(keptBins[iBin]) );
      dumVect->GetXaxis()->SetBinLabel( iBin+1, vect[iVect]->GetXaxis()->GetBinLabel( keptBins[iBin] ) );
    }

    delete vect[iVect];
    vect[iVect] = dumVect;
    vect[iVect]->SetName( tmpName.c_str() );
  }
  
}

//=====================================
string ChrisLib::ConvertEpochToDate ( int epochTime )
{

  string date="void date";
  time_t currentTime = (time_t) epochTime;
  struct tm *currentDate = localtime(&currentTime);
  date=std::to_string(currentDate->tm_mday) + "/" + std::to_string(currentDate->tm_mon+1) + "/" + std::to_string(currentDate->tm_year+1900);
  return date;

}

//=====================================
map<string,string> ChrisLib::MapAttrNode( TXMLNode* node ) {

  map<string,string> outMap;
  outMap["nodeName"]=node->GetNodeName();
  TList *attr = node->GetAttributes();
  TIterator *it = 0;
  if(attr!=0) {
    it = attr->MakeIterator();
    for ( auto attr = (TXMLAttr*) it->Next(); attr!=0; attr=(TXMLAttr*)it->Next() ) {
      outMap[attr->GetName()] = attr->GetValue();
    }
    delete it; it=0;
  }
  return outMap;
}

//=====================================
void ChrisLib::PrintArray( const string &outName, const multi_array<double,2> &array, const vector<string> &linesTitle, const vector<string> &colsTitle ) {
  
   if ( !array.size() || !array[0].size() ) return;
   if ( linesTitle.size() && linesTitle.size() != array.size() ) throw runtime_error("PrintArray : Not enough names for lines.");
   ReplaceString repStr( "_", "\\_" );
   ReplaceString repSpace( "_", " " );

   unsigned nCols = array[0].size();
   if ( linesTitle.size() ) ++nCols;
   if ( colsTitle.size() && colsTitle.size() != nCols ) throw runtime_error("PrintArray : Not enough names for columns.");
   
   cout << "writting in : " << outName << endl;
   fstream stream( outName.c_str(), fstream::out );
   for ( unsigned iLine = 0; iLine<array.size(); ++iLine ) {
     if ( !iLine && colsTitle.size() ) {
       transform( colsTitle.begin(), --colsTitle.end(), ostream_iterator<string>( stream, "," ), repSpace );
       stream << repSpace(colsTitle.back());
       stream << endl;
     }
     for ( unsigned iCol = 0; iCol<array[0].size(); ++iCol ) {
       if ( !iCol && linesTitle.size() ) stream << repStr(linesTitle[iLine]) << ",";
       if ( iCol ) stream << ",";
       stream << array[iLine][iCol];
     }
     stream << endl;
   }

   stream.close();
 }
//===========================================================
bool ChrisLib::ComparableHists( TH1* a, TH1* b ) {

  if ( !a || !b ) throw invalid_argument( "ComparableHists : Null histograms" );
  if ( a==b ) return true;
  if ( a->GetNbinsX() != b->GetNbinsX() ) return false;

  if ( a->GetXaxis()->GetXmin() != b->GetXaxis()->GetXmin() ) return false;
  if ( a->GetXaxis()->GetXmax() != b->GetXaxis()->GetXmax() ) return false;

  const TArrayD *aArray = a->GetXaxis()->GetXbins();
  const TArrayD *bArray = b->GetXaxis()->GetXbins();

  if ( aArray->GetSize() != bArray->GetSize() ) return false;

  for ( int i =0; i<aArray->GetSize(); ++i ) {
    if ( !CompareDouble( aArray->At(i), bArray->At(i) ) ) return false;
  }
  return true;
}

//===========================================================
void ChrisLib::FillDefaultFrontiers( vector<double> &list, const int nBins, double xMin, double xMax ) {
  if ( !nBins ) return;
  if ( xMin>xMax ) swap( xMin, xMax );

  double width = (xMax-xMin)/nBins;

  for ( int i=0; i<=nBins; ++i ) {
    list.push_back( xMin + i*width );
  }

}
//======================================================
string ChrisLib::RemoveSeparator( string name, const string sep ) {
  size_t pos = name.find( sep+sep );
  while ( pos != string::npos ) {
    name.erase( pos, sep.size() );
    pos = name.find( sep+sep );
  }
  
  while ( name.size()>=sep.size() && name.substr(name.size()-sep.size() ) == sep ) name.erase( name.size() - sep.size() );

  return name;
}

//======================================================
string ChrisLib::RemoveWords( string name,const list<string> &toRemove ) {
  for ( auto itRemove = toRemove.begin(); itRemove!=toRemove.end(); ++itRemove ) {
    size_t pos = name.find( *itRemove );
    while ( pos != string::npos ) {
      name.erase( pos, itRemove->size() );
      pos = name.find( *itRemove );
    }
  }
  return name;
}

//==========================================================
void ChrisLib::PrintHist( vector<TH1*> &vectHist, string outName, int mode ) {
  RemoveNullPointers( vectHist );
  if ( vectHist.empty() ) throw invalid_argument( "PrintHist : Empty input vector of histograms" );

  fstream stream;
  outName += ".csv";
  stream.open( outName, fstream::out | fstream::trunc );
      
  for ( int iBin = 0; iBin <= vectHist[0]->GetNbinsX(); ++iBin ) {
    for ( unsigned int iPlot = 0; iPlot <= vectHist.size(); ++iPlot ) {
      if ( !iBin ) {
	if ( iPlot ) {
	  string lineName = vectHist[iPlot-1]->GetTitle();
	  stream << lineName;
	  if ( mode == 2 ) stream << "," << lineName + " err";
	}
	else {
	  TString colName = vectHist[0]->GetXaxis()->GetTitle();
	  colName=colName.ReplaceAll("_", "" ).ReplaceAll("#", "" ) ;
	  stream << colName; 
	}
      }
      else {
	if ( iPlot ) { 
	  stream << vectHist[iPlot-1]->GetBinContent( iBin );
	  if ( mode == 2 ) stream << "," << vectHist[iPlot-1]->GetBinError( iBin );
	}
	else stream << ( strcmp( vectHist.front()->GetXaxis()->GetBinLabel(iBin), "" ) ? TString(vectHist[0]->GetXaxis()->GetBinLabel(iBin)) :  TString::Format( "] %2.2f : %2.2f]", vectHist[0]->GetXaxis()->GetBinLowEdge( iBin ), vectHist[0]->GetXaxis()->GetBinUpEdge( iBin ) ) );
      }
      if ( iPlot != vectHist.size() ) stream << ",";
    }
    stream << endl;
  }
  stream.close();
  cout << "Wrote " << outName  << endl;
}
//======================================================
void ChrisLib::CopyTreeSelection( TTree* inTree, const string &selection ) {
  if ( selection == "" ) return;
  TFile *dumFile = new TFile( "/tmp/dumFile", "RECREATE" );
  gROOT->cd();
  TTree* dumTree = inTree->CopyTree( selection.c_str() );
  if ( dumTree ) {
    delete inTree;
    inTree= dumTree;
    inTree->SetDirectory(0);
  }
  delete dumFile; dumFile=0;
}
//============================================================
void ChrisLib::WriteVectHist( const vector<TH1*> &vectHist, const string &outName ) {
  TFile outFile( (outName+".root").c_str(), "recreate" );
  for ( auto it = vectHist.begin(); it != vectHist.end(); ++it ) (*it)->Write( "", TObject::kOverwrite );
  outFile.Close();
}
