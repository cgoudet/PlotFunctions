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
//#include "TXMLAttr.h"
#include "TList.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TObjArray.h"
#include "TArrayD.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

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

//==============================================
bool ChrisLib::IsHist( TObject* obj ) {
  if (  string(obj->ClassName()) == "TGraphErrors" ) return false;
  else return true;
}

//##########################################################
double ChrisLib::CompareVal( double nom, double par ) {
  if ( nom==0 && par==0 ) return 0;
  if ( nom==0 ) {
    cout << "nominal value is null switching roles" << endl;
    swap( nom, par );
  }
  return ( par-nom )/ nom;
}

//##########################################################
double ChrisLib::CompareErr( double nom, double par, double errNom, double errPar ) {

  if ( nom==0 && par==0 ) return 0;
  if ( nom==0 ) {
    cout << "nominal value is null switching roles" << endl;
    swap( nom, par );
  }

  //  double val = CompareVal( nom, par );

  double diffErrNom = par / nom / nom;
  double diffErrPar = 1. / nom;
  double err = sqrt( diffErrPar *diffErrPar * errPar * errPar + diffErrNom * diffErrNom * errNom * errNom );
  return err;
}

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
    //cout << "valdif : " << valdif << " = " << MCHist->GetBinContent( i ) << " - " << DataHist->GetBinContent( i ) << endl;
    double sigma2 = ( valdif != 0 ) ? MCHist->GetBinError( i ) * MCHist->GetBinError( i ) + DataHist->GetBinError( i ) * DataHist->GetBinError( i ) : 1;
    //cout << "sigma2 : " << sigma2 << " :  " << MCHist->GetBinError( i ) << " " << DataHist->GetBinError( i ) << endl;
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
void ChrisLib::WriteLatexHeader( fstream &latexStream, string title, string author, int mode ) {

  latexStream << "\\documentclass[a4paper,12pt]{article}" << endl;
  latexStream << "\\usepackage{graphicx}" << endl;
  if ( mode ) {
    latexStream << "\\usepackage{csvsimple}" << endl;
    latexStream << "\\usepackage{adjustbox}" << endl;
  }
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
/**\brief Replace keywords by corresponding object properties.
   Works only with TH1 and TGraphErrors.
 */
string ChrisLib::ParseLegend( TObject* obj, const string &legend ) {
  TH1* hist=0;
  TGraphErrors *graph=0;
  if ( !IsHist( obj ) ) graph = static_cast<TGraphErrors*>(obj);
  else hist=static_cast<TH1*>(obj);

  double mean, rms, integral{0}, sumSq{0};
  int nEntries;
  if ( graph ) {
    mean = graph->GetMean();
    rms = graph->GetRMS();
    nEntries = graph->GetN();
    // list<double> sumSqVar {graph->GetStdDev(2), graph->GetMean(3) };
    // sumSq = SumSq(sumSqVar)*nEntries;
  }
  else {
    mean = hist->GetMean();
    rms = hist->GetRMS();
    nEntries = hist->GetEntries();
    integral = hist->Integral();
    list<double> sumSqVar {hist->GetStdDev(2), hist->GetMean(3) };
    sumSq = SumSq(sumSqVar)*nEntries;
  }


  
  TString dumString = legend;
  dumString.ReplaceAll( "__ENTRIES", TString::Format( "%1.0d", nEntries ) );
  dumString.ReplaceAll( "__MEAN", TString::Format( "%1.3e", mean ) );
  dumString.ReplaceAll( "__STDEV", TString::Format( "%1.3e", rms ) );
  dumString.ReplaceAll( "__SUMSQ", TString::Format( "%1.3e", sumSq ) );
  dumString.ReplaceAll( "__INTEGRAL", TString::Format( "%1.3e", integral ) );

  return ParseLegend( static_cast<string>(dumString) );
}
//================================
/**\brief Replace keywords with content in a string

   Accepted keywords and modifications : \n
   - __HASHTAG -> #
   - ETA_CALO -> #eta_{CALO}

   Following options are replaced by an empty string : __FILL __NOPOINT __ATLAS __STACK
 */
string ChrisLib::ParseLegend( const string &legend ) {
  TString dumString = legend;
  dumString.ReplaceAll( "__HASHTAG", "#" );
  dumString.ReplaceAll( "__FILL", "" );
  dumString.ReplaceAll("__NOPOINT", "" );
  dumString.ReplaceAll("__ATLAS", "" );
  dumString.ReplaceAll("__STACK", "" );
  dumString.ReplaceAll("ETA_CALO", "#eta_{CALO}" );
  return static_cast<string>( dumString );
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
  while ((key = static_cast<TKey*>(nextkey()))) {
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
void ChrisLib::DiffSystematics( string inFileName ) {

  string outFileName, totSystName;
  vector<string> histsName, rootFilesName, systsName;
  vector<int> modes;
  bool update{0};
  po::options_description configOptions("configOptions");
  configOptions.add_options()
    ( "outFileName", po::value<string>( &outFileName )->required(), "" )
    ( "totSystName", po::value<string>( &totSystName )->required(), "" )
    ( "mode", po::value<vector<int>>(&modes)->multitoken(), "")
    ( "histName", po::value<vector<string>>(&histsName)->multitoken(), "" )
    ( "systName", po::value<vector<string>>(&systsName)->multitoken(), "" )
    ( "rootFileName", po::value<vector<string>>(&rootFilesName)->multitoken(), "" )
    ( "update", po::value<bool>(&update)->default_value(0), "" )
    ;

  po::variables_map vm;
  ifstream ifs( inFileName, ifstream::in );
  if ( !ifs.good() ) throw invalid_argument( "InputCompare::LoadFile : Unknown file " + inFileName );
  po::store(po::parse_config_file(ifs, configOptions), vm);
  po::notify( vm );
  unsigned nSyst = modes.size();
  if ( histsName.size()!=nSyst || systsName.size()!=nSyst || rootFilesName.size() !=nSyst ) throw invalid_argument( "DiffSystematics : Inputs sizes do not match" );
  
  TFile *outFile = new TFile( outFileName.c_str(), update ? "UPDATE" : "RECREATE" );
  TH1D* totSyst = static_cast<TH1D*>(outFile->Get(totSystName.c_str())); //if totSyst is null, it will be created later
  TH1D *baseValue=0;
  for ( unsigned iSyst=0; iSyst!=nSyst; ++iSyst ) {
    TFile *inFile = new TFile( rootFilesName[iSyst].c_str() );
    if ( !inFile ) throw runtime_error( "DiffSystematics : Unknown ROOT file " + rootFilesName[iSyst] );

    TH1D *inHist = static_cast<TH1D*>(inFile->Get( histsName[iSyst].c_str() ));
    if ( !inHist ) throw runtime_error( "DiffSyst : Unknown hist " + string(inFile->GetName()) + " " + histsName[iSyst] );
    inHist->SetDirectory(0);
    inFile->Close();
    delete inFile;
    
    if ( systsName[iSyst].find("__ERR") != string::npos ) {
      //If stat appears in the name, use the error bars as the systematic
      systsName[iSyst] = ReplaceString("__ERR", "" )(systsName[iSyst]);
      ReverseErrVal(inHist);
    }
    
    if ( !baseValue && modes[iSyst]/100!=1 ) {
      baseValue = static_cast<TH1D*>(inHist->Clone( systsName[iSyst].c_str() ));
      baseValue->SetDirectory(0);
      continue;
    }

    if ( modes[iSyst]/100!=1 ) {
      vector<TH1*> hists = { inHist, baseValue };
      RebinHist( hists );
      inHist = static_cast<TH1D*>(hists[0]);
      baseValue = static_cast<TH1D*>(hists[1]);
      int modeCompare = (modes[iSyst]%100)/10*10 +3;
      CreateSystHist( inHist, baseValue, modeCompare );
    }

    //in case totSyst does not exists in the output file, create it
    if ( !totSyst ) {
      totSyst = static_cast<TH1D*>(inHist->Clone(totSystName.c_str()));
      totSyst->SetDirectory(0);
      totSyst->SetTitle( totSyst->GetName() );
      totSyst->GetYaxis()->SetTitle( "#delta" + TString(inHist->GetYaxis()->GetTitle() ) );
    }
    else {
      vector<TH1*> hists = { totSyst, inHist };
      RebinHist( hists );
      totSyst = static_cast<TH1D*>(hists[0]);
      inHist = static_cast<TH1D*>(hists[1]);
      CreateSystHist( totSyst, inHist, modes[iSyst]%100 );
    }

    for ( int iBin = 1; iBin<inHist->GetNbinsX()+1; ++iBin ) {
      inHist->SetBinError(iBin, 0 );
      totSyst->SetBinError(iBin, 0 );
    }
    
    outFile->cd();
    inHist->Write( "", TObject::kOverwrite );
    delete inHist; inHist=0;
  }

  outFile->cd();
  if ( totSyst ) {
    totSyst->Write( "", TObject::kOverwrite );
    delete totSyst;
  }
  
  if ( baseValue ) {
    if ( string(baseValue->GetName()) == "dum" ) baseValue->Write( "", TObject::kOverwrite );
    delete baseValue;
  }
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
    dumVect->GetXaxis()->SetLabelSize( vect[iVect]->GetXaxis()->GetLabelSize() );
    delete vect[iVect];
    dumVect->SetName( tmpName.c_str() );
    vect[iVect] = dumVect;
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
void ChrisLib::PrintArray( const string &outName, const multi_array<double,2> &array, const vector<string> &linesTitle, const vector<string> &colsTitle ) {
  
   if ( !array.size() || !array[0].size() ) return;
   if ( linesTitle.size() && linesTitle.size() != array.size() ) throw runtime_error("PrintArray : Not enough names for lines.");
   ReplaceString repStr( "_", "\\_" );
   ReplaceString repSpace( "_", "-" );

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
void ChrisLib::PrintHist( vector<TObject*> &vectHist, string outName, int mode ) {
  RemoveNullPointers( vectHist );
  if ( vectHist.empty() ) throw invalid_argument( "PrintHist : Empty input vector." );
  
  fstream stream;
  outName += ".csv";
  stream.open( outName, fstream::out | fstream::trunc );
  if ( !stream.good() ) throw invalid_argument( "ChrisLib::PrintHist : Unknown input file " + outName );
  TH1 * hist =0;
  TGraphErrors *graph=0;
  int nBins = 1;
  for ( int iBin = 0; iBin <= nBins; ++iBin ) {
    for ( unsigned int iPlot = 0; iPlot < vectHist.size(); ++iPlot ) {
      if ( string(vectHist[iPlot]->ClassName())=="TGraphErrors" ) graph=static_cast<TGraphErrors*>(vectHist[iPlot]);
      else hist = static_cast<TH1*>(vectHist[iPlot]);
      
      if ( !iBin ) {
	if ( !iPlot ) {
	  nBins = hist ? hist->GetNbinsX() : graph->GetN();
	  TString colName = hist ? hist->GetXaxis()->GetTitle() : graph->GetXaxis()->GetTitle();
	  colName=colName.ReplaceAll("_", "" ).ReplaceAll("#", "" ) ;
	  stream << colName << ","; 
	}
	
	int tmpNBin = hist ? hist->GetNbinsX() : graph->GetN();
	if ( tmpNBin != nBins ) throw invalid_argument( "PrintHist : All object must have same number of points/bins." );
	string lineName = vectHist[iPlot]->GetTitle();
	stream << lineName;
	if ( mode >= 2 ) stream << "," << lineName + " err";
	if ( mode >= 3 ) stream << "," << lineName + " errX";

      }
      else {
	if ( !iPlot ) {
	  if ( hist ) stream << ( strcmp( hist->GetXaxis()->GetBinLabel(iBin), "" ) ? TString(hist->GetXaxis()->GetBinLabel(iBin)) :  TString::Format( "] %2.2f : %2.2f]", hist->GetXaxis()->GetBinLowEdge( iBin ), hist->GetXaxis()->GetBinUpEdge( iBin ) ) );
	  else if ( graph ) {
	    double x, y;
	    graph->GetPoint( iBin-1, x, y );
	    stream << x << endl;
	  }
	  stream << ",";
	}
	
	double valY, errX, errY;
	if ( hist ) {
	  valY = hist->GetBinContent(iBin);
	  errY = hist->GetBinError(iBin);
	  errX = hist->GetXaxis()->GetBinWidth(iBin);
	}
	else if ( graph ) {
	  graph->GetPoint( iBin-1, errX, valY );
	  errX = graph->GetErrorX( iBin-1 );
	  errY = graph->GetErrorY( iBin-1 );
	}
	stream << valY;
	if ( mode >= 2 ) stream << "," << errY;
	if ( mode >= 3 ) stream << "," << errX;
      }
	
      if ( iPlot != vectHist.size() ) stream << ",";
    }
    stream << endl;
  }
  stream.close();
  cout << "Wrote " << outName  << endl;
										 }
//======================================================
void ChrisLib::CopyTreeSelection( TTree** inTree, const string &selection ) {
  if ( selection == "" ) return;
  gROOT->cd();
  TTree* dumTree = (*inTree)->CopyTree( selection.c_str() );
  if ( dumTree ) {
    delete *inTree;
    (*inTree)= dumTree;
    (*inTree)->SetDirectory(0);
  }
}
//============================================================
void ChrisLib::WriteVect( const vector<TObject*> &vectHist, const string &outName ) {
  TFile outFile( (outName+".root").c_str(), "recreate" );
  for ( auto it = vectHist.begin(); it != vectHist.end(); ++it ) (*it)->Write( "", TObject::kOverwrite );
  outFile.Close();
}
//==============================================
double ChrisLib::TestDoubleTree( TTree *tree, const string &branch ) {
  if ( !tree ) throw invalid_argument( "TestDoubleTree : Null TTree." );
  if ( !tree->GetEntries() ) throw runtime_error( "TestDoubleTree : No entries in TTree." );

  double val{-99};
  int err = tree->SetBranchAddress( branch.c_str(), &val );
  if ( err ) throw runtime_error( "TestDoubleTree : BranchName does not designate a double branch" );
  tree->GetEntry(0);
  return val;
}

//==============================================
void ChrisLib::CreateSystHist( TH1 *inHist, TH1* baseValue, unsigned mode ) {
  if ( !inHist || !baseValue ) throw invalid_argument( "CreateSystHist : Null inputs" );
  if ( !ComparableHists( inHist, baseValue ) ) throw invalid_argument( "CreateSyst : Not comparable histograms." );

  int nBins = baseValue->GetNbinsX();
  int maxIBin = baseValue->GetNbinsX()+1;
  if ( mode /10 ) maxIBin = maxIBin/2;
  for ( int iBin=1; iBin<=maxIBin; ++iBin ) {
    double valHist = inHist->GetBinContent( iBin );
    double valBase = baseValue->GetBinContent( iBin );
    if ( mode/10 ==1 ) {
      valHist = ( valHist + inHist->GetBinContent(nBins-iBin+1))/2;
      valBase = ( valBase + baseValue->GetBinContent(nBins-iBin+1))/2;
    }

    int restMode = mode%10;
    if ( !restMode ) {
      list<double> vals { valHist, valBase};
      valHist = Oplus( vals );
    }
    else if ( restMode==1 ) valHist = fabs(valHist) + fabs(valBase);
    else if ( restMode==2 ) valHist = valBase+valHist;
    else if ( restMode==3 ) valHist = valHist-valBase;
    else if ( restMode==4 ) valHist = fabs( valBase+valHist );
    else if ( restMode==5 ) valHist = fabs( valBase-valHist );

    inHist->SetBinContent( iBin, valHist );
    if ( mode/10==1) inHist->SetBinContent( nBins-iBin+1, valHist );

  }
}

//=============================================
void ChrisLib::ReverseErrVal( TH1* hist ) {
  for ( int iBin=1; iBin<hist->GetNbinsX()+1; ++iBin ) {
    double val = hist->GetBinContent(iBin);
    hist->SetBinContent( iBin, hist->GetBinError(iBin));
    hist->SetBinError( iBin, val );
  }
}
