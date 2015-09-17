#include <iostream>
#include <fstream>
#include "TFile.h"
#include "TH1D.h"
#include <string>
using std::fstream;
using std::string;
using std::cout;
using std::endl;

int main() {

  //Get the matrix from root file
  TFile *inFile = new TFile( "~/Documents/Resultats/DataBaseLine/Data24Sigma_NUseEl15.root");
  string name = "sigma";

  TH1D *hist = new TH1D();
  hist = (TH1D*) inFile->Get( "sigma" );

  //Prepare the outputfile
  fstream outStream;
  outStream.open(  string("csv/" + name + ".csv").c_str() , fstream::out );


  for ( int bin = 1; bin < hist->GetNbinsX()+1; bin++ ){

    outStream << hist->GetBinContent( bin ) << " ";    

  }

  outStream.close();

  return 0;
}
