#include "PlotFunctions/SideFunctions.h"
#include <iostream>
#include "TF1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TObjArray.h"
#include "time.h"
#include <chrono>
#include "TRandom.h"
#include "TClass.h"
#include "TKey.h"
#include "TFile.h"
#include <memory>
#include "THStack.h"
#include "TObject.h"

#define DEBUG 1
using std::unique_ptr;
using std::vector;
using std::cout;
using std::endl;
using std::stringstream;
using namespace std::chrono;
double ComputeChi2( TH1 *MCHist, TH1 *DataHist ) {

  if ( MCHist->GetNbinsX() != DataHist->GetNbinsX() 
       || MCHist->GetXaxis()->GetXmin() != DataHist->GetXaxis()->GetXmin() 
       || MCHist->GetXaxis()->GetXmax() != DataHist->GetXaxis()->GetXmax() ) 
{
    cout << "Histograms for chi do not match" << endl;
    exit(1);
 }
  
  double chi2 = 0;
  for ( int i = 0; i < MCHist->GetNbinsX(); i++ ) {
    double valdif = ( MCHist->GetBinError( i+1 )==0 && DataHist->GetBinError( i+1 )==0 ) ? 0 : MCHist->GetBinContent( i+1 ) - DataHist->GetBinContent( i+1 );
    double sigma2 = ( valdif != 0 ) ? MCHist->GetBinError( i+1 ) * MCHist->GetBinError( i+1 ) + DataHist->GetBinError( i+1 ) * DataHist->GetBinError( i+1 ) : 1;
    chi2 += valdif * valdif / sigma2; 
  }
    
  return chi2;
}

