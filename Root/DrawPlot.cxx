#include <iostream>
#include "TCanvas.h"
#include "TLegend.h"
#include "TLine.h"
#include "TLatex.h"
#include "PlotFunctions/SideFunctions.h"
#include <map>
#include <TROOT.h>
#include "THStack.h"

using std::map;
using std::cout;
using std::endl;
using std::min;
using std::max;

#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"

#define DEBUG 0
// enum EColor { kWhite =0,   kBlack =1,   kGray=920,
//               kRed   =632, kGreen =416, kBlue=600, kYellow=400, kMagenta=616, kCyan=432,
//               kOrange=800, kSpring=820, kTeal=840, kAzure =860, kViolet =880, kPink=900 };

int colors[] = {923, 628, 596, 414, 617, 804, 797};
int fillColors[] = { 3, 5 };

int DrawPlot( vector< TH1* > inHist,  
	      string outName, 
	      vector<string> inOptions
	       ) {

  //================ SOME CHECKS
  if ( DEBUG ) cout << "DrawPlot" << endl;

  map<string, int >  mapOptionsInt;
  mapOptionsInt["doRatio"]=0;
  mapOptionsInt["shiftColor"]=0;
  mapOptionsInt["normalize"]=0;
  mapOptionsInt["doChi2"]=0;
  mapOptionsInt["centerZoom"]=0;
  mapOptionsInt["drawStyle"]=0;
  mapOptionsInt["line"]=-99;
  mapOptionsInt["logy"]=-99;
  mapOptionsInt["stack"]=-99;
  map<string, double > mapOptionsDouble;
  mapOptionsDouble["extendUp"]=0;
  vector<string> inLegend, inLatex; 
  vector< vector< double > > latexPos;
  vector< double > legendCoord, rangeUserX, rangeUserY;
  map<string, string> mapOptionsString;
  mapOptionsString["xTitle"]="";
  mapOptionsString["yTitle"]="";
  for ( auto iOption : inOptions ) {

    string option = iOption.substr( 0, iOption.find_first_of('=' ) );
    string value = iOption.substr( iOption.find_first_of("=")+1);

    if ( mapOptionsInt.find(option) != mapOptionsInt.end() ) mapOptionsInt[option] = atoi( value.c_str() );
    else if ( mapOptionsString.find(option) != mapOptionsString.end() ) mapOptionsString[option] = value;
    else if ( mapOptionsDouble.find(option) != mapOptionsDouble.end() ) mapOptionsDouble[option] =  (double) std::atof( value.c_str() );
    else if ( option == "legend" ) inLegend.push_back( value );
    else if ( option == "latex" ) inLatex.push_back( value );
    else if ( option == "latexOpt" ) {
      latexPos.push_back( vector<double>() );
      ParseVector( value, latexPos.back() );
    }
    else if ( option == "legendPos" ) ParseVector( value, legendCoord );
    else if ( option == "rangeUserX" ) ParseVector( value, rangeUserX );
    else if ( option == "rangeUserY" ) ParseVector( value, rangeUserY );
    else {
      cout << "Option : " << option << " not known" << endl;
    }
  }

  if ( inLegend.size() && inLegend.size()!=inHist.size() ) {
    cout << "Legend do not match input" << endl;
    return  1;
  }
		 
  if ( inLatex.size() != latexPos.size() ) {
    cout << "Number of latex names and positions do not match" << endl;
    cout << inLatex.size() << " " << latexPos.size() << endl;
    return 2;
  }

  if ( inHist.size() == 1 ) mapOptionsInt["drawStyle"] = 0;
  if ( inHist.size() < 2 ) mapOptionsInt["doRatio"] = 0;
  SetAtlasStyle();

  if ( DEBUG ) cout << "Options read" << endl;
  vector< TH1* > ratio;

  //================ PAD DEFINITION
  TCanvas canvas;
  if ( inHist.size()==1 && inHist.front() && TString(inHist.front()->ClassName()).Contains("TH2") ) {
    canvas.SetRightMargin(0.1);
    inHist.front()->Draw( "COLZ" );
    for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
      if ( latexPos[iLatex].size() != 2 ) continue;

      myText( latexPos[iLatex][0], latexPos[iLatex][1], 1, inLatex[iLatex].c_str() );
    }
    canvas.SaveAs( TString(outName) + ".pdf" );
    return 0;
  }

  if ( DEBUG ) cout << "No 2D plot" << endl;

  TPad padUp( "padUp", "padUp", 0, 0.3, 1, 1 );
  padUp.SetBottomMargin( 0 );
  TPad padDown( "padDown", "padDown", 0, 0, 1, 0.3 );
  padDown.SetTopMargin( 0 );
  padDown.SetBottomMargin( 0.2 );

  if ( mapOptionsInt["doRatio"] ) {
    padUp.Draw();
    canvas.cd();
    padDown.Draw();
    padUp.cd();
  }


  if ( !legendCoord.size() ) legendCoord={ 0.7, 0.9  };

  TLine *line = new TLine( 0, 0.005, 100, 0.005);
  line->SetLineColor( kBlack );
  line->SetLineStyle( 3 );
  if ( DEBUG ) cout << "defined pads" << endl;

  //============ LOOP OTHER INPUT HIST
  //Find the extremum of the histograms to choose rangeUser if not given
  double minVal=0, maxVal=0;
  double minX=0, maxX=0;
  THStack stack;
  int refHist= -1;
  //  bool isNegativeValue = false;
  for ( unsigned int iHist = 0; iHist < inHist.size(); iHist++ ) {
    if ( !inHist[iHist] ) continue;
    if ( refHist == -1 ) refHist = iHist;
    inHist[iHist]->UseCurrentStyle();
    if ( (int) iHist == refHist ) {
      if ( mapOptionsString["xTitle"]== "" ) mapOptionsString["xTitle"] = inHist[refHist]->GetXaxis()->GetTitle();
      ParseLegend( 0, mapOptionsString["xTitle"] );
      inHist[refHist]->GetXaxis()->SetTitle( mapOptionsString["xTitle"].c_str() );
      if ( mapOptionsString["yTitle"]!="" ) {
	ParseLegend( 0, mapOptionsString["yTitle"] );
	inHist[refHist]->GetYaxis()->SetTitle( mapOptionsString["yTitle"].c_str() );
      }
      if ( DEBUG ) cout << "titles set" << endl;
    }
    //Set color and style of histogram
    //If only one histograms is plotted, plot it in red
    inHist[iHist]->SetLineColor(  colors[ max( 0, (int) ( (inHist.size()==1 ? 1 : iHist) + mapOptionsInt["shiftColor"])) ]  );
    inHist[iHist]->SetMarkerColor( colors[ max( 0, (int) ((inHist.size()==1 ? 1 : iHist) + mapOptionsInt["shiftColor"] ) )] );

    //If only one histograms is plotted, plot it in red
    switch ( mapOptionsInt["drawStyle"] ) {
    case 1 :
      inHist[iHist]->SetLineColor( colors[ max( 0, (int) (iHist/2 +mapOptionsInt["shiftColor"] ) )] );
      inHist[iHist]->SetMarkerColor( colors[ max( 0 , (int) (iHist/2 + mapOptionsInt["shiftColor"]) ) ] );
      inHist[iHist]->SetMarkerStyle( (iHist%2) ? 4 : 8 ); 
      break;
    }

    //======== CHI2 OF HISTOGRAM RATIOS
    if ( mapOptionsInt["doChi2"] && inLegend.size() && iHist ){
      switch ( mapOptionsInt["drawStyle"] ) {
      case 1 : 
	if ( iHist % 2 ) inLegend[iHist] += " : chi2=" + TString::Format( "%2.2f", ComputeChi2( inHist[iHist], inHist[iHist-1] )/inHist[iHist]->GetNbinsX() );
	break;
      default :
	inLegend[iHist] += " : chi2=" + TString::Format( "%2.2f", ComputeChi2( inHist[iHist], inHist[refHist] )/inHist[refHist]->GetNbinsX() );
      }
    }
    if ( mapOptionsInt["normalize"] && inHist[iHist]->Integral() )  inHist[iHist]->Scale( 1./inHist[iHist]->Integral() );
    if ( DEBUG ) cout << "Style set" << endl;
    //============ LOOK FOR Y EXTREMAL VALUES AND DEFINE Y RANGE
    if( (int) iHist == refHist ) {
      minVal = inHist[refHist]->GetMinimum();
      maxVal = inHist[refHist]->GetMaximum();
    }
    //Update the maximum range of the plot with extremum of current plot
    for ( int bin = 1; bin <= inHist[iHist]->GetNbinsX(); bin++ ) {
      minVal = min( inHist[iHist]->GetBinContent( bin ) - inHist[iHist]->GetBinError( bin ), minVal );
      maxVal = max( inHist[iHist]->GetBinContent( bin ) + inHist[iHist]->GetBinError( bin ), maxVal );
      //  if ( inHist[iHist]->GetBinContent( bin ) < 0 ) isNegativeValue = true;
    }
    if ( DEBUG ) cout << "extremal Y values defined and set " << endl;

    //========== LOOK FOR X EXTREMAL VALUES AND DEFINE X RANGE
    int lowBin = 1, upBin = inHist[iHist]->GetNbinsX();
    while ( inHist[iHist]->GetBinContent( lowBin ) == 0 && lowBin!=upBin ) lowBin++;
    while ( inHist[iHist]->GetBinContent( upBin ) ==0 && lowBin!=upBin ) upBin--;
    if ( lowBin != upBin ) {
      if ( !iHist || minX > inHist[iHist]->GetXaxis()->GetBinLowEdge( lowBin ) ) minX = inHist[iHist]->GetXaxis()->GetBinLowEdge( lowBin );
      if ( !iHist || maxX < inHist[iHist]->GetXaxis()->GetBinUpEdge( upBin ) ) maxX = inHist[iHist]->GetXaxis()->GetBinUpEdge( upBin );
    }

    if ( DEBUG ) cout << "X ranges defined" << endl;
  }//end iHist

  if ( DEBUG ) cout << "drawing" << endl;
  //Plotting histograms
  for ( unsigned int iHist = refHist; iHist < inHist.size(); iHist++ ) {
    if ( !inHist[iHist] ) continue;
    if ( (int) iHist == refHist ) {
      if (mapOptionsInt["doRatio"]) {
	inHist[refHist]->GetYaxis()->SetTitleOffset( 0.6 );
	inHist[refHist]->GetYaxis()->SetTitleSize( 0.06 );
      }

      if ( rangeUserY.size()!=2 ) {
	rangeUserY.clear();
	rangeUserY.push_back( minVal - ( maxVal - minVal ) *0.05 );
	rangeUserY.push_back( maxVal + ( maxVal - minVal ) *0.05 );
      }

      rangeUserY.back() += (rangeUserY.back() - rangeUserY.front()) * mapOptionsDouble["extendUp"];
      inHist[refHist]->GetYaxis()->SetRangeUser( rangeUserY[0], rangeUserY[1] );
      if ( rangeUserX.size() == 2 ) inHist[refHist]->GetXaxis()->SetRangeUser( rangeUserX[0], rangeUserX[1] );
      else if ( mapOptionsInt["centerZoom"] ) inHist[refHist]->GetXaxis()->SetRangeUser( minX, maxX );
    }
    string drawOpt = ( (int) iHist!= refHist) ? "SAME,E" : "E";
    if ( inLegend.size() > iHist && TString( inLegend[iHist].c_str() ).Contains( "__NOPOINT" ) ) {
      inHist[iHist]->SetLineColorAlpha( 0, 0 );
      inHist[iHist]->SetMarkerColorAlpha( 0, 0 );
    }

    if ( inLegend.size() > iHist && TString( inLegend[iHist].c_str() ).Contains( "__FILL" ) ) {
      drawOpt += "2";
      inHist[iHist]->SetFillColor( fillColors[iHist] );
      //      myBoxText( legendCoord[0], legendCoord[1]-0.05*iHist, 0.05, inHist[iHist]->GetFillColor(), inLegend[iHist].c_str() ); 
    }

    inHist[iHist]->Draw( drawOpt.c_str() );
    //    stack.Add( inHist[iHist] );
    if( !iHist && mapOptionsInt["line"] != -99 ) {
      double rangeMin = rangeUserX.size()== 2 ? rangeUserX[0] : (mapOptionsInt["centerZoom"] ? minX : inHist[refHist]->GetXaxis()->GetXmin() );
      double rangeMax = rangeUserX.size()== 2 ? rangeUserX[1] : ( mapOptionsInt["centerZoom"] ? maxX :inHist[refHist]->GetXaxis()->GetXmax() );
      line->DrawLine( rangeMin , mapOptionsInt["line"], rangeMax, mapOptionsInt["line"]);
    }
    //========== ADD HISTOGRAM TO LEGEND
  }//end iHist
  if ( DEBUG ) cout << "drawn" << endl;
  //  stack.Draw( mapOptionsInt["stack"] ? "F" : "nostack"  ); 

  if ( mapOptionsInt["logy"] ) {
    int topVal = ceil( log10( maxVal ) );
    int lowVal = minVal==0 ? topVal-5 : floor( log10( minVal ) );
    // cout << "minVal : " << minVal << " " << maxVal << endl;
    // cout << "lowVal : " << lowVal << " " << topVal << endl;
    if ( rangeUserY[0] < 0 ) rangeUserY[0]=pow( 10, lowVal );
    rangeUserY[1] = pow( 10, topVal + ( topVal - lowVal ) * (0.05 + mapOptionsDouble["extendUp"] ) );
    inHist[refHist]->GetYaxis()->SetRangeUser( rangeUserY[0], rangeUserY[1] );    
    if ( mapOptionsInt["doRatio"] ) {
      padUp.SetLogy(1);
    }
    else {
      canvas.SetLogy(1);
    }
    if ( DEBUG ) cout << "logy done" << endl;
  }

  // =========== PRINT LEGENDS AND LATEX
  for ( unsigned int iLegend=0; iLegend<inLegend.size(); iLegend++ ) {
    if ( !inHist[iLegend] ) continue;
    bool doFill = inLegend.size() > iLegend && TString( inLegend[iLegend].c_str() ).Contains( "__FILL" );
    ParseLegend( inHist[iLegend] , inLegend[iLegend] );
    if ( doFill )  myBoxText( legendCoord[0], legendCoord[1]-0.04*iLegend, 0.02, inHist[iLegend]->GetFillColor(), inLegend[iLegend].c_str() ); 
    else myMarkerText( legendCoord[0], legendCoord[1]-0.05*iLegend, inHist[iLegend]->GetMarkerColor(), inHist[iLegend]->GetMarkerStyle(), inLegend[iLegend].c_str(), 0.5 ); 
  }
  if ( DEBUG )  cout << "legend drawn" << endl;
  for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
    if ( latexPos[iLatex].size() != 2 ) continue;
    bool doLabel = TString( inLatex[iLatex] ).Contains("__ATLAS");
    ParseLegend( 0, inLatex[iLatex] );
    if ( doLabel ) ATLASLabel( latexPos[iLatex][0], latexPos[iLatex][1], inLatex[iLatex].c_str() );
    else myText( latexPos[iLatex][0], latexPos[iLatex][1], 1, inLatex[iLatex].c_str() );
  }
  if ( DEBUG ) cout << "latex drawn" << endl;
  //===============  CREATE RATIO PLOTS
  if ( mapOptionsInt["doRatio"] ) {
    padDown.cd();
    double minValRatio = 0;
    double maxValRatio = 0;
    bool setTitle=0;
    for ( unsigned int iHist = refHist+1; iHist < inHist.size(); iHist++ ) {
      if ( !inHist[iHist] ) continue;
      string yTitle;
      //Decide how to pair histogram for ratio
      switch ( mapOptionsInt["drawStyle"] ) {
      case 1 :
	if ( !(iHist % 2) || !inHist[iHist-1]) continue;
	ratio.push_back( 0 );
	ratio.back() = (TH1D*) inHist[iHist]->Clone();
	ratio.back()->Add( inHist[iHist-1], -1 );
	if ( mapOptionsInt["doRatio"] == 1 ) ratio.back()->Divide( inHist[iHist-1] );
	yTitle = ( mapOptionsInt["doRatio"]==1 ) ? "#frac{h_{2n+1}-h_{2n}}{h_{2n}}" : "h_{2n+1}-h_{2n}";
	break;
      default : 
	ratio.push_back( 0 );
	ratio.back() = (TH1D* ) inHist[iHist]->Clone();
	ratio.back()->Add( inHist[refHist], -1 );
	if ( mapOptionsInt["doRatio"] == 1 ) ratio.back()->Divide( inHist[refHist] );
	yTitle = ( mapOptionsInt["doRatio"]==1 ) ? "#frac{h_{n}-h_{0}}{h_{0}}" : "h_{n}-h_{0}";
      }
      if ( DEBUG ) cout << "ratio created" << endl;
      //Set graphics properties of first hitogram
      if ( !setTitle ) {
  	ratio.front()->GetXaxis()->SetTitle( inHist[refHist]->GetXaxis()->GetTitle() );
  	ratio.front()->GetXaxis()->SetLabelSize( 0.1 );
  	ratio.front()->GetXaxis()->SetTitleSize( 0.1 );
  	ratio.front()->GetYaxis()->SetLabelSize( 0.05 );
  	ratio.front()->GetYaxis()->SetTitleSize( 0.1 );
  	ratio.front()->GetYaxis()->SetTitleOffset( 0.3 );
  	ratio.front()->GetXaxis()->SetTitleOffset( 0.7 );
  	ratio.front()->SetTitle("");
        ratio.front()->GetYaxis()->SetTitle( yTitle.c_str() );
	setTitle = 1;
      }
      if ( DEBUG ) cout << "ratio front title done" << endl;
      //Update the values of Y axis range
      for ( int bin = 1; bin <= ratio.front()->GetNbinsX(); bin++ ) {
  	minValRatio = min( ratio.back()->GetBinContent(bin) - ratio.back()->GetBinError( bin), minValRatio );
  	maxValRatio = max( ratio.back()->GetBinContent(bin)+ ratio.back()->GetBinError( bin ), maxValRatio );
      }

    }// end iHist

    if ( ratio.size() ) {
      if ( DEBUG ) cout << "ratio ranges " << endl;
      //Plot all the ratio plots
      ratio.front()->GetYaxis()->SetRangeUser( minValRatio - (maxValRatio-minValRatio)*0.05, maxValRatio+(maxValRatio-minValRatio)*0.05 );
      if ( rangeUserX.size() == 2 ) ratio.front()->GetXaxis()->SetRangeUser( rangeUserX[0], rangeUserX[1] );
      else if ( mapOptionsInt["centerZoom"] ) ratio.front()->GetXaxis()->SetRangeUser( minX, maxX );
      //    if ( mapOptionsInt["centerZoom"] ) ratio.front()->GetXaxis()->SetRangeUser( inHist[refHist]->GetXaxis()->GetXmin(), inHist[refHist]->GetYaxis()->GetXmax() );
      if ( DEBUG ) cout << "plot ratio" << endl;
      for ( unsigned int iHist = 0; iHist < ratio.size(); iHist++ ) {
	ratio[iHist]->Draw( ( iHist ) ? "e,same" : "e" );
      }
      //Create a line at 0 to visualize deviations
      line->DrawLine( mapOptionsInt["centerZoom"] ? minX : ratio.front()->GetXaxis()->GetXmin(), 0, mapOptionsInt["centerZoom"] ? maxX :ratio.front()->GetXaxis()->GetXmax(), 0);
    }
    }//end doRatio

  if ( DEBUG ) cout << "saving" << endl;
  canvas.SaveAs( TString(outName) + ".pdf" );
  

  //========== CLEANING 
  delete line;
  return 0;
}


//===================================

