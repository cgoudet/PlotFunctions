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
    - inFileName : Name of the rootfile in which matrices are present.
    The file must follow the convention of the Template framework.

    - inputType (int) : Index of the set of matrices to be used for inversion : 0=combin(Err)_alpha, 1:combin(Err)_c, 2=combin(Err)sigma.

    - outFileName (string, default=InversionStudy): Name of the root file in which results will be saved.

    - mode :

    - fitMethod (int, multitoken) : list of the inversion procedures of ChriLib::InvertMatrix.
    The inserted number will be ordered by increasing order.
    If one provides -1, the expected values are taken from the proper histogram within inFileName.

 */
int main( int argc, char* argv[] ) {

  po::options_description desc("LikelihoodProfiel Usage");

  string inFileName, outFileName, binTemplate;
  unsigned int inputType = 0;
  vector< int > fitMethod;
  //define all options in the program
  desc.add_options()
    ( "help", "Display this help message")
    ( "inFileName", po::value<string>(&inFileName), "Input ROOT file" )
    ( "inputType", po::value<unsigned int>( &inputType )->default_value(1), "" )
    ( "outFileName", po::value<string>( &outFileName )->default_value("InversionStudy"), "" )
    ( "fitMethod", po::value<vector<int>>(&fitMethod)->multitoken(), "")
    ( "binTemplate", po::value<string>(&binTemplate), "")
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
  if ( outFileName == "" ) throw invalid_argument( "InversionStudy : Wrong output filename provided." );

  sort( fitMethod.begin(), fitMethod.end());

  TFile inFile( inFileName.c_str() );
  string matrixName, matrixErrName, inputsName;
  switch ( inputType ) {
  case 0 :
    matrixName = "combin_alpha";
    matrixErrName = "combinErr_alpha";
    inputsName = "inputScale_alpha";
    break;
  case 1 :
    matrixName = "combin_c";
    matrixErrName = "combinErr_c";
    inputsName = "inputScale_c";
    break;
  case 2 :
    matrixName = "combinSigma";
    matrixErrName = "combinErrSigma";
    inputsName = "inputScaleSigma";
    break;
  }

  TMatrixD *combinMatrix = static_cast<TMatrixD*>(inFile.Get( matrixName.c_str() ));
  if ( !combinMatrix ) throw runtime_error( "InversionStudy : unknown input matrix " + matrixName );
  TMatrixD *combinErrMatrix = static_cast<TMatrixD*>(inFile.Get( matrixErrName.c_str() ));
  if ( !combinErrMatrix ) throw runtime_error( "InversionStudy : unknown input error matrix " + matrixErrName );

  unsigned int nBins = combinMatrix->GetNrows();
  TMatrixD *resultMatrix = new TMatrixD( nBins, 1);
  TMatrixD *resultErrMatrix= new TMatrixD( nBins,1);

  vector< TH1*> histVect; //vector to store histograms to print
  //  vector< string > legend = { "Input", "Matrix Inversion", "Fit C", "Fit C2" };
  DrawOptions drawOpt;


  for (  auto iMode : fitMethod ) {

    if ( iMode == -1 ) {
      TH1D *hist = static_cast<TH1D*>(inFile.Get( inputsName.c_str() ) );
      if ( !hist ) throw runtime_error( "InversionStudy : unknown histogram "  + inputsName );

      histVect.push_back( static_cast<TH1D*>(hist->Clone()) );
      delete hist;

      histVect.back()->GetXaxis()->SetTitle( "#eta_{calo}" );
      histVect.back()->GetYaxis()->SetTitle( inputType ? "c" : "#alpha" );
    }
    else {
      InvertMatrix( *combinMatrix, *combinErrMatrix, *resultMatrix, *resultErrMatrix, iMode);

      if ( binTemplate!="" ){
        TH1D *hist = static_cast<TH1D*>(inFile.Get( binTemplate.c_str() ) );
        if ( !hist ) throw runtime_error( "InversionStudy : unknown histogram "  + inputsName );
        histVect.push_back(static_cast<TH1D*>(hist->Clone()));
        delete hist; hist=0;
      }
      else if ( histVect.size() ) histVect.push_back(static_cast<TH1D*>(histVect.back()->Clone()));
      else  histVect.push_back( new TH1D( "hist", "hist", nBins, -0.5, nBins-0.5 ));

      for ( int iBin = 1; iBin < histVect.back()->GetNbinsX()+1; ++iBin ) {
        histVect.back()->SetBinContent( iBin, (*resultMatrix)(iBin-1, 0) );
        histVect.back()->SetBinError( iBin, (*resultErrMatrix)(iBin-1, 0) );
      }
    }
    histVect.back()->SetName( TString::Format( "hist_%d", iMode ) );
    //   drawOpt.AddOption( "legend", legend[iProc] );
  }

  TFile *outFile = new TFile( (outFileName+".root").c_str(), "recreate" );
  for ( auto h : histVect ) h->Write( "", TObject::kOverwrite );
  outFile->Close();
  delete outFile;

  drawOpt.AddOption( "legendPos", "0.4 0.85" );
  drawOpt.AddOption( "line", "0" );
  drawOpt.AddOption( "latex", StripString( inFileName ) );
  drawOpt.AddOption( "latexOpt", "0.4 0.9" );
  drawOpt.AddOption( "outName", outFileName );
  drawOpt.AddOption( "saveRoot", "1");
  drawOpt.Draw( histVect );

  inFile.Close();
  return 0;
}
