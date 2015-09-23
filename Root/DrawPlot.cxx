#include <iostream>
#include "TCanvas.h"
#include "TLegend.h"
#include "TLine.h"
#include "TLatex.h"
#include "PlotFunctions/SideFunctions.h"

using std::cout;
using std::endl;
using std::min;
using std::max;

#include "PlotFunctions/DrawPlot.h"

void Style_Christophe();
int colors[] = {1, 2, 4, 6, 8, 28, 46};

int DrawPlot( vector< TH1* > inHist,  
	       string outName, 
	       vector< string> inLegend, 
	       unsigned int doRatio, 
	       bool normalize, 
	       bool doChi2, 
	       bool centerZoom , 
	       vector<double> rangeUser, 
	       vector<double> legendCoord,
	       vector<string> inLatex,
	      vector< vector< double > > latexPos,
	      unsigned int drawStyle
	       ) {
  cout << "DrawPlot" << endl;
  //================ SOME CHECKS
  if ( inLegend.size() && inLegend.size()!=inHist.size() ) {
    cout << "Legend do not match input" << endl;
    return  1;
  }
		 
  if ( inLatex.size() != latexPos.size() ) {
    cout << "Number of latex names and positions do not match" << endl;
    return 2;
  }

  if ( inHist.size() == 1 ) drawStyle = 0;

  Style_Christophe();


  if ( inHist.size() < 2 ) doRatio = 0;
  vector< TH1D* > ratio;

  //================ PAD DEFINITION
  TCanvas canvas( "canvas", "canvas", 1600, 1200);

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

  if ( doRatio ) {
    padUp.Draw();
    canvas.cd();
    padDown.Draw();
    padUp.cd();
  }

  if ( !legendCoord.size() ) legendCoord={ 0.7, 0.7, 0.99, 0.95  };
  TLegend *legend = new TLegend( legendCoord[0], legendCoord[1], legendCoord[2], legendCoord[3]);
  legend->SetFillColorAlpha( 0, 0 );
  legend->SetLineColorAlpha( 0, 0 );

  TLine *line = new TLine( 0, 0.005, 100, 0.005);
  line->SetLineColor( kBlack );
  line->SetLineStyle( 3 );


  //============ LOOP OTHER INPUT HIST
  //Find the extremum of the histograms to choose rangeUser if not given
  double minVal=0, maxVal=0, minX=0, maxX=0;
  bool isNegativeValue = false;
  for ( unsigned int iHist = 0; iHist < inHist.size(); iHist++ ) {

    if ( normalize && inHist[iHist]->Integral() )  inHist[iHist]->Scale( 1./inHist[iHist]->Integral() );

    //============ LOOK FOR Y EXTREMAL VALUES AND DEFINE Y RANGE
    if( !iHist ) {
      minVal = inHist.front()->GetMinimum();
      maxVal = inHist.front()->GetMaximum();
    }
    //Update the maximum range of the plot with extremum of current plot
    for ( int bin = 1; bin <= inHist[iHist]->GetNbinsX(); bin++ ) {
      minVal = min( inHist[iHist]->GetBinContent( bin ) - inHist[iHist]->GetBinError( bin ), minVal );
      maxVal = max( inHist[iHist]->GetBinContent( bin ) + inHist[iHist]->GetBinError( bin ), maxVal );
      if ( inHist[iHist]->GetBinContent( bin ) < 0 ) isNegativeValue = true;
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
    inHist[iHist]->SetLineColor( colors[ inHist.size()==1 ? 1 : iHist ] );
    inHist[iHist]->SetMarkerColor( colors[ inHist.size()==1 ? 1 : iHist ] );
    inHist[iHist]->SetMarkerStyle( 8 ); 
    inHist[iHist]->SetMarkerSize( 1 ); 

    //If only one histograms is plotted, plot it in red
    switch ( drawStyle ) {
    case 1 :
      inHist[iHist]->SetLineColor( colors[ iHist/2 ] );
      inHist[iHist]->SetMarkerColor( colors[ iHist/2 ] );
      inHist[iHist]->SetMarkerStyle( (iHist%2) ? 8 : 4 ); 
      break;
    }

    //======== CHI2 OF HISTOGRAM RATIOS
    if ( doChi2 && inLegend.size() && iHist ){
      switch ( drawStyle ) {
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
      inHist.front()->GetYaxis()->SetTitleOffset( 0.82 );
      inHist.front()->GetYaxis()->SetTitleSize( 0.06 );
      if ( rangeUser.size() == 2 ) inHist.front()->GetYaxis()->SetRangeUser( rangeUser[0], rangeUser[1] );
      else inHist.front()->GetYaxis()->SetRangeUser( isNegativeValue ? minVal - ( maxVal - minVal ) *0.05 : 0 , maxVal + ( maxVal - minVal ) *0.05 );
      if ( centerZoom ) inHist.front()->GetXaxis()->SetRangeUser( minX, maxX );    
    }
    inHist[iHist]->Draw( (iHist) ? "e,same" : "e" );

    //========== ADD HISTOGRAM TO LEGEND

    if ( inLegend.size() ) legend->AddEntry( inHist[iHist], inLegend[iHist].c_str(), "p" );
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
  if ( doRatio ) {
    padDown.cd();
 
    double minValRatio = 0;
    double maxValRatio = 0;
    for ( unsigned int iHist = 1; iHist < inHist.size(); iHist++ ) {
      string yTitle;

      //Decide how to pair histogram for ratio
      switch ( drawStyle ) {
      case 1 :
	if ( !(iHist % 2) ) continue;
	ratio.push_back( 0 );
	ratio.back() = (TH1D*) inHist[iHist]->Clone();
	ratio.back()->Add( inHist[iHist-1], -1 );
	if ( doRatio == 1 ) ratio.back()->Divide( inHist[iHist-1] );
	yTitle = ( doRatio==1 ) ? "#frac{h_{2n+1}-h_{2n}}{h_{2n}}" : "h_{2n+1}-h_{2n}";
	break;
      default : 
	ratio.push_back( 0 );
	ratio.back() = (TH1D*) inHist[iHist]->Clone();
	ratio.back()->Add( inHist.front(), -1 );
	if ( doRatio == 1 ) ratio.back()->Divide( inHist.front() );
	yTitle = ( doRatio==1 ) ? "#frac{h_{n}-h_{0}}{h_{0}}" : "h_{n}-h_{0}";
      }

      //Set graphics properties of first hitogram
      if ( iHist == 1 ) {
  	ratio.back()->GetXaxis()->SetTitle( inHist.front()->GetXaxis()->GetTitle() );
  	ratio.back()->GetXaxis()->SetLabelSize( 0.1 );
  	ratio.back()->GetXaxis()->SetTitleSize( 0.1 );
  	ratio.back()->GetYaxis()->SetLabelSize( 0.05 );
  	ratio.back()->GetYaxis()->SetTitleSize( 0.9 );
  	ratio.back()->GetYaxis()->SetTitleOffset( 0.25 );
  	ratio.back()->GetXaxis()->SetTitleOffset( 0.7 );
  	ratio.back()->SetTitle("");
  	if ( doRatio ==  1 ) ratio.back()->GetYaxis()->SetTitle( yTitle.c_str() );
      }

      //Update the values of Y axis range
      for ( int bin = 1; bin <= ratio.back()->GetNbinsX(); bin++ ) {
  	minValRatio = min( ratio.back()->GetBinContent(bin) - ratio.back()->GetBinError( bin), minValRatio );
  	maxValRatio = max( ratio.back()->GetBinContent(bin)+ ratio.back()->GetBinError( bin ), maxValRatio );
      }

    }// end iHist

    //Plot all the ratio plots
    ratio.front()->GetYaxis()->SetRangeUser( minValRatio - (maxValRatio-minValRatio)*0.05, maxValRatio+(maxValRatio-minValRatio)*0.05 );
    if ( centerZoom ) ratio.front()->GetXaxis()->SetRangeUser( minX, maxX );
    for ( unsigned int iHist = 0; iHist < ratio.size(); iHist++ ) {
      ratio[iHist]->Draw( ( iHist ) ? "e,same" : "e" );
    }
    //Create a line at 0 to visualize deviations
    line->DrawLine( centerZoom ? minX : ratio.front()->GetXaxis()->GetXmin(), 0, centerZoom ? maxX :ratio.front()->GetXaxis()->GetXmax(), 0);
  }//end doRatio

  canvas.SaveAs( TString(outName) + ".png" );
  

  //========== CLEANING 
  delete legend;
  delete line;
  return 0;
}


//===================================