//=======================================================
/**\brief Find the range to fit between +/- 3sigma
   \return 0 OK

Lower and upper bins are put into binMin and binMax variables
*/
int FindFitBestRange( TH1D *hist, int &binMin, int &binMax, double chiMinLow, double chiMinUp ) {


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
void WriteLatexHeader( fstream &latexStream, string title, string author ) {

  latexStream << "\\documentclass[a4paper,12pt]{article}" << endl;
  latexStream << "\\usepackage{graphicx}" << endl;
  latexStream << "\\usepackage{xcolor}" << endl;
  latexStream << "\\usepackage[a4paper, textwidth=0.9\\paperwidth, textheight=0.9\\paperheight]{geometry}" << endl;
  latexStream << "\\usepackage[toc]{multitoc}" << endl;
  latexStream << "\\title{" << title << "}" << endl;
  latexStream << "\\author{" << author << "}" << endl;
  latexStream << "\\date{\\today}" <<endl;
  latexStream << "\\begin{document}\\maketitle" << endl;

}
  //======================================
string StripString( string &inString, bool doPrefix, bool doSuffix ) {
  
  if ( doPrefix ) inString = inString.substr( inString.find_last_of( "/" )+1 );
  if ( doSuffix ) inString = inString.substr( 0, inString.find_last_of( "." ) );
  
  return inString;
}

//=======================================
void RemoveExtremalEmptyBins( TH1 *hist ) {

  int lowBin = 1, upBin = hist->GetNbinsX();
  while ( hist->GetBinContent( lowBin ) == 0 ) lowBin++;
  while ( hist->GetBinContent( upBin ) == 0 ) upBin--;

  hist->GetXaxis()->SetRangeUser( hist->GetXaxis()->GetBinLowEdge( lowBin ), hist->GetXaxis()->GetBinUpEdge( upBin ) );
}

//========================================================
void ParseLegend( TH1* hist, string &legend ) {

  TString dumString = legend;
  if ( hist ) { 
    dumString.ReplaceAll( "__ENTRIES", TString::Format( "%1.0f", hist->GetEntries() ) );
    dumString.ReplaceAll( "__MEAN", TString::Format( "%1.3e", hist->GetMean() ) );
    dumString.ReplaceAll( "__STDEV", TString::Format( "%1.3e", hist->GetStdDev() ) );
    dumString.ReplaceAll( "__INTEGRAL", TString::Format( "%1.3e", hist->GetSumOfWeights() ) );
  }
  dumString.ReplaceAll( "__HASHTAG", "#" );
  dumString.ReplaceAll( "__FILL", "" );
  dumString.ReplaceAll("__NOPOINT", "" );
  dumString.ReplaceAll("__ATLAS", "" );
  dumString.ReplaceAll("__STACK", "" );
  dumString.ReplaceAll("ETA_CALO", "#eta_{CALO}" );

  legend = dumString;

}

//============================================
TTree* Bootstrap( vector< TTree* > inTrees, unsigned int nEvents ) {
  cout << "Bootstrap" << endl;
  string outTreeName = inTrees.front()->GetName() + string( "_bootstrap" );
  TTree * outTree = new TTree ( outTreeName.c_str(), outTreeName.c_str() );
  outTree->SetDirectory(0);
  TRandom rand;
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  unsigned long seed = t1.time_since_epoch().count();
  rand.SetSeed( seed );
  cout << "bootstrapSeed : " << seed << endl;
  cout << "nEvents : " << nEvents << endl;
  unsigned int totEntry = 0;
  for ( unsigned int iTree = 0; iTree < inTrees.size(); iTree++ ) totEntry += inTrees[iTree]->GetEntries();
  if ( !nEvents ) nEvents = totEntry;
  cout << "nEvents : " << nEvents << endl;

  TClass *expectedClass;
  EDataType expectedType;

  do {  
    for ( unsigned int iTree = 0; iTree < inTrees.size(); iTree++ ) {
      cout << "iTree : " << iTree << endl;
      TObjArray *branches = inTrees[iTree]->GetListOfBranches();

      vector< double > varDouble( branches->GetEntries(), 0 );
      vector< long long int > varLongLong( branches->GetEntries(), 0 );

      for ( unsigned int iBranch = 0; iBranch < (unsigned int) branches->GetEntries(); iBranch++ ) {

	( (TBranch*) (*branches)[iBranch])->GetExpectedType( expectedClass, expectedType );

	if ( !expectedClass ) {
	  switch ( expectedType ) { //documentation at https://root.cern.ch/doc/master/TDataType_8h.html#add4d321bb9cc51030786d53d76b8b0bd
	  case 8 : {//double
	    inTrees[iTree]->SetBranchAddress( (*branches)[iBranch]->GetName(), &varDouble[iBranch] );
	    if ( !iTree ) outTree->Branch( (*branches)[iBranch]->GetName(), &varDouble[iBranch] );
	    break;}
	  case 16 :
	    inTrees[iTree]->SetBranchAddress( (*branches)[iBranch]->GetName(), &varLongLong[iBranch] );
	    if ( !iTree ) outTree->Branch( (*branches)[iBranch]->GetName(), &varLongLong[iBranch] );
	    break;
	  default :
	    cout << "bootstrap not planned for type : " << expectedType << endl;
	    exit(0);
	  }//end switch
	}//end if expectedClass
	else {
	  cout << "bootstrap not planned for handmade classes" << endl;
	  exit(0);
	}
	//	if ( expectedClass )  delete expectedClass; expectedClass=0;
      }//end iBranch

      unsigned int nEntries = inTrees[iTree]->GetEntries();
      for ( unsigned int iEvent = 0; iEvent < nEntries; iEvent++ ) {
	if ( outTree->GetEntries() >= nEvents ) {
	  cout << "nEvents : " << nEvents << endl;
	  cout << "iEvent : " << iEvent << endl;
	  break;
	}
	inTrees[iTree]->GetEntry( iEvent );

	unsigned int nUse = rand.Poisson( 1.0*nEvents/totEntry );
	//  cout << "nUse : " << nUse << endl;
	for ( unsigned int iUse = 0; iUse < nUse; iUse++ ) outTree->Fill();

      }//end for iEvent

    }//end iTree
  }
  while ( outTree->GetEntries() < nEvents );

  //  outTree->Print();
  cout << "entries : " << outTree->GetEntries() << endl;

  return outTree;

}

//================================================
string FindDefaultTree( TFile* inFile ) { 
  if ( !inFile ) return "";
  string inFileName = inFile->GetName();
  StripString( inFileName );
  vector<string> listTreeNames;
  
  TIter nextkey( inFile->GetListOfKeys());
  TKey *key=0;
  while ((key = (TKey*)nextkey())) {
    if (strcmp( "TTree",key->GetClassName())) continue;
    listTreeNames.push_back( key->GetName() );
  }
  delete key; key=0;
  
  if ( !listTreeNames.size() ) {
    cout << "No TTree in this file. exiting" << endl;
    exit( 0 );
  }
  else  if ( listTreeNames.size() == 1 ) return listTreeNames.front();
  else 
    for ( auto treeName : listTreeNames )
      if ( TString( treeName ).Contains( inFileName ) ) return treeName;
  return listTreeNames.front();
  
}

//===========================================
void AddTree( TTree *treeAdd, TTree *treeAdded ) {
  TList *list = new TList();
  list->Add( treeAdd );
  list->Add( treeAdded );
  treeAdd->Merge( list );

}

//===================================
void SaveTree( TTree *inTree, string prefix) {
  prefix += string( inTree->GetName()) + ".root";
  TFile *dumFile = new TFile( prefix.c_str(), "RECREATE" );
  inTree->Write();
  dumFile->Close("R");
  delete dumFile;
  delete inTree; inTree=0;
}

//================================
void DiffSystematics( string inFileName, unsigned int mode, bool update ) {

  TFile *outFile = 0;
  TH1D *totSyst = 0;
  TH1D* baseValue = 0;


  //Variables to create uncertainties
  fstream inStream;
  inStream.open( inFileName, fstream::in );
  if ( !inStream.is_open() ) {
    cout << inFileName << " does not exist." << endl;
    exit(0);
  }

  string rootFileName, histName, systName, outSystName;
  unsigned int counterSyst=0;
  while ( inStream >> rootFileName >> histName >> systName ) {
    //remove commentaries
    if ( TString( rootFileName).Contains("#") ) continue;
    if ( !outFile ) {
      outFile = new TFile( rootFileName.c_str(), update ? "UPDATE" : "RECREATE" );
      outSystName = histName;
      totSyst = (TH1D*) outFile->Get( histName.c_str() );
      continue; //counterSyst remains at 0 
    }
    unsigned int tempMode = mode;

    TFile *inFile = new TFile( rootFileName.c_str() );
    if ( !inFile ) {
      cout << rootFileName << " does not exist." << endl;
      exit(0);
    }
    inFile->cd();

    if ( !counterSyst && ( tempMode/10==0 || tempMode/10==2 ) ) {
	//Get base Value
	baseValue = (TH1D*) inFile->Get( histName.c_str() )->Clone();
	baseValue->SetDirectory(0);
	outFile->cd();
	if ( systName != "dum" ) baseValue->Write( systName.c_str(), TObject::kOverwrite );
	if ( DEBUG ) cout << "baseValue defined : " << baseValue << endl;	
      }
    else {
      TH1D* inHist = (TH1D*) inFile->Get( histName.c_str() );
      if ( !inHist ) {
	cout << histName << " does not exist within " << inFile->GetName() << endl;
	exit(0);
      }

      if ( TString( systName ).Contains( "__STAT" ) ) {
	systName = string(TString( systName ).ReplaceAll( "__STAT", "" ));
	for ( int iBin=1; iBin<=inHist->GetNbinsX(); iBin++ ) inHist->SetBinContent( iBin, inHist->GetBinError( iBin ) );
	tempMode = 10 + ( mode % 10 );
	cout << "mode : " << tempMode << endl;
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
	  if ( DEBUG ) cout << "symDiff " << iBin << " : " << symDiff << " " << inHist->GetBinContent(iBin) << " " << inHist->GetBinContent( inHist->GetNbinsX() - iBin+1 ) << " "  << baseValue->GetBinContent(iBin) << " " << baseValue->GetBinContent( baseValue->GetNbinsX() - iBin+1 ) << endl;
	  inHist->SetBinContent( iBin, symDiff );
	  inHist->SetBinContent( inHist->GetNbinsX() - iBin+1, symDiff );
	}
	break;
      }
      }//end switch
      for ( int iBin = 1; iBin<inHist->GetNbinsX()+1; iBin++ ) inHist->SetBinError(iBin, 0 );
      outFile->cd();
      cout << "inHist name : " << inHist->GetName() << endl;
      inHist->Write( "", TObject::kOverwrite );

      if ( DEBUG ) cout << "Add systematic to the model" << endl;
      if ( !totSyst ) {
	totSyst = (TH1D*) inHist->Clone();
	for ( int iBin = 1; iBin<totSyst->GetNbinsX()+1; iBin++ ) totSyst->SetBinContent( iBin, 0 );
	totSyst->SetName( outSystName.c_str()) ;
	totSyst->SetTitle( totSyst->GetName() );
      }
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
      cout << inHist->GetBinContent(12) << " " << totSyst->GetBinContent( 12 ) << endl;
      delete inHist; inHist=0;
    }//end else

    outFile->cd();
    if ( totSyst ) totSyst->Write( "", TObject::kOverwrite );
    delete inFile;
    counterSyst++;
  }//end while

  delete totSyst;
  delete baseValue; baseValue=0;
  cout << "outFile : " << outFile->GetName() << endl;
  delete outFile;
}//EndDiffSystematic

