#include <iostream>
#include <istream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <stdio.h>
#include <time.h>
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
#include "PlotFunctions/SideFunctions.h"

using namespace ChrisLib;
using namespace std;

int main(int argc, char *argv[])
{

  TH1::AddDirectory(kFALSE);

  // string path= "/sps/atlas/a/aguerguichon/Calibration/DataxAOD/BackUp/";
  string path= "/sps/atlas/a/aguerguichon/Calibration/DataxAOD/";
  string fileName, pattern, name;  
  vector <string> vectYear (2);
  vector <TH1*> vectProf; 
  TFile *inFile=0;
  TTree *inTree=0;
  TProfile *prof=0;
  TH1D *hist=0;
  unsigned int binToRemove=100;
  unsigned int nEntries;
  unsigned int timeMin=1e11;
  unsigned int timeMax=0;
  map <string, double> mapDouble;
  map <string, long long int> mapLong;
  double meanZDistri, m12, muPU, weight, timeStamp; 
  vector <string> vectOpt;

  bool isMuPU=0;

  if (isMuPU) name="/afs/in2p3.fr/home/a/aguergui/public/Calibration/PlotFunctions/PubNote/Mutest";
  else name="/afs/in2p3.fr/home/a/aguergui/public/Calibration/PlotFunctions/PubNote/Timetest";
  vectYear[0]= "15";
  vectYear[1]= "16";

  pattern="Data15_13TeV_Zee_Lkh1";
  fileName=path+pattern+"/"+pattern+"_0.root";
  inFile= TFile::Open(fileName.c_str());

  TH1 *ZMass= (TH1*) inFile->Get( "Data15_13TeV_Zee_Lkh1_0_ZMass");
  meanZDistri= ZMass->GetMean();
  cout<<meanZDistri<<endl;
  inFile->Close();

  for (unsigned int year=0; year<vectYear.size(); year++)
    {
      pattern="Data"+vectYear[year]+"_13TeV_Zee_Lkh1";
      if (isMuPU) prof = new TProfile ("hprof", "", 30, 0, 30 );
      for (int iFile=0; iFile<10; iFile++) 
	{
	  fileName=path+pattern+"/"+pattern+"_"+to_string(iFile)+".root";
	  inFile= TFile::Open(fileName.c_str());

	  if (!inFile) break;
	  cout<<fileName.c_str()<<endl;
	  inTree= (TTree*) inFile->Get( (pattern+"_"+to_string(iFile)+"_selectionTree").c_str() );
      
	  // TH1 *ZMass= (TH1*) inFile->Get( (pattern+"_0_ZMass").c_str() );
	  // meanZDistri= ZMass->GetMean();
      
	  //cout<<pattern+"_0_ZMass "<<meanZDistri<<endl;

	  MapBranches mapBranches;
	  mapBranches.LinkTreeBranches(inTree);

	  nEntries= inTree->GetEntries();
      

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
	      // if ( fabs( mapDouble.at("eta_calo_1") )>1.55 && fabs( mapDouble.at("eta_calo_2") )>1.55) continue;
	  
	      if (isMuPU) prof->Fill(muPU, m12, weight);
	      else
		{
		  if (timeStamp < timeMin) timeMin=timeStamp;
		  if (timeStamp > timeMax) timeMax=timeStamp;
		}
	  
	    }
    	  //      delete ZMass;

	  inFile->Close();
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
	  //vectOpt.push_back("legend="+vectYear[year]);

	}

    }

  if (!isMuPU)
    {
      unsigned int nBins= (timeMax-timeMin)/(7*24*60*60); //nb of weeks
      prof = new TProfile ("hprof", "", nBins, timeMin, timeMax);

      for (unsigned int year=0; year<vectYear.size(); year++)
	{
	  pattern="Data"+vectYear[year]+"_13TeV_Zee_Lkh1";
	  for (int iFile=0; iFile<10; iFile++)
	    {

	      fileName=path+pattern+"/"+pattern+"_"+to_string(iFile)+".root";
	      inFile= TFile::Open(fileName.c_str());
	      if (!inFile) break;

	      inTree= (TTree*) inFile->Get( (pattern+"_"+to_string(iFile)+"_selectionTree").c_str() );
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

	      
		  prof->Fill( mapLong.at("timeStamp"), m12, weight);


		  //	      if (timeStamp==0 || timeStamp==3 || timeStamp==7 || timeStamp==35 || timeStamp==36 || timeStamp==40) cout<<timeStamp<<" "<<mapLong.at("runNumber")<<endl;

		  // timeStamp=(mapLong.at("timeStamp")-timeMin)/(7*24*60*60);
		  // if (year==0 || mapLong.at("runNumber")>=298633) 
		  //	{
		  // 	  if (timeStamp==37)  {m12=0;}
		  // 	  prof->Fill(timeStamp, m12, weight);


		  // }
		}
	    }//end iFile
	  
	  // if (year==0)
	  //   {
	  //     hist= new TH1D("hist", "", nBins, timeMin, timeMax);
	  //     hist->Sumw2();
	  //   }
	  

	      prof->Scale(1/meanZDistri);
	      vectProf.push_back(prof);
	      //for (unsigned int iBin=1; iBin<=nBins; iBin++)
	      //{
	      	  // if ( prof->GetBinContent(iBin)==0 ) {binToRemove=iBin; continue;}
		  
	      	  // if ( iBin > 15 && iBin > binToRemove)
	      	  //   {
	      	  //     hist->SetBinContent( iBin-binToRemove+20, prof->GetBinContent(iBin) );
	      	  //     hist->SetBinError( iBin-binToRemove+20, prof->GetBinError(iBin));
	      	  //     if (iBin % 6 ==0 ) hist->GetXaxis()->SetBinLabel( iBin-binToRemove+20, ConvertEpochToDate(hist->GetBinCenter(iBin)).c_str() );
	      	  //     else hist->GetXaxis()->SetBinLabel( iBin-binToRemove+20, " " );
		      
	      	  //   }

	      	  // else
	      	  //   {
	      	  //     hist->SetBinContent( iBin, prof->GetBinContent(iBin) );
	      	  //     hist->SetBinError( iBin, prof->GetBinError(iBin));
	      	  //     if (iBin % 6 ==0 ) hist->GetXaxis()->SetBinLabel( iBin, ConvertEpochToDate(hist->GetBinCenter(iBin)).c_str() );
	      	  //     else hist->GetXaxis()->SetBinLabel( iBin, " " );

	      	  //   }
		  		  
	     
		  // if (iBin>30 && prof->GetBinContent(iBin)!=0)
		  //   {
		  // hist->SetBinContent( iBin-20, prof->GetBinContent(iBin) );
		  //     hist->SetBinError( iBin-20, prof->GetBinError(iBin));
		  //   }
		  // else
		  //   {
		  //     hist->SetBinContent( iBin, prof->GetBinContent(iBin) );
		  //     hist->SetBinError( iBin, prof->GetBinError(iBin));		  
		  //   }
		  
	      //}
	    
	  //  delete ZMass;

	}
    }



  if (isMuPU)
    {
      vectOpt.push_back("xTitle= #mu");
      vectOpt.push_back("yTitle= m_{ee} / < m_{ee}(2015) >");
      vectOpt.push_back("rangeUserY= 0.998 1.0035");
      vectOpt.push_back("rangeUserX= 5 31");
      vectOpt.push_back("line=1");
      vectOpt.push_back("drawStyle=4");
    }

  else
    {
      //   hist->GetXaxis()->SetLabelSize(0.1);
      // hist->GetXaxis()->SetBinLabel(1, "12/08/15");
      // hist->GetXaxis()->SetBinLabel(4, "6/09/15");
      // hist->GetXaxis()->SetBinLabel(8, "26/09/15");
      // hist->GetXaxis()->SetBinLabel(12, "3/11/15");
      // hist->GetXaxis()->SetBinLabel(19, "12/05/16");
      // hist->GetXaxis()->SetBinLabel(22, "3/06/16");

      prof->GetXaxis()->SetTitle("Date (day/month/year)");
      //hist->SetLineColor(kRed);

      //vectProf.push_back(hist);
      vectOpt.push_back("yTitle= m_{ee} / < m_{ee}(2015) >");
      vectOpt.push_back("rangeUserY= 0.998 1.0035");
      vectOpt.push_back("rangeUserX= 0 23");
      vectOpt.push_back("line=1");
      vectOpt.push_back("extendUp=0.3");
    }

  DrawPlot(vectProf, name, vectOpt);

  cout<<"End of plotting"<<endl;
  delete inFile;
  delete prof;
  return 0;
}


