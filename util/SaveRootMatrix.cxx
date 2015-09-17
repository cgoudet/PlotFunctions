#include <iostream>
#include <iomanip>
#include "TFile.h"
#include "TMatrixD.h"
#include "TTree.h"
#include "TMatrixT.h"
#include "TMatrix.h"
#include <string>
#include <fstream>

using std::setw;
using std::string;
using std::cout;
using std::endl;


int main() {

  //Get the matrix from root file
  TFile *inFile = new TFile( "/sps/atlas/c/cgoudet/Calibration/PreRec/Plots/Note_430662875.root");
  string name = "combinErrSigma";

  TMatrixD *matrix =  new TMatrixD();
  matrix = (TMatrixD*) inFile->Get( name.c_str() );
  matrix->Print();

  //Prepare the outputfile
  std::fstream outStream;
  outStream.open( "/afs/in2p3.fr/home/c/cgoudet/private/Template/PlotFunctions/csv/" + TString( name )  + ".csv" , std::fstream::out );


  unsigned int Nrow = matrix->GetNrows();
  unsigned int Ncol = matrix->GetNcols();

  cout << "Nrow : " << Nrow << " " << matrix->GetNrows() << endl;
  cout << "Ncol : " << Ncol << endl;



  for ( unsigned int row = 0; row < Nrow; row++ ) {
    for ( unsigned int col = 0; col < Ncol; col++ ) {
      //      outStream << setw(15) << (*matrix)(row, col) ;
      outStream << (*matrix)(row, col) << "," ;
	   //      cout << (*matrix)(row, col) << " ";
    }
    outStream << endl;
  }
  outStream.close();

  return 0;
}