//===================================================
void VarOverTime( string inFileName, bool update) {

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

  while ( inStream >> rootFileName >> histName >> val ) {

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
// void LinkTreeBranches( TTree *inTree, TTree *outTree, map<string, double> &mapDouble,map<string, int> &mapInt, map<string, long long int > &mapLongLong ) {

//   TObjArray *branches = inTree->GetListOfBranches();
//   TClass *expectedClass;
//   EDataType expectedType;
//   string name;
//   for ( unsigned int iBranch = 0; iBranch < (unsigned int) branches->GetEntries(); iBranch++ ) {
    
//     ( (TBranch*) (*branches)[iBranch])->GetExpectedType( expectedClass, expectedType );
//     name=(*branches)[iBranch]->GetName();    
//     if ( !expectedClass ) {
//       switch ( expectedType ) { //documentation at https://root.cern.ch/doc/master/TDataType_8h.html#add4d321bb9cc51030786d53d76b8b0bd
//       case 8 : {//double
// 	mapDouble[name] = 0;
// 	inTree->SetBranchAddress( name.c_str(), &mapDouble[name] );
// 	if ( outTree ) {
// 	  if ( !outTree->FindBranch( name.c_str()) ) outTree->Branch( name.c_str(), &mapDouble[name] );
// 	  else outTree->SetBranchAddress( name.c_str(), &mapDouble[name] );
// 	}
// 	break;}
//       case 3 : {//int
// 	mapInt[name] = 0;
// 	inTree->SetBranchAddress( name.c_str(), &mapInt[name] );
// 	if ( outTree ) {
// 	  if ( !outTree->FindBranch( name.c_str()) ) outTree->Branch( name.c_str(), &mapInt[name] );
// 	  else outTree->SetBranchAddress( name.c_str(), &mapInt[name] );
// 	}
// 	break;}

//       case 16 :
// 	mapLongLong[ name ] = 0;
// 	inTree->SetBranchAddress( name.c_str(), &mapLongLong[name] );
// 	if ( outTree ) {
// 	  if ( !outTree->FindBranch( name.c_str()) ) outTree->Branch( name.c_str(), &mapLongLong[name] );
// 	  else outTree->SetBranchAddress( name.c_str(), &mapLongLong[name] );
// 	}
// 	break;
//       default :
// 	cout << "bootstrap not planned for type : " << expectedType << endl;
//       }
//     }
//   }
//   return ;
// }


//===================================
void RescaleStack( THStack *stack, double integral ) {
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
