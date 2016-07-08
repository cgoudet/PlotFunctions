#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include <vector>
#include "TH1F.h"
#include "TString.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/InvertMatrix.h"
#include <vector>
#include <string>
#include "PlotFunctions/SideFunctions.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

using std::string;
using std::vector;
using std::cout;
using std::endl;

void Style_Christophe();


int main( int argc, char* argv[] ) {

  po::options_description desc("LikelihoodProfiel Usage");

  string inFileName, outFileName;
  unsigned int inputType = 0;
  int mode=-1;
  //define all options in the program
  desc.add_options()
    ("help", "Display this help message")
    ("inFileName", po::value<string>(&inFileName), "" )
    ("inputType", po::value<unsigned int>( &inputType )->default_value(1), "" )
    ( "outFileName", po::value<string>( &outFileName ), "" )
    ( "mode", po::value<int>( &mode )->default_value(-1), "" )
    ;
  
  //Define options gathered by position                                                          
  po::positional_options_description p;
  p.add("inFileName", 1);

  // create a map vm that contains options and all arguments of options       
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).style(po::command_line_style::unix_style ^ po::command_line_style::allow_short).run(), vm);
  po::notify(vm);
  
  if (vm.count("help")) {cout << desc; return 0;}
  //===========================================
  if ( inFileName == "" ) {
    cout << "no input file" << endl;
    exit(0);
  }

  double inputConstantValue = 0.0062348;
  double inputConstantError = 0.001;
  unsigned int readMatrix = 1;
  TFile inFile( inFileName.c_str() );
  string matrixName, matrixErrName;
  switch ( inputType ) {
  case 0 : 
    matrixName = "combin_alpha";
    matrixErrName = "combinErr_alpha";
    break;
  case 1 :
    matrixName = "combin_c";
    matrixErrName = "combinErr_c";
    break;
  }

  TMatrixD *combinMatrix = (TMatrixD*) inFile.Get( matrixName.c_str() );

  cout << "combinMatrix : " << combinMatrix << endl;
  TMatrixD *combinErrMatrix = (TMatrixD*) inFile.Get( matrixErrName.c_str() );
  cout << "combinErrMatrix : " << combinErrMatrix << endl;

  unsigned int nBins = combinMatrix->GetNrows();
  TMatrixD *subMatrix = new TMatrixD( nBins, nBins );
  TMatrixD *subErrMatrix = new TMatrixD( nBins, nBins );
  for ( unsigned int iLine = 0; iLine < (unsigned int) subMatrix->GetNrows(); iLine++ ) {
    for ( unsigned int iCol = 0; iCol < (unsigned int) subMatrix->GetNcols(); iCol++ ) {

      if ( readMatrix ) {
           (*subErrMatrix)(iLine, iCol) = (*combinErrMatrix)(iLine, iCol);
	   (*subMatrix)(iLine, iCol) = (*combinMatrix)(iLine, iCol);
      }
      else {
	(*subErrMatrix)(iLine, iCol) = inputConstantError;
	(*subMatrix)(iLine, iCol) = inputConstantValue;
      }
      
    }}
  cout << "read" << endl;

  TMatrixD *resultMatrix = new TMatrixD( nBins, 1);
  TMatrixD *resultErrMatrix= new TMatrixD( nBins,1);

  vector< TH1*> histVect;
  // vector< string > legend = { "Input", "Matrix Inversion" };
  vector< string > legend = { "Input", "Matrix Inversion", "Fit C", "Fit C2" };
  vector<string> options;
  vector< int > fitMethod = { 0, 1, 11, 12};
  
  for (  unsigned int iProc = 0; iProc < legend.size(); iProc++ ) {
    if ( mode !=-1 && fitMethod[iProc]!=mode ) continue;
    if ( !fitMethod[iProc] && inputType==1 ) continue;
    if ( !iProc && false )   {
      histVect.push_back( (TH1D*) inFile.Get( "inputScale_c" )->Clone() );
      histVect.back()->SetName( TString::Format( "hist_%d", (int) histVect.size() ) );
      histVect.back()->GetXaxis()->SetTitle( "#eta_{calo}" );
      histVect.back()->GetYaxis()->SetTitle( "C" );
    }
    else {
      InvertMatrix( *subMatrix, *subErrMatrix, *resultMatrix, *resultErrMatrix, fitMethod[iProc]);
      cout << "inverted" << endl;
      histVect.push_back( (TH1D*) inFile.Get( "measScale_c" )->Clone() );
      histVect.back()->SetName( mode==-1 ? TString::Format( "hist_%d", (int) histVect.size() ) : TString("measScale_") + ( inputType ? "c" : "alpha" ) );
      for ( int iBin = 1; iBin < histVect.back()->GetNbinsX()+1; iBin++ ) {
	histVect.back()->SetBinContent( iBin, (*resultMatrix)(iBin-1, 0) );
	histVect.back()->SetBinError( iBin, (*resultErrMatrix)(iBin-1, 0) );
      }
    }
    
    options.push_back( "legend=" + legend[iProc] );
  }

  if ( outFileName != "" ) {
    cout << outFileName << endl;
    TFile *outFile = new TFile( outFileName.c_str(), "recreate" );
    for ( auto hist : histVect ) hist->Write( "", TObject::kOverwrite );
    outFile->Close("R" );
    delete outFile;
    
    options.push_back( "legendPos=0.4 0.85" );
    options.push_back( "line=0" );
    string inFileName = inFile.GetName();
    options.push_back( "latex="+StripString( inFileName ) );
    options.push_back( "latexOpt=0.4 0.9" );
    //  options.push_back("doRatio=1");
    //  options.push_back( "rangeUserY=0 0.035" );
    DrawPlot( histVect, StripString( outFileName, 0, 1 ) , options );
  }
  return 0;
}
