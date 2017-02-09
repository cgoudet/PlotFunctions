#include "PlotFunctions/CatalogFunctions.h"
#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/DrawOptions.h"
#include "PlotFunctions/SideFunctions.h"
using namespace ChrisLib;

#include "TObject.h"
#include "TH1D.h"

#include <exception>
using std::runtime_error;
#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <fstream>
using std::fstream;
#include <algorithm>
using std::transform;

void  ChrisLib::CompareSystModel() {

  string prefix { "/home/goudet/Documents/PES/" };
  vector<string> directories;
  vector<string> legends;
  string prod;

  int mode = 0;
  switch (mode){
  case 0 :
    directories = { "h013_ALL_range20/", "h013_Full_range20/", "h013_FullMerged_range20/", "h013_mergeEta_range20/" };
    legends = { "ALL", "FULL", "FULL_merged", "mergeEta" };
    prod = "h013";
    break;
  case 1 :
    directories = { "h014_ALL_range20/" };
    legends = { "ALL" };
    prod = "h014";
    break;
  default :
    throw runtime_error("CompareSystModel : Wrong mode");
  }

  vector<string> categories;
  if ( prod == "h013" ) categories =  { "Inclusive", "ggH-CenLow", "ggH-CenHigh", "ggH-FwdLow", "ggH-FwdHigh", "VBFloose", "VBFtight", "VHhad-loose", "VHhad-tight", "VHMET", "VHlep", "VHdilep", "ttHhad", "ttHlep"};
  else if ( prod == "h014") categories = { "Inclusive", "ggH-0J-Cen", "ggH-0J-Fwd", "ggH-1J-Low", "ggH-1J-Med", "ggH-1J-High", "ggH-1J-BSM", "ggH-2J-Low", "ggH-2J-Med", "ggH-2J-High", "ggH-2J-BSM", "VBF-HjjLow-loose", "VBF-HjjLow-tight", "VBF-HjjHigh-loose", "VBF-HjjHigh-tight", "VHhad-loose", "VHhad-tight", "qqH-BSM", "VHMET-Low", "VHMET-High", "VHMET-BSM", "VHlep-Low", "VHlep-High", "VHdilep-Low", "VHdilep-High", "ttHhad-6j2b", "ttHhad-6j1b", "ttHhad-5j2b", "ttHhad-5j1b", "tHhad-4j2b", "tHhad-4j1b", "ttHlep", "tHlep-1fwd", "tHlep-0fwd" };
  transform( categories.begin(), categories.end(), categories.begin(), [](const string &s){return s+"Up";});


  vector<TObject*> vectHist;
  for ( auto dir : directories ) {
    dir = AddSlash(dir);
    TH1D *hist = new TH1D( TString::Format("Syst_%d", static_cast<int>(vectHist.size()) ), "Syst", categories.size(), -0.5, categories.size()-0.5 );
    string fileName { prefix+dir+"SystVariation_mean.csv"};
    fstream stream(fileName.c_str());
    if (!stream.is_open()) throw runtime_error( "CompareSystModel : " + fileName + "does not exist ");
    MapBranches mapBr;
    mapBr.LinkCSVFile( stream );
    while ( true ) {
      mapBr.ReadCSVEntry(stream);
      if ( stream.eof() ) break;
      for ( unsigned iBin=0; iBin<categories.size(); ++iBin ) {
        double val =  mapBr.GetDouble(categories[iBin]);
        hist->SetBinContent( iBin+1, hist->GetBinContent(iBin+1)+val*val );
        hist->SetBinError( iBin+1, 0 );
      }
    }

    for ( unsigned iBin=0; iBin<categories.size(); ++iBin ) {
      hist->GetXaxis()->SetBinLabel( iBin+1, categories[iBin].c_str() );
      hist->SetBinContent( iBin+1, sqrt( hist->GetBinContent(iBin+1)) );
    }

    vectHist.push_back( hist );
  }

  DrawOptions drawOpt;
  drawOpt.AddOption( "outName", prefix+ "CompareSystModel_"+std::to_string(mode) );
  drawOpt.AddOption( "latex", "mean" );
  drawOpt.AddOption( "latexOpt", "0.16 0.96" );
  drawOpt.AddOption( "latex", prod);
  drawOpt.AddOption( "latexOpt", "0.16 0.92" );
  drawOpt.AddOption( "yTitle", "total systematic" );
  drawOpt.AddOption( "xTitle", "category" );
  drawOpt.AddOption( "rangeUserY", "0 0.99" );
  drawOpt.AddOption( "topMargin", "0.1" );
  drawOpt.AddOption( "bottomMargin", "0.2" );
  drawOpt.AddOption( "legendPos", "0.7 0.96" );

  for ( auto legend : legends ) drawOpt.AddOption( "legend", legend );
  drawOpt.Draw( vectHist );
}
//===
