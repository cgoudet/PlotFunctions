#include "PlotFunctions/CatalogFunctions.h"
#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/DrawOptions.h"
#include "PlotFunctions/SideFunctions.h"
using namespace ChrisLib;

#include "TObject.h"
#include "TH1D.h"

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <fstream>
using std::fstream;

void  ChrisLib::CompareSystModel() {

  vector<string> categoriesUp { "InclusiveUp", "ggH-CenLowUp", "ggH-CenHighUp", "ggH-FwdLowUp", "ggH-FwdHighUp", "VBFlooseUp", "VBFtightUp", "VHhad-looseUp", "VHhad-tightUp", "VHMETUp", "VHlepUp", "VHdilepUp", "ttHhadUp", "ttHlepUp"};
  vector<string> categoriesDown { "InclusiveDown", "InclusiveUp", "ggH-CenLowDown", "ggH-CenLowUp", "ggH-CenHighDown", "ggH-CenHighUp", "ggH-FwdLowDown", "ggH-FwdLowUp", "ggH-FwdHighDown", "ggH-FwdHighUp", "VBFlooseDown", "VBFlooseUp", "VBFtightDown", "VBFtightUp", "VHhad-looseDown", "VHhad-looseUp", "VHhad-tightDown", "VHhad-tightUp", "VHMETDown", "VHMETUp", "VHlepDown", "VHlepUp", "VHdilepDown", "VHdilepUp", "ttHhadDown", "ttHhadUp", "ttHlepDown", "ttHlepUp"};

  string prefix { "/home/goudet/Documents/Resultats/" };
  vector<string> directories { "h013_1NP_range20/", "h013_Full_range20/", "h013_range20_merged/" };
  vector<string> legends { "ALL", "FULL", "FULL_mergedEta" };
  
  vector<TObject*> vectHist;
  for ( auto dir : directories ) {
    TH1D *hist = new TH1D( TString::Format("Syst_%d", static_cast<int>(vectHist.size()) ), "Syst", categoriesUp.size(), -0.5, categoriesUp.size()-0.5 );
    fstream stream( prefix + dir + "SystVariation_mean.csv" );
    MapBranches mapBr;
    mapBr.LinkCSVFile( stream );
    while ( true ) {
      mapBr.ReadCSVEntry(stream);
      if ( stream.eof() ) break;
      for ( unsigned iBin=0; iBin<categoriesUp.size(); ++iBin ) {
	double val =  mapBr.GetDouble(categoriesUp[iBin]);
	//	cout << "val : "  << iBin << " " << val << " " << hist->GetBinContent( iBin+1 ) << " " <<  Oplus( val, hist->GetBinContent(iBin+1) ) << endl;
	hist->SetBinContent( iBin+1, hist->GetBinContent(iBin+1)+val*val );

	hist->SetBinError( iBin+1, 0 );
      }
    }

    for ( unsigned iBin=0; iBin<categoriesUp.size(); ++iBin ) {
      hist->GetXaxis()->SetBinLabel( iBin+1, categoriesUp[iBin].c_str() );
      hist->SetBinContent( iBin+1, sqrt( hist->GetBinContent(iBin+1)) );
				     
    }
    
    vectHist.push_back( hist );
  }

  DrawOptions drawOpt;
  drawOpt.AddOption( "outName", "CompareSystModel" );
  drawOpt.AddOption( "rangeUserY", "0 0.99" );
    drawOpt.AddOption( "extendUp", "0.2" );
  for ( auto legend : legends ) drawOpt.AddOption( "legend", legend );
  drawOpt.Draw( vectHist );
}
