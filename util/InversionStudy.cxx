#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/DrawOptions.h"
#include "PlotFunctions/InvertMatrix.h"
#include "PlotFunctions/SideFunctions.h"

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TString.h"
#include "TCanvas.h"
#include "TLegend.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <vector>
#include <string>
#include <iostream>
#include <exception>
#include <vector>
using std::invalid_argument;
using std::runtime_error;
using std::string;
using std::vector;
using std::cout;
using std::endl;

void Style_Christophe();
using namespace ChrisLib;

/** \brief Compares different inversion methods

    This functions uses a matrix a couple of square matrices with the same dimension to compare the different inversion algoritms.
    One of the matrices is considered as the central value and the second one as uncertainties.
    The matrices are drawn from a rootfile with the conventions of the Template framework.


    This function uses command arguments :
    - fileName : Name of the rootfile in which matrices are present.
    The file must follow the convention of the Template framework.

    - inputType (int) : Index of the set of matrices to be used for inversion : 0=combin(Err)_alpha, 1:combin(Err)_c, 2=combin(Err)sigma.

    - outFileName : Name of the root file in which results will be saved.

    - mode :


 */
int main( int argc, char* argv[] ) {

  po::options_description desc("LikelihoodProfiel Usage");

  string inFileName, outFileName;
  unsigned int inputType = 0, readMatrix=1;
  int mode=-1;
  double inputConstantValue, inputConstantError;

  //define all options in the program
  desc.add_options()
    ("help", "Display this help message")
    ("inFileName", po::value<string>(&inFileName), "" )
    ("inputType", po::value<unsigned int>( &inputType )->default_value(1), "" )
    ( "outFileName", po::value<string>( &outFileName ), "" )
    ( "mode", po::value<int>( &mode )->default_value(-1), "" )
    ( "inConstVal", po::value<double>( &inputConstantValue )->default_value(0.007), "")
    ( "inConstErr", po::value<double>( &inputConstantError)->default_value(0.001), "")
    ( "readMatrix", po::value<unsigned int>( &readMatrix), "")
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
  if ( inFileName == "" ) throw invalid_argument( "InversionStudy : No input filename provided." );

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
  case 2 :
    matrixName = "combinSigma";
    matrixErrName = "combinErrSigma";
    break;
  }

  TMatrixD *combinMatrix = (TMatrixD*) inFile.Get( matrixName.c_str() );
  if ( !combinMatrix ) throw runtime_error( "InversionStudy : unknown input matrix " + matrixName );
  TMatrixD *combinErrMatrix = (TMatrixD*) inFile.Get( matrixErrName.c_str() );
  if ( !combinErrMatrix ) throw runtime_error( "InversionStudy : unknown input error matrix " + matrixErrName );


  unsigned int nBins = combinMatrix->GetNrows();
  TMatrixD *resultMatrix = new TMatrixD( nBins, 1);
  TMatrixD *resultErrMatrix= new TMatrixD( nBins,1);

  vector< TH1*> histVect;
  vector< string > legend = { "Input", "Matrix Inversion", "Fit C", "Fit C2" };
  DrawOptions drawOpt;
  vector< int > fitMethod = { 0, 1, 11, 12};

  for (  int iProc = -1; iProc < fitMethod.size(); iProc++ ) {
    cout << "iProc : " << iProc << endl;
    if ( mode !=-1 && fitMethod[iProc]!=mode ) continue;
    if ( !fitMethod[iProc] && inputType==1 ) continue;
    if ( !iProc && false )   {
      histVect.push_back( static_cast<TH1D*>(inFile.Get( "inputScale_c" )->Clone()) );
      histVect.back()->SetName( TString::Format( "hist_%d", (int) histVect.size() ) );
      histVect.back()->GetXaxis()->SetTitle( "#eta_{calo}" );
      histVect.back()->GetYaxis()->SetTitle( "C" );
    }
    else {
      InvertMatrix( *subMatrix, *subErrMatrix, *resultMatrix, *resultErrMatrix, fitMethod[iProc]);
      cout << "inverted" << endl;
      histVect.push_back( static_cast<TH1D*>(inFile.Get( "measScale_sigma" )->Clone()) );
      histVect.back()->SetName( mode==-1 ? TString::Format( "hist_%d", static_cast<int>(histVect.size()) ) : TString("measScale_") + ( inputType ? "c" : "alpha" ) );
      for ( int iBin = 1; iBin < histVect.back()->GetNbinsX()+1; iBin++ ) {
        histVect.back()->SetBinContent( iBin, (*resultMatrix)(iBin-1, 0) );
        histVect.back()->SetBinError( iBin, (*resultErrMatrix)(iBin-1, 0) );
      }
    }

    drawOpt.AddOption( "legend", legend[iProc] );
  }

  if ( outFileName != "" ) {
    cout << outFileName << endl;
    TFile *outFile = new TFile( outFileName.c_str(), "recreate" );
    for ( auto hist : histVect ) hist->Write( "", TObject::kOverwrite );
    outFile->Close("R" );
    delete outFile;


    drawOpt.AddOption( "legendPos", "0.4 0.85" );
    drawOpt.AddOption( "line", "0" );
    string inFileName = inFile.GetName();
    drawOpt.AddOption( "latex", StripString( inFileName ) );
    drawOpt.AddOption( "latexOpt", "0.4 0.9" );
    drawOpt.AddOption( "outName", StripString( outFileName, 0, 1 ) );
    //  drawOpt.AddOption("doRatio=1");
    //  drawOpt.AddOption( "rangeUserY=0 0.035" );

    drawOpt.Draw( histVect );
  }
  return 0;
}
