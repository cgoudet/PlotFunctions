#include <iostream>
using std::cout;
using std::endl;
#include "TCanvas.h"
#include "TFile.h"
#include "RooWorkspace.h"
#include "RooSimultaneous.h"
#include "RooCategory.h"
#include "RooAbsData.h"
#include <vector>
using std::vector;
#include <string>
using std::string;
#include "TIterator.h"
#include "TLegend.h"
#include "RooPlot.h"
#include "TString.h"
#include "RooRealVar.h"
#include <fstream> 
using std::fstream;
using namespace RooFit;
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"
#include "PlotFunctions/SideFunctions.h"

int main() {
  SetAtlasStyle();

  int prog = 2;
  TCanvas *can= new TCanvas();
  switch ( prog ) {
  case 0 : {

  TFile *inFile= new TFile( "/sps/atlas/c/cgoudet/Hgam/Couplages/Outputs/StatChallenge_h011_pdfReco.root" );
  RooWorkspace* inWS = (RooWorkspace*) inFile->Get( "combination" );

  RooSimultaneous*  pdf = (RooSimultaneous*) inWS->pdf( "combinedPdf" );
  RooCategory* cat = (RooCategory*)&pdf->indexCat();
  RooAbsData* data = inWS->data("obsData_G" );


  vector<string> varFiles,legends;
  varFiles.push_back( "/sps/atlas/c/cgoudet/Hgam/Couplages/JobsOutput/Mus/mu_XS_ggH_.csv" );
  legends.push_back( "nominal" );

  // varFiles.push_back( "/sps/atlas/c/cgoudet/Hgam/Couplages/JobsOutput/Mus_fixedPER/mu_XS_ggH_.csv" );
  // legends.push_back( "fixedPER" );

  fstream stream;
  string name;
  double value, error;

  RooAbsData* ds=0;
  TIterator* dataItr = data->split(*cat, true)->MakeIterator();
  while ((ds = (RooAbsData*)dataItr->Next())) { // loop over all channels
    ds->Print();
    RooPlot* frame = inWS->var("m_yy")->frame(110,160,25);
    frame->SetTitle("");
    ds->plotOn( frame );

    for ( unsigned int iVarFile= 0; iVarFile<varFiles.size(); iVarFile++ ) {
      stream.open( varFiles[iVarFile].c_str(), fstream::in );
      while ( stream >> name >> value >> error ) {
    	cout << name << " " << value << endl;
    	RooRealVar *var = inWS->var( name.c_str() );
    	if ( var ) var->setVal( value );      
      }
      stream.close();

    pdf->getPdf( ds->GetName() )->plotOn( frame, LineColor(iVarFile+2) );
    // inWS->var("mu_XS_ggH")->setVal(0);
    // inWS->var("mu_XS_VBF")->setVal(0);
    // inWS->var("mu_XS_ZH")->setVal(0);
    // inWS->var("mu_XS_WH")->setVal(-0.2);
    // inWS->var("mHcomb")->setVal(130);
    // pdf->getPdf( ds->GetName() )->plotOn( frame, LineColor(4) );
    }
    frame->Draw();

    for ( unsigned int iVarFile= 0; iVarFile<=varFiles.size(); iVarFile++ ) {
      myLineText( 0.7, 0.9-0.05*iVarFile, 0.035, iVarFile ? 1+iVarFile : 1, frame->getAttLine(frame->getObject(iVarFile)->GetName())->GetLineStyle(), iVarFile ? legends[iVarFile-1].c_str() : "Data" ); 
    }
    myText(0.16, 0.9, 1, ds->GetName() );
    TString name = "/sps/atlas/c/cgoudet/Plots/TestNui_" + TString(ds->GetName()) + ".pdf";
    can->SaveAs( name );
    vector<string> mus = { "ggH", "VBF", "ttH", "ZH", "WH" };
    for ( auto proc : mus ) inWS->var( string("mu_XS_"+proc).c_str() )->setVal(1);
    inWS->var("mHcomb")->setVal(125);
  }
  break;
  }
  case 1 :  {
    vector<string> rootFileNames, pdfNames, varFiles, observables, legends;
    rootFileNames.push_back( "/sps/atlas/c/cgoudet/Hgam/Couplages/Outputs/StatChallenge_h011_pdfReco.root" );
    varFiles.push_back( "/sps/atlas/c/cgoudet/Hgam/Couplages/JobsOutput/Mus/mu_XS_ggH_.csv" );
    pdfNames.push_back( "combinedPdf" );
    legends.push_back( "Christophe" );

    rootFileNames.push_back( "/sps/atlas/c/cgoudet/Hgam/Couplages/Outputs/Marc/file_workspace_CATEGORIZATION_DEVELOPMENT_2015_ProductionModes.root" );
    varFiles.push_back( "/sps/atlas/c/cgoudet/Hgam/Couplages/Outputs/Marc/fit.csv" );
    pdfNames.push_back( "Pdf_model" );
    legends.push_back( "Marc" );

    vector<RooPlot*> plots;
    vector<RooSimultaneous*> pdfs;
    vector<RooWorkspace*> ws;
    vector<RooCategory*> cats;
    unsigned int nCat = 0;
    fstream stream;
    for ( unsigned int iFile=0; iFile < rootFileNames.size(); iFile++ ) {
      TFile *inFile = new TFile( rootFileNames[iFile].c_str() );
      ws.push_back( (RooWorkspace* ) inFile->Get( FindDefaultTree( inFile, "RooWorkspace" ).c_str() ) );
      pdfs.push_back( (RooSimultaneous*) ws.back()->obj( pdfNames[iFile].c_str() ) );
      cats.push_back( (RooCategory*) &pdfs.back()->indexCat() );
      nCat = nCat ? std::min( nCat, (unsigned int) cats.back()->numTypes() ) : cats.back()->numTypes();
    }
    cout << "nCat : " << nCat << endl;

    for ( unsigned int iCat=0; iCat<nCat; iCat++ ) {
      RooPlot *plot = ws.front()->var("m_yy")->frame(110, 160, 50);

      for ( unsigned int iFile=0; iFile < rootFileNames.size(); iFile++ ) {
	cout << "iFile : " << iFile << endl;
	cats[iFile]->setIndex(iCat);
	string name;
	double value, error;
	stream.open( varFiles[iFile].c_str(), fstream::in );
	while ( stream >> name >> value >> error ) {
	  cout << name << " " << value << endl;
	  RooRealVar *var = ws[iFile]->var( name.c_str() );
	  if ( var ) var->setVal( value );      
	}
	stream.close();
	pdfs[iFile]->getPdf( cats[iFile]->getLabel() )->plotOn(plot, LineColor(2+iFile) );
      }
    plot->Draw();
    for ( unsigned int iFile=0; iFile < rootFileNames.size(); iFile++ ) {
      myLineText( 0.7, 0.9-0.05*iFile, 0.035,  2+iFile, plot->getAttLine(plot->getObject(iFile)->GetName())->GetLineStyle(), legends[iFile].c_str() ); 
    }
    myText(0.16, 0.9, 1, cats.front()->getLabel() );
    TString name = "/sps/atlas/c/cgoudet/Plots/CompareResults_" + TString(cats.front()->getLabel()) + ".pdf";
    can->SaveAs( name );

    }
    break;
  }
  case 2 : {
  TFile *inFile= new TFile( "/sps/atlas/c/cgoudet/Hgam/Couplages/Outputs/StatChallenge_asimov.root" );
  RooWorkspace* inWS = (RooWorkspace*) inFile->Get( FindDefaultTree( inFile, "RooWorkspace" ).c_str() );

  RooSimultaneous*  pdf = (RooSimultaneous*) inWS->pdf( "combinedPdf" );
  RooCategory* cat = (RooCategory*)&pdf->indexCat();
  RooAbsData* data = inWS->data( "obsData" );


  vector<string> varFiles,legends;
  legends.push_back( "Best fit" );
  legends.push_back( "mu_ttH=1" );
  legends.push_back( "mu_ZH=1" );


  string name;
  double value, error;
  fstream stream;


  RooAbsData* ds=0;
  TIterator* dataItr = data->split(*cat, true)->MakeIterator();
  while ((ds = (RooAbsData*)dataItr->Next())) { // loop over all channels
    stream.open( "/sps/atlas/c/cgoudet/Hgam/Couplages/JobsOutput/20160425160433/mu_XS_ggH_.csv", fstream::in );
    while ( stream >> name >> value >> error ) {
      cout << name << " " << value << endl;
      RooRealVar *var = inWS->var( name.c_str() );
      if ( var ) var->setVal( value );      
    }
    stream.close(); 
    if ( !TString( ds->GetName() ).Contains( "VH_dileptons" ) ) continue;
    //   inWS->obj("Yield_Signal_ttH_Channel_VH_dileptons")->Print();
    //   inWS->var( "mu_XS_ttH" )->setVal(0);
    //   inWS->obj("Yield_Signal_ttH_Channel_VH_dileptons")->Print();
    //   inWS->obj("Yield_Signal_ZH_Channel_VH_dileptons")->Print();
    // }
    
    ds->Print();
    RooPlot* frame = inWS->var("m_yy")->frame(109.95,159.95,500);
    frame->SetTitle("");
    ds->plotOn( frame, DataError(RooAbsData::Poisson) );

    pdf->getPdf( ds->GetName() )->plotOn( frame, LineColor(2) );
    vector<string> mus = { "ggH", "VBF", "ttH", "ZH", "WH" };
    //    for ( auto proc : mus ) inWS->var( string("mu_XS_"+proc).c_str() )->setVal(0);
    // pdf->getPdf( ds->GetName() )->plotOn( frame, LineColor(3) );
    // inWS->var( "mu_XS_ttH" )->setVal(0);
    // inWS->var( "mu_XS_ZH" )->setVal(0);
    // pdf->getPdf( ds->GetName() )->plotOn( frame, LineColor(3) );
    // inWS->var( "mu_XS_ttH" )->setVal(-0.1);
    // inWS->var( "mu_XS_ZH" )->setVal(0);
    //    double ttHval = inWS->var( "mu_XS_ZH" )->getVal();
    // cout << "ttHval : " << ttHval << endl;
    // inWS->var( "mu_XS_ttH" )->setVal(0);
    // pdf->getPdf( ds->GetName() )->plotOn( frame, LineColor(3) );
    //    inWS->var( "mu_XS_ttH" )->setVal(ttHval);
    // inWS->var( "mu_XS_ZH" )->setVal(ttHval/2);
    // pdf->getPdf( ds->GetName() )->plotOn( frame, LineColor(4) );
    // inWS->var( "mu_XS_ZH" )->setVal(1);
    //    pdf->getPdf( ds->GetName() )->plotOn( frame, LineColor(4) );

    // inWS->var("mu_XS_ZH")->setVal(1);
    // pdf->getPdf( ds->GetName() )->plotOn( frame, LineColor(4) );
    // inWS->var("mu_XS_ZH")->setVal(0);
    // inWS->var("mu_XS_ttH")->setVal(1);
    // pdf->getPdf( ds->GetName() )->plotOn( frame, LineColor(5) );
    
    frame->Draw();

    // for ( unsigned int iVarFile= 0; iVarFile<legends.size()+1; iVarFile++ ) {
    //   myLineText( 0.7, 0.9-0.05*iVarFile, 0.035, iVarFile ? 1+iVarFile : 1, frame->getAttLine(frame->getObject(iVarFile)->GetName())->GetLineStyle(), iVarFile ? legends[iVarFile-1].c_str() : "Data" ); 
    // }
    myText(0.16, 0.9, 1, ds->GetName() );
    //    myText(0.16, 0.85, 1, "Marc Workspace" );
    TString name = "/sps/atlas/c/cgoudet/Plots/TestAsimov_" + TString(ds->GetName()) + ".pdf";
    can->SaveAs( name );
    // vector<string> mus = { "ggH", "VBF", "ttH", "ZH", "WH" };
    // for ( auto proc : mus ) inWS->var( string("mu_XS_"+proc).c_str() )->setVal(0);
    //    cout << "ttHval : " << ttHval << endl;
  }//end while
  break;
  }//end case

//end switch
  }

  return 0; 
}
