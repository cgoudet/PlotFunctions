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
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"

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

  map<string, int >  mapOptionsInt;
  mapOptionsInt["doRatio"]=0;
  mapOptionsInt["shiftColor"]=0;
  mapOptionsInt["normalize"]=0;
  mapOptionsInt["doChi2"]=0;
  mapOptionsInt["centerZoom"]=0;
  mapOptionsInt["drawStyle"]=0;
  mapOptionsInt["line"]=-99;
  map<string, double > mapOptionsDouble;
  mapOptionsDouble["extendUp"]=0;
  vector<string> inLegend, inLatex; 
  vector< vector< double > > latexPos;
  vector< double > legendCoord, rangeUserX, rangeUserY;

  for ( auto iOption : inOptions ) {

    string option = iOption.substr( 0, iOption.find_first_of('=' ) );
    string value = iOption.substr( iOption.find_first_of("=")+1);

    if ( mapOptionsInt.find(option) != mapOptionsInt.end() ) mapOptionsInt[option] = atoi( value.c_str() );
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
  vector< TH1D* > ratio;

  //================ PAD DEFINITION
  TCanvas canvas;
  if ( inHist.size()==1 && TString(inHist.front()->ClassName()).Contains("TH2") ) {
    canvas.SetRightMargin(0.1);
    inHist.front()->Draw( "COLZ" );
    for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
      if ( latexPos[iLatex].size() != 2 ) continue;
      myText( latexPos[iLatex][0], latexPos[iLatex][1], 1, inLatex[iLatex].c_str() );
    }
    canvas.SaveAs( TString(outName) + ".pdf" );
    return 0;
  }

  TPad padUp( "padUp", "padUp", 0, 0.3, 1, 1 );
  padUp.SetBottomMargin( 0 );
  //  padUp.SetLeftMargin( 0.07 );
  TPad padDown( "padDown", "padDown", 0, 0, 1, 0.3 );
  padDown.SetTopMargin( 0 );
  padDown.SetBottomMargin( 0.2 );
  //  padDown.SetLeftMargin( 0.07 );

  if ( mapOptionsInt["doRatio"] ) {
    padUp.Draw();
    canvas.cd();
    padDown.Draw();
    padUp.cd();
  }

  if ( !legendCoord.size() ) legendCoord={ 0.7, 0.9  };
  // TLegend *legend = new TLegend( legendCoord[0], legendCoord[1], legendCoord[2], legendCoord[3]);
  // legend->SetFillColorAlpha( 0, 0 );
  // legend->SetLineColorAlpha( 0, 0 );
  // legend->SetLineWidth(0);

  TLine *line = new TLine( 0, 0.005, 100, 0.005);
  line->SetLineColor( kBlack );
  line->SetLineStyle( 3 );


  //============ LOOP OTHER INPUT HIST
  //Find the extremum of the histograms to choose rangeUser if not given
  double minVal=0, maxVal=0;
  double minX=0, maxX=0;
  //  bool isNegativeValue = false;
  for ( unsigned int iHist = 0; iHist < inHist.size(); iHist++ ) {

    inHist[iHist]->UseCurrentStyle();
    if ( !iHist ) {
      string dumString = inHist.front()->GetXaxis()->GetTitle();
      ParseLegend( 0, dumString );
      inHist.front()->GetXaxis()->SetTitle( dumString.c_str() );
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
	inLegend[iHist] += " : chi2=" + TString::Format( "%2.2f", ComputeChi2( inHist[iHist], inHist.front() )/inHist.front()->GetNbinsX() );
      }
    }


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


  }//end iHist

  //Plotting histograms
  for ( unsigned int iHist = 0; iHist < inHist.size(); iHist++ ) {
    if ( !iHist ) {
      if (mapOptionsInt["doRatio"]) {
	inHist.front()->GetYaxis()->SetTitleOffset( 0.6 );
	inHist.front()->GetYaxis()->SetTitleSize( 0.06 );
      }

      if ( rangeUserY.size()!=2 ) {
	rangeUserY.clear();
	rangeUserY.push_back( minVal - ( maxVal - minVal ) *0.05 );
	rangeUserY.push_back( maxVal + ( maxVal - minVal ) *0.05 );
      }

      rangeUserY.back() += (rangeUserY.back() - rangeUserY.front()) * mapOptionsDouble["extendUp"];
      
      inHist.front()->GetYaxis()->SetRangeUser( rangeUserY[0], rangeUserY[1] );
      if ( rangeUserX.size() == 2 ) inHist.front()->GetXaxis()->SetRangeUser( rangeUserX[0], rangeUserX[1] );
      else if ( mapOptionsInt["centerZoom"] ) inHist.front()->GetXaxis()->SetRangeUser( minX, maxX );
    }
    string drawOpt = (iHist) ? "SAME,E" : "E";
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
    if( !iHist && mapOptionsInt["line"] != -99 ) {
      double rangeMin = rangeUserX.size()== 2 ? rangeUserX[0] : (mapOptionsInt["centerZoom"] ? minX : inHist.front()->GetXaxis()->GetXmin() );
      double rangeMax = rangeUserX.size()== 2 ? rangeUserX[1] : ( mapOptionsInt["centerZoom"] ? maxX :inHist.front()->GetXaxis()->GetXmax() );
      line->DrawLine( rangeMin , mapOptionsInt["line"], rangeMax, mapOptionsInt["line"]);
    }
    //========== ADD HISTOGRAM TO LEGEND
  }//end iHist

  // =========== PRINT LEGENDS AND LATEX

  for ( unsigned int iLegend=0; iLegend<inLegend.size(); iLegend++ ) {
    bool doFill = inLegend.size() > iLegend && TString( inLegend[iLegend].c_str() ).Contains( "__FILL" );
    ParseLegend( inHist[iLegend] , inLegend[iLegend] );
    if ( doFill )  myBoxText( legendCoord[0], legendCoord[1]-0.04*iLegend, 0.02, inHist[iLegend]->GetFillColor(), inLegend[iLegend].c_str() ); 
    else myMarkerText( legendCoord[0], legendCoord[1]-0.05*iLegend, inHist[iLegend]->GetMarkerColor(), inHist[iLegend]->GetMarkerStyle(), inLegend[iLegend].c_str(), 0.5 ); 
  }

  //  TLatex latex;
  for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
    if ( latexPos[iLatex].size() != 2 ) continue;
    bool doLabel = TString( inLatex[iLatex] ).Contains("__ATLAS");
    ParseLegend( 0, inLatex[iLatex] );
    if ( doLabel ) ATLASLabel( latexPos[iLatex][0], latexPos[iLatex][1], inLatex[iLatex].c_str() );
    else myText( latexPos[iLatex][0], latexPos[iLatex][1], 1, inLatex[iLatex].c_str() );
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
    if ( rangeUserX.size() == 2 ) ratio.front()->GetXaxis()->SetRangeUser( rangeUserX[0], rangeUserX[1] );
    else if ( mapOptionsInt["centerZoom"] ) ratio.front()->GetXaxis()->SetRangeUser( minX, maxX );
      //    if ( mapOptionsInt["centerZoom"] ) ratio.front()->GetXaxis()->SetRangeUser( inHist.front()->GetXaxis()->GetXmin(), inHist.front()->GetYaxis()->GetXmax() );
    for ( unsigned int iHist = 0; iHist < ratio.size(); iHist++ ) {
      ratio[iHist]->Draw( ( iHist ) ? "e,same" : "e" );
    }
    //Create a line at 0 to visualize deviations
    line->DrawLine( mapOptionsInt["centerZoom"] ? minX : ratio.front()->GetXaxis()->GetXmin(), 0, mapOptionsInt["centerZoom"] ? maxX :ratio.front()->GetXaxis()->GetXmax(), 0);
  }//end doRatio

  canvas.SaveAs( TString(outName) + ".pdf" );
  

  //========== CLEANING 
  //  delete legend;
  delete line;
  return 0;
}


//===================================

