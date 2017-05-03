#include "PlotFunctions/CatalogFunctions.h"
#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/DrawOptions.h"
#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/RobustMinimize.h"
using namespace ChrisLib;

#include "TObject.h"
#include "TH1D.h"
#include "TFile.h"
#include "TF1.h"
#include "RooDataHist.h"
#include "RooExponential.h"
#include "RooMinimizer.h"
#include "RooArgSet.h"

using namespace RooFit;

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
using std::to_string;

void  ChrisLib::CompareSystModel() {

  string prefix { "/home/goudet/Documents/Resultats/" };
  vector<string> directories;
  vector<string> legends;
  string prod;

  string namePrefix = "CompareSystModel";
  int mode = 2;
  switch (mode){
  case 0 :
    directories = { "h015_1NP_BDT_catMerge_range55", "h015_1NP_BDT_catMerge_root" };
    legends = { "sumEntries", "fitNormalization"};
    prod = "h015Merged";
    break;
  case 1 :
    directories = { "h015_1NP", "h015_1NP_BDT_catMerge_root" };
    legends = directories;
    prod = "h015Merged";
    break;
  case 2 :
    directories = { "h015_FULLMerge", "h015_FULLMerge_BDT_catMerge_root"};
    legends = directories;
    prod = "h015Merged";
    namePrefix="CompareCategories";
    break;
  case 3 :
    directories = { "h015_1NP_BDT_catMerge_root", "h015_FULLMerge_BDT_catMerge_root"};
    legends = { "1NP", "FULL"};
    prod="h015Merged";
    namePrefix = "CompareFits";
    break;
  case 4 :
    directories = { "h015_FULL_BDT_catMerge_root"};
    legends={"H015_FULLMerge"};
    prod="h015Merged";
    namePrefix = "CompareModels";
    break;
  default :
    throw runtime_error("CompareSystModel : Wrong mode");
  }

  vector<string> categories;
  if ( prod == "h013" ) categories =  { "Inclusive", "ggH_CenLow", "ggH_CenHigh", "ggH_FwdLow", "ggH_FwdHigh", "VBFloose", "VBFtight", "VHhad_loose", "VHhad_tight", "VHMET", "VHlep", "VHdilep", "ttHhad", "ttHlep"};
  else if ( prod == "h014" ) categories = { "Inclusive", "ggH_0J_Cen", "ggH_0J_Fwd", "ggH_1J_Low", "ggH_1J_Med", "ggH_1J_High", "ggH_1J_BSM", "ggH_2J_Low", "ggH_2J_Med", "ggH_2J_High", "ggH_2J_BSM", "VBF_HjjLow_loose", "VBF_HjjLow_tight", "VBF_HjjHigh_loose", "VBF_HjjHigh_tight", "VHhad_loose", "VHhad_tight", "qqH_BSM", "VHMET_Low", "VHMET_High", "VHMET_BSM", "VHlep_Low", "VHlep_High", "VHdilep_Low", "VHdilep_High", "ttHhad_6j2b", "ttHhad_6j1b", "ttHhad_5j2b", "ttHhad_5j1b", "tHhad_4j2b", "tHhad_4j1b", "ttHlep", "tHlep_1fwd", "tHlep_0fwd" };
  else if ( prod == "h015" ) categories = { "Inclusive", "GGH_0J_CEN", "GGH_0J_FWD","GGH_1J_LOW","GGH_1J_MED","GGH_1J_HIGH","GGH_1J_BSM","GGH_2J_LOW","GGH_2J_MED","GGH_2J_HIGH","GGH_2J_BSM","VBF_HjjLO_loose","VBF_HjjLO_tight","VBF_HjjHI_loose","VBF_HjjHI_tight","VHhad_loose","VHhad_tight","QQH_BSM", "VHMET_LOW","VHMET_MED","VHMET_BSM","VHlep_LOW","VHlep_HIGH","VHdilep_LOW", "VHdilep_HIGH","tHhad_4j2b", "tHhad_4j1b", "ttHhad_BDT4", "ttHhad_BDT3",  "ttHhad_BDT2", "ttHhad_BDT1", "ttHlep", "tHlep_1fwd", "tHlep_0fwd"};
  else if ( prod == "h015Merged" ) categories = { "Inclusive", "GGH_0J_CEN", "GGH_0J_FWD","GGH_1J_LOW","GGH_1J_MED","GGH_1J_HIGH","GGH_1J_BSM","GGH_2J_LOW","GGH_2J_MED","GGH_2J_HIGH","GGH_2J_BSM","VBF_HjjLO_loose","VBF_HjjLO_tight","VBF_HjjHI_loose","VBF_HjjHI_tight","VHhad_loose","VHhad_tight","QQH_BSM", "VHMET_LOW","VHMET_HIGH","VHlep_LOW","VHlep_HIGH","VHdilep","tHhad_4j2b", "tHhad_4j1b", "ttHhad_BDT4", "ttHhad_BDT3",  "ttHhad_BDT2", "ttHhad_BDT1", "ttHlep", "tHlep_1fwd", "tHlep_0fwd"};
  transform( categories.begin(), categories.end(), categories.begin(), [](const string &s){return s+"Up";});

  vector<string> variables {"mean", "sigma", "yield"};

  ReplaceString repUp( "Up");
  for ( auto vVar : variables ) {
    vector<TObject*> vectHist;
    for ( auto dir : directories ) {
      if ( dir.back() == '/' ) dir.pop_back();
      TH1D *hist = new TH1D( TString::Format("Syst_%d", static_cast<int>(vectHist.size()) ), "Syst", categories.size(), -0.5, categories.size()-0.5 );
      string fileName { prefix+AddSlash(dir)+dir+"_SystVariation_" + vVar + ".csv"};
      cout << fileName << endl;
      fstream stream(fileName.c_str());
      if (!stream.is_open()) throw runtime_error( "CompareSystModel : " + fileName + " does not exist ");
      MapBranches mapBr;
      mapBr.LinkCSVFile( stream );

      while ( true ) {
        mapBr.ReadCSVEntry(stream);
        if ( stream.eof() ) break;
        for ( unsigned iBin=0; iBin<categories.size(); ++iBin ) {
          double val =  mapBr.GetDouble(categories[iBin]);
          if ( categories[iBin]=="VHMET-BSM") cout << categories[iBin] << " " << val << endl;
          hist->SetBinContent( iBin+1, hist->GetBinContent(iBin+1)+val*val );
          hist->SetBinError( iBin+1, 0 );
        }
      }
      cout << "racine" << endl;
      for ( unsigned iBin=0; iBin<categories.size(); ++iBin ) {
        hist->GetXaxis()->SetBinLabel( iBin+1, repUp(categories[iBin]).c_str() );
        hist->SetBinContent( iBin+1, sqrt( hist->GetBinContent(iBin+1)) );
        if ( categories[iBin]=="VHMET-BSM") cout << categories[iBin] << " " << hist->GetBinContent(iBin+1) << endl;
      }

      vectHist.push_back( hist );
    }



    DrawOptions drawOpt;
    drawOpt.AddOption( "outName", prefix+ namePrefix+"_" + prod + "_" + vVar );
    drawOpt.AddOption( "latex", vVar );
    drawOpt.AddOption( "latexOpt", "0.16 0.96" );
    // drawOpt.AddOption( "latex", prod);
    // drawOpt.AddOption( "latexOpt", "0.16 0.92" );
    drawOpt.AddOption( "latex", "Up");
    drawOpt.AddOption( "latexOpt", "0.16 0.88" );
    drawOpt.AddOption( "yTitle", "total systematic" );
    drawOpt.AddOption( "xTitle", "category" );
    drawOpt.AddOption( "rangeUserY", "0 0.99" );
    drawOpt.AddOption( "topMargin", "0.16" );
    drawOpt.AddOption( "bottomMargin", "0.2" );
    drawOpt.AddOption( "legendPos", "0.7 0.96" );

    for ( auto legend : legends ) drawOpt.AddOption( "legend", legend );
    drawOpt.Draw( vectHist );
    DeleteContainer( vectHist );
  }
}
//===
void ChrisLib::TestMarc(){
  string directory = "/home/goudet/Documents/Programs/Marc/";
  TFile *inFile = new TFile( "/home/goudet/Documents/Programs/Marc/file_hist.root");
  inFile->ls();

  TH1D *hist = static_cast<TH1D*>( inFile->Get( "hist_met_gamgam") );
  double newErr= 1e-5;
  cout << "bins : " << hist->GetBinContent(7) << " " << hist->GetBinError(7) << endl;
  hist->SetBinError(7, newErr);
  hist->SetBinError(10, newErr);
  TF1 *exp=new TF1( "exp", "[0]+exp([1]*x)", 105e3, 160e3);
  hist->Fit(exp);

  DrawOptions drawOpt;
  drawOpt.AddOption( "outName", directory+"histRoot");
  drawOpt.AddOption( "latex", "p0="+to_string(exp->GetParameter(0)));
  drawOpt.AddOption( "latex", "p1="+to_string(exp->GetParameter(1)));
  drawOpt.AddOption( "latexOpt", "0.16 0.9");
  drawOpt.AddOption( "latexOpt", "0.16 0.85");
  drawOpt.AddOption( "rangeUserY", "0 0.99");
  vector<TObject*> v {hist} ;
  drawOpt.Draw(v);
  v.clear();

  RooRealVar *m_yy=new RooRealVar("m_yy","m_yy",125000,105000,160000);
  RooRealVar *weight = new RooRealVar( "weight", "weight", 1);

  RooDataHist *datahist=new RooDataHist("datahist","datahist",RooArgList(*m_yy),hist);
  datahist->Print("v");
  const RooArgSet *set = datahist->get(6);
  set->Print("v");
  cout << "weight : " << datahist->weight() << endl;
  cout << "error : " << datahist->weightError(RooAbsData::ErrorType::SumW2 ) << endl;
  v.push_back(datahist);

  RooRealVar *roorealvar_exp_slope=new RooRealVar("exp_slope","exp_slope",-1.7e-7,-1,1);
  RooExponential *Pdf_Bkg_exp=new RooExponential("Pdf_Bkg_exp","Pdf_Bkg_exp",*m_yy,*roorealvar_exp_slope);
  v.push_back(Pdf_Bkg_exp );


  RooAbsReal* nll = Pdf_Bkg_exp->createNLL( *datahist, CloneData(false));
  RooMinimizer *_minuit = new RooMinimizer(*nll);
  robustMinimize(*nll, *_minuit);
  roorealvar_exp_slope->Print();

  drawOpt.AddOption( "outName", directory+"histRooFit");
  drawOpt.Draw( m_yy, v );


  //RooDataHist *handmade = new RooDataHist( "handmade", "handmade", )


}
