#include <iostream>
#include "TCanvas.h"
#include "TLegend.h"
#include "TLine.h"
#include "TLatex.h"
#include "PlotFunctions/SideFunctions.h"
#include <map>

using std::map;
using std::cout;
using std::endl;
using std::min;
using std::max;

#include "PlotFunctions/DrawPlot.h"

void Style_Christophe();
int colors[] = {1, 2, 4, 6, 8, 28, 46};
int fillColors[] = { 3, 5 };

int DrawPlot( vector< TH1* > inHist,  
	      string outName, 
	      vector<string> inOptions
	       ) {


  //================ SOME CHECKS

  map<string, int >  mapOptionsInt;
  mapOptionsInt["doRatio"]=0;
  mapOptionsInt["shiftColor"]=0;
  mapOptionsInt["normalize"]=0;
  mapOptionsInt["doChi2"]=0;
  mapOptionsInt["centerZoom"]=0;
  mapOptionsInt["drawStyle"]=0;
  mapOptionsInt["line"]=-99;
  vector<string> inLegend, inLatex; 
  vector< vector< double > > latexPos;
  vector< double > legendCoord, rangeUserX, rangeUserY;

  for ( auto iOption : inOptions ) {
    string option = iOption.substr( 0, iOption.find_first_of('=' ) );
    string value = iOption.substr( iOption.find_first_of("=")+1);

    if ( mapOptionsInt.find(option) != mapOptionsInt.end() ) mapOptionsInt[option] = atoi( value.c_str() );
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
      //      exit(0);
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
  Style_Christophe();
  vector< TH1D* > ratio;

  //================ PAD DEFINITION
  TCanvas canvas;

  if ( inHist.size()==1 && TString(inHist.front()->ClassName()).Contains("TH2") ) {
    canvas.SetRightMargin( 0.1 );
    inHist.front()->Draw( "COLZ" );
    canvas.SaveAs( TString(outName) + ".pdf" );
    return 0;
  }

  TPad padUp( "padUp", "padUp", 0, 0.3, 1, 1 );
  padUp.SetBottomMargin( 0 );
  padUp.SetLeftMargin( 0.07 );
  TPad padDown( "padDown", "padDown", 0, 0, 1, 0.3 );
  padDown.SetTopMargin( 0 );
  padDown.SetBottomMargin( 0.2 );
  padDown.SetLeftMargin( 0.07 );

  if ( mapOptionsInt["doRatio"] ) {
    padUp.Draw();
    canvas.cd();
    padDown.Draw();
    padUp.cd();
  }

  if ( !legendCoord.size() ) legendCoord={ 0.7, 0.95-inLegend.size()*0.05, 0.99, 0.95  };
  TLegend *legend = new TLegend( legendCoord[0], legendCoord[1], legendCoord[2], legendCoord[3]);
  legend->SetFillColorAlpha( 0, 0 );
  legend->SetLineColorAlpha( 0, 0 );
  legend->SetLineWidth(0);

  TLine *line = new TLine( 0, 0.005, 100, 0.005);
  line->SetLineColor( kBlack );
  line->SetLineStyle( 3 );


  //============ LOOP OTHER INPUT HIST
  //Find the extremum of the histograms to choose rangeUser if not given
  double minVal=0, maxVal=0, minX=0, maxX=0;
  //  bool isNegativeValue = false;
  for ( unsigned int iHist = 0; iHist < inHist.size(); iHist++ ) {

    if ( mapOptionsInt["normalize"] && inHist[iHist]->Integral() )  inHist[iHist]->Scale( 1./inHist[iHist]->Integral() );

    //============ LOOK FOR Y EXTREMAL VALUES AND DEFINE Y RANGE
    if( !iHist ) {
      minVal = inHist.front()->GetMinimum();
      maxVal = inHist.front()->GetMaximum();
    }
    //Update the maximum range of the plot with extremum of current plot
    for ( int bin = 1; bin <= inHist[iHist]->GetNbinsX(); bin++ ) {
      minVal = min( inHist[iHist]->GetBinContent( bin ) - inHist[iHist]->GetBinError( bin ), minVal );
      maxVal = max( inHist[iHist]->GetBinContent( bin ) + inHist[iHist]->GetBinError( bin ), maxVal );
      //  if ( inHist[iHist]->GetBinContent( bin ) < 0 ) isNegativeValue = true;
    }

    //========== LOOK FOR X EXTREMAL VALUES AND DEFINE X RANGE
    int lowBin = 1, upBin = inHist[iHist]->GetNbinsX();
    while ( inHist[iHist]->GetBinContent( lowBin ) == 0 && lowBin!=upBin ) lowBin++;
    while ( inHist[iHist]->GetBinContent( upBin ) ==0 && lowBin!=upBin ) upBin--;
    if ( lowBin != upBin ) {
      if ( !iHist || minX > inHist[iHist]->GetXaxis()->GetBinLowEdge( lowBin ) ) minX = inHist[iHist]->GetXaxis()->GetBinLowEdge( lowBin );
      if ( !iHist || maxX < inHist[iHist]->GetXaxis()->GetBinUpEdge( upBin ) ) maxX = inHist[iHist]->GetXaxis()->GetBinUpEdge( upBin );
    }

    //Set color and style of histogram
    //If only one histograms is plotted, plot it in red
    inHist[iHist]->SetLineColor(  colors[ max( 0, (int) ( (inHist.size()==1 ? 1 : iHist) + mapOptionsInt["shiftColor"])) ]  );
    inHist[iHist]->SetMarkerColor( colors[ max( 0, (int) ((inHist.size()==1 ? 1 : iHist) + mapOptionsInt["shiftColor"] ) )] );
    inHist[iHist]->SetMarkerStyle( 8 ); 
    inHist[iHist]->SetMarkerSize( 0.5 ); 

    //If only one histograms is plotted, plot it in red
    switch ( mapOptionsInt["drawStyle"] ) {
    case 1 :
      inHist[iHist]->SetLineColor( colors[ max( 0, (int) (iHist/2 +mapOptionsInt["shiftColor"] ) )] );
      inHist[iHist]->SetMarkerColor( colors[ max( 0 , (int) (iHist/2 + mapOptionsInt["shiftColor"]) ) ] );
      inHist[iHist]->SetMarkerStyle( (iHist%2) ? 8 : 4 ); 
      break;
    }

    //======== CHI2 OF HISTOGRAM RATIOS
    if ( mapOptionsInt["doChi2"] && inLegend.size() && iHist ){
      switch ( mapOptionsInt["drawStyle"] ) {
      case 1 : 
	if ( iHist % 2 ) inLegend[iHist] += " : chi2=" + TString::Format( "%2.2f", ComputeChi2( inHist[iHist], inHist[iHist-1] )/inHist[iHist]->GetNbinsX() );
	break;
      default :
	inLegend[iHist] += " : chi2=" + TString::Format( "%2.2f", ComputeChi2( inHist[iHist], inHist.front() )/inHist.front()->GetNbinsX() );
      }
    }
    if  ( inLegend.size() ) ParseLegend( inHist[iHist], inLegend[iHist] );    

  }//end iHist

  //Plotting histograms
  for ( unsigned int iHist = 0; iHist < inHist.size(); iHist++ ) {
    if ( !iHist ) {
      if (mapOptionsInt["doRatio"]) {
	inHist.front()->GetYaxis()->SetTitleOffset( 0.6 );
	inHist.front()->GetYaxis()->SetTitleSize( 0.06 );
      }

      if ( rangeUserY.size() == 2 ) inHist.front()->GetYaxis()->SetRangeUser( rangeUserY[0], rangeUserY[1] );
      else inHist.front()->GetYaxis()->SetRangeUser( minVal - ( maxVal - minVal ) *0.05 , maxVal + ( maxVal - minVal ) *0.05 );
      if ( rangeUserX.size() == 2 ) inHist.front()->GetXaxis()->SetRangeUser( rangeUserX[0], rangeUserX[1] );
      else if ( mapOptionsInt["centerZoom"] ) inHist.front()->GetXaxis()->SetRangeUser( minX, maxX );
    }
    string drawOpt = (iHist) ? "SAME,E" : "E";
    string legendOpt="";
    if ( inLegend.size() > iHist && TString( inLegend[iHist].c_str() ).Contains( "__NOPOINT" ) ) {
      inHist[iHist]->SetLineColorAlpha( 0, 0 );
      inHist[iHist]->SetMarkerColorAlpha( 0, 0 );
      inLegend[iHist]= TString(inLegend[iHist].c_str() ).ReplaceAll("__NOPOINT", "" );
    }
    else legendOpt+="p";

    if ( inLegend.size() > iHist && TString( inLegend[iHist].c_str() ).Contains( "__FILL" ) ) {
      inLegend[iHist]= TString(inLegend[iHist].c_str() ).ReplaceAll("__FILL", "" );
      drawOpt += "2";
      legendOpt+="f";
      inHist[iHist]->SetFillColor( fillColors[iHist] );
    }
    inHist[iHist]->Draw( drawOpt.c_str() );
    if( !iHist && mapOptionsInt["line"] != -99 ) {
      double rangeMin = rangeUserX.size()== 2 ? rangeUserX[0] : (mapOptionsInt["centerZoom"] ? minX : inHist.front()->GetXaxis()->GetXmin() );
      double rangeMax = rangeUserX.size()== 2 ? rangeUserX[1] : ( mapOptionsInt["centerZoom"] ? maxX :inHist.front()->GetXaxis()->GetXmax() );
      line->DrawLine( rangeMin , mapOptionsInt["line"], rangeMax, mapOptionsInt["line"]);
    }
    //========== ADD HISTOGRAM TO LEGEND
    if ( inLegend.size() > iHist && inLegend[iHist] != "" ) legend->AddEntry( inHist[iHist], inLegend[iHist].c_str() , legendOpt.c_str() );
  }//end iHist

  // =========== PRINT LEGENDS AND LATEX
  if ( inLegend.size() ) legend->Draw();
  TLatex latex;

  for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
    if ( latexPos[iLatex].size() < 2 ) continue;
    if ( latexPos[iLatex].size() > 2 ) latex.SetTextSize( latexPos[iLatex][2] );
    else   latex.SetTextSize(0.03);
    latex.DrawLatexNDC( latexPos[iLatex][0], latexPos[iLatex][1], inLatex[iLatex].c_str() );
  }

  //===============  CREATE RATIO PLOTS
  if ( mapOptionsInt["doRatio"] ) {
    padDown.cd();
 
    double minValRatio = 0;
    double maxValRatio = 0;
    for ( unsigned int iHist = 1; iHist < inHist.size(); iHist++ ) {
      string yTitle;

      //Decide how to pair histogram for ratio
      switch ( mapOptionsInt["drawStyle"] ) {
      case 1 :
	if ( !(iHist % 2) ) continue;
	ratio.push_back( 0 );
	ratio.back() = (TH1D*) inHist[iHist]->Clone();
	ratio.back()->Add( inHist[iHist-1], -1 );
	if ( mapOptionsInt["doRatio"] == 1 ) ratio.back()->Divide( inHist[iHist-1] );
	yTitle = ( mapOptionsInt["doRatio"]==1 ) ? "#frac{h_{2n+1}-h_{2n}}{h_{2n}}" : "h_{2n+1}-h_{2n}";
	break;
      default : 
	ratio.push_back( 0 );
	ratio.back() = (TH1D*) inHist[iHist]->Clone();
	ratio.back()->Add( inHist.front(), -1 );
	if ( mapOptionsInt["doRatio"] == 1 ) ratio.back()->Divide( inHist.front() );
	yTitle = ( mapOptionsInt["doRatio"]==1 ) ? "#frac{h_{n}-h_{0}}{h_{0}}" : "h_{n}-h_{0}";
      }

      //Set graphics properties of first hitogram
      if ( iHist == 1 ) {
  	ratio.back()->GetXaxis()->SetTitle( inHist.front()->GetXaxis()->GetTitle() );
  	ratio.back()->GetXaxis()->SetLabelSize( 0.1 );
  	ratio.back()->GetXaxis()->SetTitleSize( 0.1 );
  	ratio.back()->GetYaxis()->SetLabelSize( 0.05 );
  	ratio.back()->GetYaxis()->SetTitleSize( 0.1 );
  	ratio.back()->GetYaxis()->SetTitleOffset( 0.3 );
  	ratio.back()->GetXaxis()->SetTitleOffset( 0.7 );
  	ratio.back()->SetTitle("");
        ratio.back()->GetYaxis()->SetTitle( yTitle.c_str() );
      }

      //Update the values of Y axis range
      for ( int bin = 1; bin <= ratio.back()->GetNbinsX(); bin++ ) {
  	minValRatio = min( ratio.back()->GetBinContent(bin) - ratio.back()->GetBinError( bin), minValRatio );
  	maxValRatio = max( ratio.back()->GetBinContent(bin)+ ratio.back()->GetBinError( bin ), maxValRatio );
      }

    }// end iHist

    //Plot all the ratio plots
    ratio.front()->GetYaxis()->SetRangeUser( minValRatio - (maxValRatio-minValRatio)*0.05, maxValRatio+(maxValRatio-minValRatio)*0.05 );
    if ( mapOptionsInt["centerZoom"] ) ratio.front()->GetXaxis()->SetRangeUser( minX, maxX );
    for ( unsigned int iHist = 0; iHist < ratio.size(); iHist++ ) {
      ratio[iHist]->Draw( ( iHist ) ? "e,same" : "e" );
    }
    //Create a line at 0 to visualize deviations
    line->DrawLine( mapOptionsInt["centerZoom"] ? minX : ratio.front()->GetXaxis()->GetXmin(), 0, mapOptionsInt["centerZoom"] ? maxX :ratio.front()->GetXaxis()->GetXmax(), 0);
  }//end doRatio

  canvas.SaveAs( TString(outName) + ".pdf" );
  

  //========== CLEANING 
  delete legend;
  delete line;
  return 0;
}


//===================================

