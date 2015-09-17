#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "SideFunctions/DrawPlot.h"
#include <bitset>    
#include "TMath.h"
#include <string>
#include "TString.h"
#include <fstream>

using std::fstream;
using std::string;
using std::cout;
using std::endl;
using std::bitset;



int main () {

  int mode= 2;
  vector< vector< string> > inFileName = {

    {"/sps/atlas/c/cgoudet/Calibration/DataxAOD/MC_13TeV_Zee_0.root", "/sps/atlas/c/cgoudet/Calibration/DataxAOD/MC_13TeV_Zee_1.root"},
    {"/sps/atlas/c/cgoudet/Calibration/DataxAOD/Data_13TeV_Zee_Lkh1_scaled_0.root"}
  };

  vector< vector< double > > bins = {
    {-1.55, -1.37}, {1.37, 1.55}
    //    {-2, -1.8}, {-1.8, -1.6}, {1.6, 1.8}, {1.8, 2}
    //    {-2.47, -1.55}, {-1.55, -1.37}, {1.37, 1.55}, {1.55, 2.47}
  };


  vector<vector<TH1*>> hist;
  vector<vector<string>> legends;  
  for ( unsigned int iFile = 0; iFile < inFileName.size(); iFile++ ) {
    hist.push_back( vector<TH1*>() );
    legends.push_back( vector<string>() );

    string dumString= inFileName[iFile].front().substr( inFileName[iFile].front().find_last_of( "/" ) +1);
    dumString = dumString.substr( 0, dumString.find_last_of( "." ) ) ;
    dumString = dumString.substr( 0, dumString.find_first_of("_") );
    for ( unsigned int iBin = 0; iBin < bins.size()+1; iBin++ ) {
      legends.back().push_back( dumString + " "  + string(iBin ? TString::Format( "%2.1f < eta < %2.1f", bins[iBin-1][0], bins[iBin-1][1] ) : "remaining") );
      hist.back().push_back(0);
      TString name = TString::Format( legends.back().back().c_str(), iBin );
      hist.back().back() = new TH1D( name, name, 40, 70, 110);
      hist.back().back()->GetXaxis()->SetTitle( "M_{Z}" );
      hist.back().back()->GetYaxis()->SetTitle( TString::Format( "evt/%2.2f", (hist.back().back()->GetYaxis()->GetXmax()-hist.back().back()->GetYaxis()->GetXmin())/hist.back().back()->GetNbinsX() ) );
      hist.back().back()->Sumw2();
    }
    

    for ( unsigned int iSubFile = 0; iSubFile < inFileName[iFile].size(); iSubFile++ ) {
      TFile *inFile = new TFile( inFileName[iFile][iSubFile].c_str() );
      dumString = inFileName[iFile][iSubFile].substr( inFileName[iFile][iSubFile].find_last_of( "/" ) +1);
      dumString = dumString.substr( 0, dumString.find_last_of( "." ) ) + "_selectionTree" ;
      TTree *inTree = (TTree*) inFile->Get( dumString.c_str() );
      cout << "inTree : " << inTree << endl;
      double eta1, eta2, m12;
      inTree->SetBranchAddress( "eta_1", &eta1 );
      inTree->SetBranchAddress( "eta_2", &eta2 );
      inTree->SetBranchAddress( "m12", &m12 );
      

      
      for ( unsigned int iEvent=0; iEvent<inTree->GetEntries(); iEvent++ ) {
	inTree->GetEntry( iEvent );
	bitset<20> bit;
	for ( unsigned int iBin = 0; iBin < bins.size(); iBin++ ) {
	  bool is1 = (eta1 < bins[iBin][1] && eta1>bins[iBin][0]);
	  bool is2 = ( eta2 < bins[iBin][1] && eta2>bins[iBin][0] );

	  switch ( mode ) {
	  case 1 :
	    if (  is1 xor is2 )  bit.set( iBin+1 );
	    else if ( is1 && is2 ) bit.set( 0 );
	    break;
	  case 0 :
	    if ( is1 && is2 )  bit.set( iBin+1 );
	    else if ( is1 || is2 ) bit.set( 0 );
	    break;
	  case 2 :
	    if ( is1 || is2 )  bit.set( iBin+1 );
	    break;
	  }
	}


	if ( bit.to_ulong() % 2 == 1 ) continue;
	if ( !bit.to_ulong() ) hist.back()[0]->Fill(m12);
	else  {

	  double decimal =  TMath::Log2(bit.to_ulong()) - floor( TMath::Log2(bit.to_ulong() ) ) ;
	  //	  cout << TMath::Log2(bit.to_ulong()) << " " << decimal << endl;
	  if ( decimal == 0 ) hist.back()[ TMath::Log2(bit.to_ulong()) ]->Fill( m12 );
	}
	
      }
    }

    dumString = dumString.substr( 0, dumString.find_first_of("_") );
    DrawPlot( hist.back(), string( TString::Format( "plot/CompareHV_%s", dumString.c_str() )), legends[iFile], 1, 1, false, 0, {0, 0.18}, { 0.5, 0.7, 0.99, 0.99} );
  }
   
  fstream stream;
  string name = string(TString::Format("HV%d_%dBins.csv", mode, (int) bins.size() ));
  cout << "opening : " << name << endl;
  stream.open( name.c_str(), fstream::out | fstream::trunc );

  vector< vector< double>> mean, sigma;
  vector< vector< unsigned int> > entries;
  for (  int iBin = -1; iBin < (int) bins.size()+1; iBin++ ) {
    if ( iBin == -1 ) stream << "," ;
    else {
    stream << string(iBin ? TString::Format( "%2.1f<eta<%2.1f", bins[iBin-1][0], bins[iBin-1][1] ) : "remaining") << ",";
    mean.push_back( vector<double>() );
    sigma.push_back( vector<double>() );
    entries.push_back( vector<unsigned int>() );
    }
    for ( unsigned int iFile = 0; iFile <  inFileName.size()+1; iFile++ ) {
      if ( iBin == -1 ) {
	if ( iFile == inFileName.size() ) { if ( iFile == 2 ) stream << "(h1-h0)/h0,err"; }
	else {
	string dumString = inFileName[iFile].front().substr( inFileName[iFile].front().find_last_of( "/" ) +1);
	dumString = dumString.substr(0, dumString.find_last_of( "." ) ) + "_selectionTree" ;
	dumString = dumString.substr( 0, dumString.find_first_of("_") );
	stream << dumString + ".m12," << dumString + ".sigma," << dumString + ".events,"; 
	}
	continue;
      }


      if ( iFile == inFileName.size() ) {
	if ( inFileName.size() != 2 ) continue;
	double meanDiff = ( mean.back().back() - mean.back().front())/mean.back().front();
	double errFront = sigma.back().front()/sqrt( entries.back().front() );
	double errBack = sigma.back().back()/sqrt( entries.back().back() );
	double err2 = errFront*errFront/mean.back().back()/mean.back().back() 
	  + errBack*errBack*(1/mean.back().back() + meanDiff/mean.back().back())*(1/mean.back().back() + meanDiff/mean.back().back());
	stream << meanDiff << "," << sqrt(err2 ) ;
	continue;
      }


      mean.back().push_back( hist[iFile][iBin]->GetMean() );
      sigma.back().push_back( hist[iFile][iBin]->GetStdDev() );
      entries.back().push_back( hist[iFile][iBin]->GetEntries() );
      //      legends[iFile][iBin] += TString::Format( ", evt=%u, m=%2.2f, s=%2.2f", entries.back().back(), mean.back().back(), sigma.back().back() );
      stream << mean.back().back() << "," << sigma.back().back() << "," << entries.back().back() << ",";
    }
    stream << endl;
  }
  

  return 0;
}
