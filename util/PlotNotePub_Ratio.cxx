#include <iostream>
#include <istream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/multi_array.hpp>

#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TProfile.h"
#include "TLorentzVector.h"
#include "TRandom.h"
#include "TPad.h"
#include "TROOT.h"
#include "TLine.h"
#include "TLegend.h"

#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"

using namespace std;
using std::map;

int main()
{

  SetAtlasStyle();
  unsigned int nEntries, bin, nBins;
  map <string, double> mapDouble;
  map <string, long long int> mapLongLong;
  map <string, TH1D*> mapHist;
  vector <string> vectNElec;
  vector <TLorentzVector> VLElec(2);
  double SF, phi, eta, pt, deltaSF;
  TLorentzVector VLZ;
  string type;
  TFile *inFile=0;
  TTree *inTree=0;
  TCanvas *cRatio=0;
  vector <string> inFileNames;
  string fileName, pattern, year, letter;
  TRandom randN;
  TH1D* systHist=0;
  
  vectNElec.push_back("1");
  vectNElec.push_back("2");

  TH1::AddDirectory(kFALSE);

  TFile *scalesFile= TFile::Open( "/sps/atlas/c/cgoudet/Calibration/ScaleResults/160519/EnergyScaleFactors.root" );  
  TH1D *histAlphaVal= (TH1D*) scalesFile->Get("centVal_alpha");
  TH1D *histAlphaSyst= (TH1D*) scalesFile->Get("totSyst_alpha");
  TH1D *histCVal= (TH1D*) scalesFile->Get("centVal_c");
  TH1D *histCSyst= (TH1D*) scalesFile->Get("totSyst_c");
  nBins= 20;

  year="2015";
  letter="b";

  for (unsigned int isData= 0; isData<2; isData++)
    {
      if (isData==0)
  	{
  	  pattern="Data_13TeV_Zee_"+year+"_Lkh1_scaled";
  	  inFile= TFile::Open( ( "/sps/atlas/c/cgoudet/Calibration/DataxAOD/"+pattern+"/"+pattern+"_0.root").c_str() );
  	  inTree= (TTree*) inFile->Get( (pattern+"_0_selectionTree").c_str() );
	  cout<<(pattern+"_selectionTree").c_str()<<endl;
  	  type= "Data";

  	  MapBranches mapBranches;
  	  mapBranches.LinkTreeBranches(inTree);

  	  nEntries= inTree->GetEntries();
  
  	  cout<<"nEntries: "<<nEntries<<endl;
     
  	  for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
  	    {
  	      inTree->GetEntry(iEntry);
  	      mapDouble=mapBranches.GetMapDouble();
  	      mapLongLong=mapBranches.GetMapLongLong();
      
  	      for (int signDelta=-1; signDelta<2; signDelta++)
  		{
  		  for (unsigned int iElec=0; iElec<vectNElec.size(); iElec++)
  		    {
  		      eta=mapDouble.at("eta_calo_"+vectNElec[iElec]);
  		      phi=mapDouble.at("phi_"+vectNElec[iElec]);
  		      bin= histAlphaVal->FindFixBin(eta);
  		      SF=histAlphaVal->GetBinContent(bin);
  		      //deltaSF=sqrt( pow(histAlphaVal->GetBinError(bin),2) + pow(histAlphaSyst->GetBinContent(bin),2) );
		      deltaSF=histAlphaSyst->GetBinContent(bin);
  		      pt=mapDouble.at("pt_"+vectNElec[iElec])/(1+SF+signDelta*deltaSF);
  		      VLElec[iElec].SetPtEtaPhiM(pt, eta, phi, 0.511);//in MeV
  		    }
  		  VLZ=VLElec[0]+VLElec[1];
  		  string histName= type+to_string(signDelta);

  		  if (mapHist.count(histName)==0) {mapHist.insert( make_pair(histName, new TH1D(histName.c_str(), "", nBins, 80, 100)) );}
  		  mapHist[histName]->Fill(VLZ.M()/1000); 

  		}
  	    }
  	  inFile->Close();

  	}

      else //for MC
  	{
  	  type="MC";
  	  pattern="MC_13TeV_Zee_2015"+letter+"_Lkh1_scaled";
  	  for (unsigned int i=0; i<100; i++)
  	    {
  	      fileName= "/sps/atlas/c/cgoudet/Calibration/DataxAOD/"+pattern+"/"+pattern+"_"+to_string(i)+".root";
  	      inFile= TFile::Open( fileName.c_str() );
  	      if (!inFile) {break;}
  	      else inFileNames.push_back(fileName);
  	      inFile->Close();
  	    }

  	  for (unsigned int iFile=0; iFile<inFileNames.size(); iFile++)
  	    {
	      
  	      inFile= TFile::Open( inFileNames[iFile].c_str() );
  	      cout<<(pattern+"_"+to_string(iFile)+"_selectionTree").c_str()<<endl;
  	      inTree= (TTree*) inFile->Get( (pattern+"_"+to_string(iFile)+"_selectionTree").c_str() );
	      
  	      MapBranches mapBranches;
  	      mapBranches.LinkTreeBranches(inTree);

  	      nEntries= inTree->GetEntries();

  	      cout<<"nEntries: "<<nEntries<<endl;

  	      for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
  		{
  		  inTree->GetEntry(iEntry);
  		  mapDouble=mapBranches.GetMapDouble();
  		  mapLongLong=mapBranches.GetMapLongLong();

  		  for (int signDelta=-1; signDelta<2; signDelta++)
  		    {
  		      randN.SetSeed();
  		      for (unsigned int iElec=0; iElec<vectNElec.size(); iElec++)
  			{
  			  eta=mapDouble.at("eta_calo_"+vectNElec[iElec]);
  			  phi=mapDouble.at("phi_"+vectNElec[iElec]);
  			  bin= histCVal->FindFixBin(eta);
  			  SF=histCVal->GetBinContent(bin);
  			  //deltaSF=sqrt( pow(histCVal->GetBinError(bin),2) + pow(histCSyst->GetBinContent(bin),2) );
			  deltaSF=histCSyst->GetBinContent(bin);
  			  pt=mapDouble.at("pt_"+vectNElec[iElec])*( 1+(SF+signDelta*deltaSF)*randN.Gaus(0,1) );
  			  VLElec[iElec].SetPtEtaPhiM(pt, eta, phi, 0.511);//in MeV
  			}
  		      VLZ=VLElec[0]+VLElec[1];
  		      string histName= type+to_string(signDelta);

  		      if (mapHist.count(histName)==0) {mapHist.insert( make_pair(histName, new TH1D(histName.c_str(), "", nBins, 80, 100)) );}
  		      mapHist[histName]->Fill(VLZ.M()/1000); 

  		    }
  		}

  	      inFile->Close();
  	    }

  	}
    }

  TFile outFile(("histNotePub_"+year+".root").c_str(), "RECREATE");
  systHist= new TH1D("systTot","",nBins,80,100 );
  for (auto it: mapHist)
    {
      it.second->Write();
    }

  double maxC, maxAlpha;

           
  for(bin=1; bin<=nBins; bin++)
    {
      maxC=max( abs(mapHist["MC0"]->GetBinContent(bin)/mapHist["MC-1"]->GetBinContent(bin)),abs(mapHist["MC0"]->GetBinContent(bin)/mapHist["MC1"]->GetBinContent(bin)) );
      maxAlpha=max( abs(mapHist["Data0"]->GetBinContent(bin)/mapHist["Data-1"]->GetBinContent(bin)), abs(mapHist["Data0"]->GetBinContent(bin)/mapHist["Data1"]->GetBinContent(bin)) );
      //      cout<<maxC<<" alpha: "<<maxAlpha<<" q sum "<<sqrt(pow(maxAlpha,2)+pow(maxC,2))<<endl;

      systHist->SetBinError( bin, sqrt(pow(maxAlpha-1,2)+pow(maxC-1,2)));
    }

  systHist->Write();

  outFile.Close();
 
  //  TFile *fileSyst=TFile::Open("./histNotePub.root");
  //systHist=(TH1D*)fileSyst->Get("systTot");

  inFile=TFile::Open(("/afs/in2p3.fr/home/a/aguergui/public/Calibration/MCDataRatio_"+year+"_m12.root").c_str());
  cRatio=(TCanvas*)inFile->Get("c1");
  TH1D *histRatio=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(1)))->GetListOfPrimitives()->At(0);

  TH1D *histData=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(0)))->GetListOfPrimitives()->At(3);
  TH1D *histMC=(TH1D*)((TPad*)(cRatio->GetListOfPrimitives()->At(0)))->GetListOfPrimitives()->At(2);

  TH1D *histNew=(TH1D*)histRatio->Clone();

  TCanvas *newC = new TCanvas("canvas", "");
  
  TPad padUp( "padUp", "padUp", 0, 0.3, 1, 1 );
  padUp.SetTopMargin( 0.08 );
  padUp.SetBottomMargin( 0.02 );
  TPad padDown( "padDown", "padDown", 0, 0, 1, 0.3 );
  padDown.SetTopMargin( 0.05 );
  padDown.SetBottomMargin( 0.3 );

  newC->Divide(1, 2);
  padUp.Draw();
  newC->cd();
  padDown.Draw();
  padUp.cd();

  float sizeText=0.05;

  histData->SetMarkerStyle(8);
  histData->SetMarkerSize(1.3);
  histData->SetMarkerColor(kBlue);
  histData->SetLineColor(kBlue);
  histMC->SetTitle("");
  histMC->GetYaxis()->SetTitle("#frac{1}{N} #frac{dN}{1 GeV}");
  histMC->GetYaxis()->SetTitleSize(0.05);
  histMC->GetYaxis()->SetTitleOffset(0.75);
  histMC->GetYaxis()->SetRangeUser(0, 0.2);
  histMC->SetLineColor(kBlack);
  histMC->GetXaxis()->SetLabelSize(0);
  histMC->Draw("HIST");
  histData->Draw("SAME");
  ATLASLabel(0.15, 0.85, "Internal");
  myText(0.15, 0.75, 1,"#sqrt{s}=13 TeV, L = 3.2 fb^{-1}", sizeText);
  myText(0.15, 0.65, 1, (year+" data").c_str(), sizeText);
  
  TLegend *leg= new TLegend(0.65,0.85,0.85,0.7);
  leg->AddEntry(histData,"Calibrated Data","lp");
  leg->AddEntry(histMC,"Corrected MC","l");
  leg->SetBorderSize(0);
  leg->SetTextSize(0.04);
  leg->Draw();
  
  // myLineText(0.7, 0.85, histMC->GetLineColor(), histMC->GetLineStyle(), "Corrected MC", sizeText);
  // myMarkerText(0.7, 0.75, histData->GetMarkerColor(), histData->GetMarkerStyle(), "Calibrated data", sizeText);

  padDown.cd();
  systHist->SetLineColorAlpha(0,0);
  systHist->SetMarkerColorAlpha(0,0);
  systHist->SetFillColor(kGreen-10);
  systHist->SetStats(0);
  systHist->GetXaxis()->SetLabelSize(0.12);
  systHist->GetXaxis()->SetTitleSize(0.12);
  systHist->GetXaxis()->SetTitleOffset(1);
  systHist->GetXaxis()->SetTitle("m_{ee} [GeV]");

  systHist->GetYaxis()->SetTitle("Data/MC -1");
  systHist->GetYaxis()->SetTitleOffset(0.35);
  systHist->GetYaxis()->SetLabelSize(0.09);
  systHist->GetYaxis()->SetTitleSize(0.12);
  systHist->GetYaxis()->SetRangeUser(-0.052, 0.052);
  TLine *line= new TLine(80, 0, 100, 0);
  line->SetLineColor( kBlack);
  line->SetLineStyle(3);

  systHist->Draw("E2");
  histNew->Draw("SAME");
  line->Draw();
  newC->SaveAs(("Ratio_"+year+".pdf").c_str());


   
 //===========End of program
  delete scalesFile;
  delete inFile;
  delete cRatio;
  cout<<"Plot done."<<endl;
  return 0;
}
