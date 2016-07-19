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

#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/DrawPlot.h"


using namespace std;

int main(int argc, char *argv[])
{

  TH1::AddDirectory(kFALSE);

  string path= "/sps/atlas/c/cgoudet/Calibration/DataxAOD/";
  string fileName, pattern;  
  vector <string> vectYear (2);
  vector <TH1*> vectProf; 
  TFile *inFile=0;
  TTree *inTree=0;
  TProfile *prof=0;
  TH1D *hist=0;
  unsigned int nEntries;
  unsigned int timeMin=1e11;
  unsigned int timeMax=0;
  map <string, double> mapDouble;
  map <string, long long int> mapLong;
  double meanZDistri, m12, muPU, weight, timeStamp; 
  vector <string> vectOpt;

  bool isMuPU=1;

  vectYear[0]= "2015";
  vectYear[1]= "2016";

  pattern="Data_13TeV_Zee_2015_Lkh1_scaled";
  fileName=path+pattern+"/"+pattern+"_0.root";
  inFile= TFile::Open(fileName.c_str());

  TH1 *ZMass= (TH1*) inFile->Get( "Data_13TeV_Zee_2015_Lkh1_scaled_0_ZMass");
  meanZDistri= ZMass->GetMean();
  cout<<meanZDistri<<endl;
  inFile->Close();

  for (unsigned int year=0; year<vectYear.size(); year++)
    {
      pattern="Data_13TeV_Zee_"+vectYear[year]+"_Lkh1_scaled";
      fileName=path+pattern+"/"+pattern+"_0.root";
      inFile= TFile::Open(fileName.c_str());

      cout<<fileName.c_str()<<endl;
      if (!inFile) cout<<"error"<<endl;
      inTree= (TTree*) inFile->Get( (pattern+"_0_selectionTree").c_str() );
      
      // TH1 *ZMass= (TH1*) inFile->Get( (pattern+"_0_ZMass").c_str() );
      // meanZDistri= ZMass->GetMean();
      
      //cout<<pattern+"_0_ZMass "<<meanZDistri<<endl;

      MapBranches mapBranches;
      mapBranches.LinkTreeBranches(inTree);

      nEntries= inTree->GetEntries();
      
      if (isMuPU) prof = new TProfile ("hprof", "", 30, 0, 30 );

      for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
	{
	  inTree->GetEntry(iEntry);
	  mapDouble=mapBranches.GetMapDouble();
	  mapLong=mapBranches.GetMapLongLong();

	  m12=mapDouble.at("m12");
	  muPU=mapDouble.at("muPU");
	  weight=mapDouble.at("weight");
	  timeStamp=mapLong.at("timeStamp");
  
	  if (m12<80 || m12>100) continue;

	  if (isMuPU) prof->Fill(muPU, m12, weight);
	  else
	    {
	      if (timeStamp < timeMin) timeMin=timeStamp;
	      if (timeStamp > timeMax) timeMax=timeStamp;
	    }
	  
	}
      
      if (isMuPU)
	{

	  prof->Scale(1/meanZDistri);
	  if (year==0) 
	    {
	      prof->SetMarkerStyle(8);
	      prof->SetMarkerSize(1.3);
	    }
	  else
	    {
	      prof->SetMarkerStyle(25);
	      prof->SetMarkerSize(1.3);
	    }
	  vectProf.push_back(prof);
	  vectOpt.push_back("legend="+vectYear[year]);

	}


      //      delete ZMass;

      inFile->Close();
    }

  unsigned int nEventsInBin=0;
  if (!isMuPU)
    {
      unsigned int nBins= (timeMax-timeMin)/(7*24*60*60); //nb of weeks
      prof = new TProfile ("hprof", "", nBins, 0, nBins);

      for (unsigned int year=0; year<vectYear.size(); year++)
	{

	  pattern="Data_13TeV_Zee_"+vectYear[year]+"_Lkh1_scaled";
	  fileName=path+pattern+"/"+pattern+"_0.root";
	  inFile= TFile::Open(fileName.c_str());
	  inTree= (TTree*) inFile->Get( (pattern+"_0_selectionTree").c_str() );
	  // TH1 *ZMass= (TH1*) inFile->Get( (pattern+"_0_ZMass").c_str() );
	  // meanZDistri= ZMass->GetMean();

	  MapBranches mapBranches;
	  mapBranches.LinkTreeBranches(inTree);

	  nEntries= inTree->GetEntries();
      

	  for (unsigned int iEntry=0; iEntry<nEntries; iEntry++)
	    {
	      inTree->GetEntry(iEntry);
	      mapDouble=mapBranches.GetMapDouble();
	      mapLong=mapBranches.GetMapLongLong();

	      m12=mapDouble.at("m12");
	      if (m12<80 || m12>100) continue;
	      weight=mapDouble.at("weight");
	      timeStamp=(mapLong.at("timeStamp")-timeMin)/(7*24*60*60);
	      //	      if (timeStamp==0 || timeStamp==3 || timeStamp==7 || timeStamp==35 || timeStamp==36 || timeStamp==40) cout<<timeStamp<<" "<<mapLong.at("runNumber")<<endl;
	      if (year==0 || mapLong.at("runNumber")>=298633) 
		{
		  if (timeStamp==37)  {nEventsInBin++; m12=0;}
		  prof->Fill(timeStamp, m12, weight);

		}
	    }

	  if (year==0)
	    {
	      hist= new TH1D("hist", "", nBins, 0, nBins);
	      hist->Sumw2();
	    }
	  
	  else
	    {
	      prof->Scale(1/meanZDistri);
	      for (unsigned int iBin=1; iBin<=nBins; iBin++)
		{
		  //cout<<iBin<<" bin content "<<prof->GetBinContent(iBin)<<endl;
		  if (iBin>30 && prof->GetBinContent(iBin)!=0)
		    {
		      hist->SetBinContent( iBin-20, prof->GetBinContent(iBin) );
		      hist->SetBinError( iBin-20, prof->GetBinError(iBin));
		    }
		  else
		    {
		      hist->SetBinContent( iBin, prof->GetBinContent(iBin) );
		      hist->SetBinError( iBin, prof->GetBinError(iBin));
		    }
		}
	    }
	  //  delete ZMass;
	}
    }

  cout<<"nEventsInBin: "<<nEventsInBin<<endl;

  vectOpt.push_back("latex=__ATLAS Internal");


  if (isMuPU)
    {
      vectOpt.push_back("latexOpt=0.25 0.85");
      vectOpt.push_back("latex=#sqrt{s} = 13 TeV, L = 3.2 (2015) + 2.7 (2016) fb^{-1}");
      vectOpt.push_back("latexOpt=0.5 0.85");
      vectOpt.push_back("legendPos= 0.8 0.8");
      //vectOpt.push_back("xTitle= Average interactions per bunch crossing");
      vectOpt.push_back("xTitle= #mu");
      vectOpt.push_back("yTitle= m_{ee}/<m_{ee}(2015)>");
      vectOpt.push_back("rangeUserY= 0.998 1.002");
      vectOpt.push_back("rangeUserX= 5 31");
      vectOpt.push_back("line=1");
      vectOpt.push_back("extendUp=0.3");
      vectOpt.push_back("drawStyle=4");
    }

  else
    {
      //   hist->GetXaxis()->SetLabelSize(0.1);
      hist->GetXaxis()->SetBinLabel(1, "12/08/15");
      hist->GetXaxis()->SetBinLabel(4, "6/09/15");
      hist->GetXaxis()->SetBinLabel(8, "26/09/15");
      hist->GetXaxis()->SetBinLabel(12, "3/11/15");
      hist->GetXaxis()->SetBinLabel(19, "12/05/16");
      hist->GetXaxis()->SetBinLabel(22, "3/06/16");
      hist->GetXaxis()->SetTitle("Date (day/month/year)");


      vectProf.push_back(hist);
      vectOpt.push_back("latexOpt=0.2 0.85");
      vectOpt.push_back("latex=#sqrt{s} = 13 TeV, L = 3.2 (2015) + 2.7 (2016) fb^{-1}");
      vectOpt.push_back("latexOpt=0.2 0.78");     
      vectOpt.push_back("yTitle= m_{ee}/<m_{ee}(2015)>");
      vectOpt.push_back("rangeUserY= 0.998 1.002");
      vectOpt.push_back("rangeUserX= 0 23");
      vectOpt.push_back("line=1");
      vectOpt.push_back("extendUp=0.3");
    }

  DrawPlot(vectProf, "testProf", vectOpt);

  cout<<"End of plotting"<<endl;
  delete inFile;
  delete prof;
  return 0;
}
