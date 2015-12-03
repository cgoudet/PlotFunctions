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

using std::string;
using std::vector;
using std::cout;
using std::endl;

void Style_Christophe();


int main( ) {
  unsigned int inputType = 1;
  double inputConstantValue = 0.0062348;
  double inputConstantError = 0.001;
  unsigned int readMatrix = 1;
  TFile inFile( "/sps/atlas/c/cgoudet/Calibration/ScaleResults/282712/Data6_25ns.root" );
  inFile.ls();
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

  (*subErrMatrix)(1,0) = 100;
  (*subErrMatrix)(0,1) = 100;
  TMatrixD *resultMatrix = new TMatrixD( nBins, 1);
  TMatrixD *resultErrMatrix= new TMatrixD( nBins,1);

  vector< TH1*> histVect;
   // vector< string > legend = { "Input", "Matrix Inversion" };
  vector< string > legend = { "Input", "Matrix Inversion", "Fit C", "Fit C2" };
  vector<string> options;
  vector< int > fitMethod = { 0, 1, 11, 12};
  for (  unsigned int iProc = 2; iProc < legend.size(); iProc++ ) {
    if ( !iProc  )   {
      histVect.push_back( (TH1D*) inFile.Get( "inputScale_c" )->Clone() );
      histVect.back()->SetName( TString::Format( "hist_%d", (int) histVect.size() ) );
      histVect.back()->GetXaxis()->SetTitle( "#eta_{calo}" );
      histVect.back()->GetYaxis()->SetTitle( "C" );
    }
    else {
      InvertMatrix( *subMatrix, *subErrMatrix, *resultMatrix, *resultErrMatrix, fitMethod[iProc]);
      cout << "inverted" << endl;
      histVect.push_back( (TH1D*) inFile.Get( "measScale_c" )->Clone() );
      histVect.back()->SetName( TString::Format( "hist_%d", (int) histVect.size() ) );
      for ( int iBin = 1; iBin < histVect.back()->GetNbinsX()+1; iBin++ ) {
	histVect.back()->SetBinContent( iBin, (*resultMatrix)(iBin-1, 0) );
	histVect.back()->SetBinError( iBin, (*resultErrMatrix)(iBin-1, 0) );
      }

    }
    options.push_back( "legend=" + legend[iProc] );
  }


  options.push_back( "legendPos=0.4 0.8 0.9 0.95" );
  options.push_back( "line=0" );
  string inFileName = inFile.GetName();
  options.push_back( "latex="+StripString( inFileName ) );
  options.push_back( "latexOpt=0.43 0.76" );
  DrawPlot( histVect, "/sps/atlas/c/cgoudet/Plots/InversionStudy", options );
  return 0;
}
