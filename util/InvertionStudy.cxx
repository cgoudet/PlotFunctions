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

using std::string;
using std::vector;
using std::cout;
using std::endl;

void Style_Christophe();


int main( ) {
  unsigned int inputType = 1;
  unsigned int nBins = 6;
  double inputConstantValue = 0.0062348;
  double inputConstantError = 0.001;
  unsigned int readMatrix = 1;
  //  TFile inFile( "/home/goudet/Hgg/Zim/Calibration/TemplateMethod/150818_Note_430662875.root" );
  TFile inFile( "/sps/atlas/c/cgoudet/Calibration/PreRec/Plots/150818_Note_430662875.root" );
  //  TFile inFile( "/sps/atlas/c/cgoudet/Calibration/Test/MC6_0.root" );
  string matrixName, matrixErrName;
  switch ( inputType ) {
  case 0 : 
    matrixName = "combin_alpha";
    matrixErrName = "combinErr_alpha";
    break;
  case 1 :
    matrixName = "combinSigma";
    matrixErrName = "combinErrSigma";
    break;
  }
  inFile.ls();  
  TMatrixD *combinMatrix = (TMatrixD*) inFile.Get( matrixName.c_str() );
  cout << "combinMatrix : " << combinMatrix << endl;
  TMatrixD *combinErrMatrix = (TMatrixD*) inFile.Get( matrixErrName.c_str() );
  cout << "combinErrMatrix : " << combinErrMatrix << endl;

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

  // (*subMatrix)(0,0)= 1e-3;
  // (*subMatrix)(1,0)= 1.5e-3;
  // (*subMatrix)(0,1)= 1.5e-3;
  // (*subMatrix)(1,1)= 2e-3;
  // (*subErrMatrix)(1, 1) = inputConstantError*5;
  // (*subErrMatrix)(0, 0) = inputConstantError/5;

  TMatrixD *resultMatrix = new TMatrixD( nBins, 1);
  TMatrixD *resultErrMatrix= new TMatrixD( nBins,1);

  vector< TH1*> histVect;
   // vector< string > legend = { "Input", "Matrix Inversion" };
  vector< string > legend = { "Input", "Matrix Inversion", "Fit C", "Fit C2" };
  for (  int iProc = -1; iProc < 4; iProc++ ) {
    if ( iProc == 2 ) continue;
    if ( iProc == -1 )   {
      histVect.push_back( (TH1D*) inFile.Get( "simSigma" )->Clone() );
      // histVect.push_back(0);
      // histVect.back() = new TH1D( "h", "h", nBins, 0.5, nBins+0.5 );
      histVect.back()->SetName( TString::Format( "hist_%d", (int) histVect.size() ) );
      histVect.back()->GetXaxis()->SetTitle( "#eta_{calo}" );
      histVect.back()->GetYaxis()->SetTitle( "C" );
    }
    else {
      unsigned int inProc = inputType ? (iProc? 9+iProc : 1 ): 0;
      InvertMatrix( *subMatrix, *subErrMatrix, *resultMatrix, *resultErrMatrix, inProc);
      cout << "inverted" << endl;
      //      histVect.push_back(0);
      // histVect.back() = new TH1D( "h", "h", nBins, 0.5, nBins+0.5 );
      histVect.push_back( (TH1D*) inFile.Get( "simSigma" )->Clone() );
      histVect.back()->SetName( TString::Format( "hist_%d", (int) histVect.size() ) );
      for ( int iBin = 1; iBin < histVect.back()->GetNbinsX()+1; iBin++ ) {
	histVect.back()->SetBinContent( iBin, (*resultMatrix)(iBin-1, 0) );
	histVect.back()->SetBinError( iBin, (*resultErrMatrix)(iBin-1, 0) );
      }
    }
  }

  cout << histVect.size() << " " << legend.size() << endl;
  DrawPlot( histVect, "/afs/in2p3.fr/home/c/cgoudet/private/Template/PlotFunctions/Plots/InversionStudy"
	    , legend
	    , 0, 0, 0, 0
	    , vector<double>()
	    , { 0.4, 0.7, 0.6, 0.95 }
);
  return 0;
}
