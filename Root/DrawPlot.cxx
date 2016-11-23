#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"

#include "TCanvas.h"
#include "TLegend.h"
#include "TLine.h"
#include "TLatex.h"
#include <TROOT.h>
#include "THStack.h"
#include "RooPlot.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"
#include "RooSimultaneous.h"
#include "TF1.h"
#include "TObject.h"

#include <iostream>
#include <map>
#include <list>

using std::invalid_argument;
using std::runtime_error;
using std::map;
using std::cout;
using std::endl;
using std::min;
using std::max;
using std::list;

using namespace ChrisLib;
using namespace RooFit;

#define DEBUG 0
// enum EColor { kWhite =0,   kBlack =1,   kGray=920,
//               kRed   =632, kGreen =416, kBlue=600, kYellow=400, kMagenta=616, kCyan=432,
//               kOrange=800, kSpring=820, kTeal=840, kAzure =860, kViolet =880, kPink=900 };


int colors[] = {1, 
		632, 600, 616, 416, 800, 
		921,
		629, 597, 613, 413, 797, 
		635, 603, 619, 419, 807 };

int fillColors[] = { 3, 5 };

/**
   Draw a set of histograms on the same plots.
   \param inHist Vector of histograms.
   \param outName name of the output plot without the suffix (plot will be in .pdf).
   \param inOptions list of options using the convention : option=value.

   Supported options : \n
   - legend= value \n
   Content of the legend to use for a given histogram. 
   The number of legend options must be either 0 or the size of inHist.
   Legend accepts some special code detailed lower.
   \n
   \n
   - legendPos= valueX valueY \n
   Position in relative coordinates of the top left corner of the legend box.\n
   \n
   - rangeUserY(X)= valueMin valueMax \n
   Ranges to use for the Y(X) axis. \n
   \n
   doRatio= value \n
   Switch to a given type of ratio between plots of inHist. 
   0 : no ratio, 
   1 : (h1-h0)/h0,
   2 : h1-h0\n
   \n
   normalize=value \n
   Normalize all histograms to the integral given by value.\n
   \n
   doChi2=1 \n
   Compte the chi2 test between two histograms. The value will be added to the legend.\n
   \n
   centerZoom=1 \n
   Plot removing the empty bins at the extremities of all histograms. \n
   \n
   latex= text \n
   Add text to your plot with the latex convention of root. Special caracters must bu introduced with #
   Latex accept the special syntax detailed lower.
   The position of the text is defined by latexOpt of the same index.
   Several texts can be displayed using multiple occurences of the latex options.
   \n
   \n
   latexOpt=value1 value2\n
   Coordiantes in relative values of the top left corner of the latex box.
   \n
   \n
   drawStyle=value\n
   Decides how to group multiples histograms.
   0 : all histograms have different colors and same markers. Ratio is performed for each histogram relative to the first one.
   1 : histograms are group by pair. Each pair has a different color. Markers are different within a pair. Ratio is performed between histograms of the same pair.
   \n
   \n
   shiftColor=value\n
   Colors of histograms follow an order. shiftColor allows to change the origin of the color vector given above.
   \n
   \n
   line = value\n
   Draw an horizontal line at Y=value
   \n
   \n
   extendUp=value \n
   Increase the range on the Y axis by value%, at fixed lower range.
   \n
   \n
   x(y)Title= name \n
   change the title of the x(y) axis.
   \n
   \n
   logy=1 \n
   Set the Y axis to log scale
   \n
   \n
   stack=1\n
   Stack histograms instead of superimposing them. 
   Does not work with logy yet.

   Additional key woords can be put into the legend : \n
   __MEAN is replaced with the mean of the histogram
   __STDEV is replaced with the histogram standard deviation
   __FILL will modify the plotting options of histogram to fill between error bars
   __NOPOINT will make the histogram plotted without marker
   __HASHTAG is replaced with a # otherwise comment caracter
   __ENTRIES is replaced by the number of entries in the histo
   __INTEGRAL is replaced by the integral
 */


int ChrisLib::DrawPlot( vector< TH1* > &inHist,  
			string outName, 
			vector<string> inOptions
			) {

  //================ SOME CHECKS
  if ( DEBUG ) cout << "DrawPlot hist" << endl;

  map<string, int >  mapOptionsInt;
  map<string, double > mapOptionsDouble;
  vector<string> inLegend, inLatex; 
  vector< vector< double > > latexPos;
  vector< double > legendCoord, rangeUserX, rangeUserY;
  map<string, string> mapOptionsString;
  ReadOptions( inHist.size(), inOptions, mapOptionsDouble, mapOptionsInt, mapOptionsString,
	       inLegend, inLatex, latexPos, legendCoord, rangeUserX, rangeUserY );


  SetAtlasStyle();

  if ( DEBUG ) cout << "Options read" << endl;
  vector< TH1* > ratio;

  //================ PAD DEFINITION
  TCanvas canvas;
  canvas.SetBottomMargin( 0.2 );
  if ( inHist.size()==1 && inHist.front() && TString(inHist.front()->ClassName()).Contains("TH2") ) {
    canvas.SetRightMargin(0.1);
    inHist.front()->Draw( "COLZ" );
    for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
      if ( latexPos[iLatex].size() != 2 ) continue;

      myText( latexPos[iLatex][0], latexPos[iLatex][1], 1, inLatex[iLatex].c_str() );
    }
    string canOutName = outName + "." + mapOptionsString["extension"];
    canvas.SaveAs( canOutName.c_str() );
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

  TLine line( 0, 0.005, 100, 0.005);
  line.SetLineColor( kBlack );
  line.SetLineStyle( 3 );
  if ( DEBUG ) cout << "defined pads" << endl;

  //============ LOOP OTHER INPUT HIST
  //Find the extremum of the histograms to choose rangeUser if not given
  double minVal=0, maxVal=0;
  double minX=-0.99, maxX=0.99;
  vector<THStack*> stack;
  int refHist= -1;
  unsigned int totEventStack=0;

  if ( mapOptionsDouble["clean"] !=-99 ) CleanHist( inHist, mapOptionsDouble["clean"] );

  if ( DEBUG ) cout << "Cleaned" << endl;
  //  bool isNegativeValue = false;
  for ( unsigned int iHist = 0; iHist < inHist.size(); iHist++ ) {
    if ( !inHist[iHist] ) continue;
    if ( refHist == -1 ) refHist = iHist;
    inHist[iHist]->UseCurrentStyle();
    if ( (int) iHist == refHist ) {
      if ( mapOptionsString["xTitle"]== "" ) mapOptionsString["xTitle"] = inHist[refHist]->GetXaxis()->GetTitle();
      ParseLegend( mapOptionsString["xTitle"] );
      inHist[refHist]->GetXaxis()->SetTitle( mapOptionsString["xTitle"].c_str() );
      if ( mapOptionsString["yTitle"]!="" ) {
	ParseLegend( mapOptionsString["yTitle"] );
	inHist[refHist]->GetYaxis()->SetTitle( mapOptionsString["yTitle"].c_str() );
      }
      if ( DEBUG ) cout << "titles set" << endl;
    }
    //Set color and style of histogram
    //If only one histograms is plotted, plot it in red
    inHist[iHist]->SetLineColor(  colors[ max( 0, (int) ( (inHist.size()==1 ? 1 : iHist) + mapOptionsInt["shiftColor"])) ]  );
    inHist[iHist]->SetMarkerColor( inHist[iHist]->GetLineColor() );

    vector<string> functionNames = { "cubicFit", "quadraticFit" };
    TIter next(inHist[iHist]->GetListOfFunctions());
    while (TObject *obj = next()) {
      cout << obj->GetName() << endl;
      inHist[iHist]->GetFunction( obj->GetName() )->SetLineColor( inHist[iHist]->GetLineColor() );
    }


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
    if ( mapOptionsDouble["normalize"] && inHist[iHist]->Integral() && !mapOptionsInt["stack"] )  {
      inHist[iHist]->Sumw2();
      inHist[iHist]->Scale( mapOptionsDouble["normalize"]/inHist[iHist]->Integral() );
    }

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
     //initialize minX and maxX

     //widen x axis in nominal case
     if ( !mapOptionsInt["centerZoom"] ) {
       minX = minX==-0.99 ? inHist[iHist]->GetXaxis()->GetXmin() : min( minX, inHist[iHist]->GetXaxis()->GetXmin() );
       maxX = maxX==0.99  ? inHist[iHist]->GetXaxis()->GetXmax() :  max( maxX, inHist[iHist]->GetXaxis()->GetXmax() );
     }
     else {
       //get smaller interva in bin unit withoutextremal 0
       int lowBin = 1, upBin = inHist[iHist]->GetNbinsX();
       while ( inHist[iHist]->GetBinContent( lowBin ) == 0 && lowBin!=upBin ) lowBin++;
       while ( inHist[iHist]->GetBinContent( upBin ) ==0 && lowBin!=upBin ) upBin--;
       minX = minX==-0.99 ? inHist[iHist]->GetXaxis()->GetBinLowEdge( lowBin ) : min( minX, inHist[iHist]->GetXaxis()->GetBinLowEdge( lowBin ) );
       maxX = maxX==0.99 ? inHist[iHist]->GetXaxis()->GetBinUpEdge( upBin ) : max( maxX, inHist[iHist]->GetXaxis()->GetBinUpEdge( upBin ) );
     }

    if ( DEBUG ) cout << "X ranges defined" << endl;
  }//end iHist

  if ( DEBUG ) cout << "drawing" << endl;

  while ( rangeUserY.size() < 2 ) rangeUserY.push_back( pow(-1, rangeUserY.size()+1)*0.99 );
  if ( rangeUserY.front() == -0.99 ) rangeUserY.front() = minVal - ( maxVal - minVal ) *0.05;
  if ( rangeUserY.back() == 0.99 ) rangeUserY.back() = maxVal + ( maxVal - minVal ) *0.05;
  rangeUserY.back() += (rangeUserY.back() - rangeUserY.front()) * mapOptionsDouble["extendUp"];


  if ( rangeUserX.size() == 2 ) inHist[refHist]->GetXaxis()->SetRangeUser( rangeUserX[0], rangeUserX[1] );
  else {
    rangeUserX.clear();
    if ( minX==maxX ) maxX = minX+1;
    rangeUserX.push_back( minX );
    rangeUserX.push_back( maxX );
  }

  TH1F* dumHist = 0;
  if ( !strcmp( inHist[refHist]->GetXaxis()->GetBinLabel(1), "" ) ) {
    if ( mapOptionsInt["doRatio"] ) dumHist = padUp.DrawFrame( rangeUserX.front(), rangeUserY.front(), rangeUserX.back(), rangeUserY.back() );
    else dumHist = canvas.DrawFrame( rangeUserX.front(), rangeUserY.front(), rangeUserX.back(), rangeUserY.back() );
    dumHist->GetXaxis()->SetTitle( inHist[refHist]->GetXaxis()->GetTitle() );
    dumHist->GetYaxis()->SetTitle( inHist[refHist]->GetYaxis()->GetTitle() );

    if (mapOptionsInt["doRatio"]) {
      dumHist->GetYaxis()->SetTitleOffset( 0.6 );
      dumHist->GetYaxis()->SetTitleSize( 0.06 );
    }
  }
  else {
    inHist[refHist]->GetYaxis()->SetRangeUser( rangeUserY.front(), rangeUserY.back() );
  }

  //Plotting histograms
  for ( unsigned int iHist = refHist; iHist < inHist.size(); iHist++ ) {
    if ( !inHist[iHist] ) continue;

    string drawOpt = strcmp( inHist[refHist]->GetXaxis()->GetBinLabel(1), "" ) && (int)iHist==refHist ?  "" :"SAME,";
    switch ( mapOptionsInt["drawStyle"] ){
    case 2 : drawOpt += "HIST"; break;
    case 3 : drawOpt += "HISTL"; break;
    case 4 : 
      inHist[0]->SetMarkerStyle(8);
      inHist[1]->SetMarkerStyle(25);
      inHist[iHist]->SetMarkerSize(1.3);
      break;
    default : drawOpt += "E"; 
    }

    if ( inLegend.size() > iHist && TString( inLegend[iHist].c_str() ).Contains( "__NOPOINT" ) ) {
      inHist[iHist]->SetLineColorAlpha( 0, 0 );
      inHist[iHist]->SetMarkerColorAlpha( 0, 0 );
    }

    
    if ( inLegend.size() > iHist && TString( inLegend[iHist].c_str() ).Contains( "__FILL" ) ) {
      drawOpt += "2";
      inHist[iHist]->SetFillColor( fillColors[iHist] );
      //      myBoxText( legendCoord[0], legendCoord[1]-0.05*iHist, 0.05, inHist[iHist]->GetFillColor(), inLegend[iHist].c_str() ); 
    }

    //Added for PlotNotePub (Antinea)
    //    inHist[iHist]->SetLabelSize(0.05);
    //    inHist[iHist]->GetXaxis()->SetTitleOffset(1.7);
    //    inHist[0]->SetMarkerStyle(8);
    //inHist[1]->SetMarkerStyle(25);

    if ( !mapOptionsInt["stack" ] ) inHist[iHist]->Draw( drawOpt.c_str() );
    else {
      inHist[iHist]->SetFillColor( inHist[iHist]->GetLineColor() );
      if ( inLegend.size() && !TString(inLegend[iHist]).Contains( "__STACK" ) ) {
	if ( stack.size() ) {
	  if ( mapOptionsDouble["normalize"]!=0 ) RescaleStack( stack.back(), mapOptionsDouble["normalize"] );
	  stack.back()->Draw( stack.size()==1 ? "HIST B" : "HIST B same" );
	}
	totEventStack = 0;
	stack.push_back(0);
	stack.back() = new THStack();
      }
      stack.back()->Add( inHist[iHist] );
      totEventStack += inHist[iHist]->Integral();
      if ( iHist==inHist.size()-1 ) {
	if ( mapOptionsDouble["normalize"]!=0 ) RescaleStack( stack.back(), mapOptionsDouble["normalize"] );
	stack.back()->Draw( stack.size()==1 ? "HIST B" : "HIST B same" );
      }
    }


    if( !iHist && mapOptionsDouble["line"] != -99 ) {
      double rangeMin = rangeUserX.size()== 2 ? rangeUserX[0] : (mapOptionsInt["centerZoom"] ? minX : inHist[refHist]->GetXaxis()->GetXmin() );
      double rangeMax = rangeUserX.size()== 2 ? rangeUserX[1] : ( mapOptionsInt["centerZoom"] ? maxX :inHist[refHist]->GetXaxis()->GetXmax() );
      line.DrawLine( rangeMin , mapOptionsDouble["line"], rangeMax, mapOptionsDouble["line"]);
    }
    //========== ADD HISTOGRAM TO LEGEND
  }//end iHist

  if ( DEBUG ) cout << "drawn" << endl;
  //  stack.Draw( mapOptionsInt["stack"] ? "F" : "nostack"  ); 

  if ( mapOptionsInt["logy"] ) {
    int topVal = ceil( log10( maxVal ) );
    int lowVal = minVal==0 ? topVal-5 : floor( log10( minVal ) );
    if ( rangeUserY[0] < 0 ) rangeUserY[0]=pow( 10, lowVal );
    rangeUserY[1] = pow( 10, topVal + ( topVal - lowVal ) * (0.05 + mapOptionsDouble["extendUp"] ) );
    if ( mapOptionsDouble["stack"] == 0 ) inHist[refHist]->GetYaxis()->SetRangeUser( rangeUserY[0], rangeUserY[1] );    
    else {
      stack.front()->SetMinimum( rangeUserY[0] );
      stack.front()->SetMaximum( rangeUserY[1] );
      stack.front()->Draw();
    }
    if ( mapOptionsInt["doRatio"] ) {
      padUp.SetLogy(1);
    }
    else {
      canvas.SetLogy(1);
    }
    if ( DEBUG ) cout << "logy done" << endl;
  }

  // =========== PRINT LEGENDS AND LATEX
  canvas.cd();
  for ( unsigned int iLegend=0; iLegend<inLegend.size(); iLegend++ ) {
    if ( !inHist[iLegend] ) continue;
    bool doFill = inLegend.size() > iLegend && TString( inLegend[iLegend].c_str() ).Contains( "__FILL" );
    ParseLegend( inHist[iLegend] , inLegend[iLegend] );
    if ( doFill )  myBoxText( legendCoord[0], legendCoord[1]-0.04*iLegend, inHist[iLegend]->GetFillColor(), inLegend[iLegend].c_str() ); 
    else if ( mapOptionsInt["drawStyle"] ) myMarkerText( legendCoord[0], legendCoord[1]-0.05*iLegend, inHist[iLegend]->GetMarkerColor(), inHist[iLegend]->GetMarkerStyle(), inLegend[iLegend].c_str()  ); 
    else myLineText( legendCoord[0], legendCoord[1]-0.05*iLegend, inHist[iLegend]->GetLineColor(), inHist[iLegend]->GetLineStyle(), inLegend[iLegend].c_str()  ); 
    if (mapOptionsInt["drawStyle"]==4)
      {
	myLineText( legendCoord[0]-0.005, legendCoord[1]-0.05*iLegend, inHist[iLegend]->GetLineColor(), inHist[iLegend]->GetLineStyle(), ""  ); 
	myMarkerText( legendCoord[0], legendCoord[1]-0.05*iLegend, inHist[iLegend]->GetMarkerColor(), inHist[iLegend]->GetMarkerStyle(), inLegend[iLegend].c_str()  );
      }
  }
  if ( DEBUG )  cout << "legend drawn" << endl;
  for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
    if ( latexPos[iLatex].size() != 2 ) continue;
    bool doLabel = TString( inLatex[iLatex] ).Contains("__ATLAS");
    ParseLegend( inLatex[iLatex] );
    if ( doLabel ) ATLASLabel( latexPos[iLatex][0], latexPos[iLatex][1], inLatex[iLatex].c_str(),1 , 0.04 );
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
	//  	ratio.front()->GetYaxis()->SetLabelSize( 0.05 );
  	ratio.front()->GetYaxis()->SetLabelSize( 0.08 );
  	ratio.front()->GetYaxis()->SetTitleSize( 0.1 );
  	ratio.front()->GetYaxis()->SetTitleOffset( 0.5 );
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
      if ( DEBUG ) cout << "plot ratio" << endl;
      for ( unsigned int iHist = 0; iHist < ratio.size(); iHist++ ) {
	ratio[iHist]->Draw( ( iHist ) ? "e,same" : "e" );
      }
      //Create a line at 0 to visualize deviations
      line.DrawLine( mapOptionsInt["centerZoom"] ? minX : ratio.front()->GetXaxis()->GetXmin(), 0, mapOptionsInt["centerZoom"] ? maxX :ratio.front()->GetXaxis()->GetXmax(), 0);
    }
    }//end doRatio

  if ( DEBUG ) cout << "saving" << endl;
  string canOutName = outName + "." + mapOptionsString["extension"];
  canvas.SaveAs( canOutName.c_str() );

  //  canvas.SaveAs( TString(outName) + ".pdf" );
  //  canvas.SaveAs( TString(outName) + ".root" );
  

  //========== CLEANING 
  //  
  return 0;
}


//===================================

int ChrisLib::DrawPlot( RooRealVar *frameVar,
	      vector<TObject*> inObj,
	      string outName,
	      vector<string> inOptions
	      ) {
  //  cout << "DrawPlot frame" << endl;

  vector<string> inLegend, inLatex; 
  vector< vector< double > > latexPos;
  vector< double > legendCoord, rangeUserX, rangeUserY;
  map<string, int> mapOptionsInt;
  mapOptionsInt["nComparedEvents"]=100;
  map<string, double> mapOptionsDouble;
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

  if ( inLegend.size() && inLegend.size()!=inObj.size() ) { cout << "Legend do not match input" << endl;return  1;}
  if ( inLatex.size() != latexPos.size() ) {cout << "Number of latex names and positions do not match" << endl << inLatex.size() << " " << latexPos.size() << endl; return 2; }

  SetAtlasStyle();

  if ( DEBUG ) cout << "Options read" << endl;

  SetAtlasStyle();
  TCanvas *canvas = new TCanvas();
  if ( rangeUserX.size() == 2 ) frameVar->setRange( rangeUserX.front(), rangeUserX.back() );

  RooPlot* frame=frameVar->frame(mapOptionsInt["nComparedEvents"]);
  frame->SetTitle(""); //empty title to prevent printing "A RooPlot of ..."
  frame->SetXTitle(frameVar->GetTitle());

  vector<map<string,int>> legendInfo;
  for ( unsigned int iPlot=0; iPlot<inObj.size(); iPlot++ ) {
    legendInfo.push_back( map<string, int>());
    legendInfo.back()["color"] = colors[iPlot];
    if ( string(inObj[iPlot]->ClassName() ) == "RooDataSet" ) {
      //      ( (RooDataSet*) inObj[iPlot])->Print();
      ( (RooDataSet*) inObj[iPlot])->plotOn( frame, LineColor(  colors[iPlot] ), DataError( RooAbsData::SumW2 ) );
      legendInfo.back()["doLine"] = 0;
      legendInfo.back()["style"] = frame->getAttLine(frame->getObject(iPlot)->GetName())->GetLineStyle();
    }
    else {
      //      ( (RooAbsPdf*) inObj[iPlot])->Print();
      ( (RooAbsPdf*) inObj[iPlot])->plotOn( frame, LineColor(  colors[iPlot] ) );
      legendInfo.back()["doLine"] = 1;
      legendInfo.back()["style"] = frame->getAttMarker(frame->getObject(iPlot)->GetName())->GetMarkerStyle();
    }
  }

  frame->Draw();
  for ( unsigned int iPlot=0; iPlot<inObj.size(); iPlot++ ) {
    if ( legendInfo[iPlot]["doLine"] )    myMarkerText( 0.7, 0.9-0.05*iPlot, legendInfo[iPlot]["color"], legendInfo[iPlot]["style"], inLegend.size() ? inLegend[iPlot].c_str() : "" ); 
    else  myLineText( 0.7, 0.9-0.05*iPlot, legendInfo[iPlot]["color"], legendInfo[iPlot]["style"], inLegend.size() ? inLegend[iPlot].c_str() : "" ); 

  }

  for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
    if ( latexPos[iLatex].size() != 2 ) continue;
    bool doLabel = TString( inLatex[iLatex] ).Contains("__ATLAS");
    ParseLegend( inLatex[iLatex] );
    if ( doLabel ) ATLASLabel( latexPos[iLatex][0], latexPos[iLatex][1], inLatex[iLatex].c_str() );
    else myText( latexPos[iLatex][0], latexPos[iLatex][1], 1, inLatex[iLatex].c_str() );
  }

  string canOutName = outName + "." + mapOptionsString["extension"];
  canvas->SaveAs( canOutName.c_str() );

  //  canvas->SaveAs( TString( outName + ".pdf") );
  delete frame;
  delete canvas; canvas=0;
  return 0;
}

//================================================
vector<string> ChrisLib::PlotPerCategory( vector<TObject*> vectObj, RooCategory *cat, string prefix, vector<string> options ) {

  vector<string> plotNames;

  for ( int iCat = 0; iCat < cat->numTypes(); iCat++ ) {
    RooRealVar *varFrame=0;
    vector<string> tmpOptions( options );
    cat->setIndex( iCat );
    tmpOptions.push_back( "latex=" + string( cat->getLabel() ) );
    tmpOptions.push_back( "latexOpt=0.18 0.9" );
    vector<TObject*> outVectObj;
    for ( unsigned int iObj=0; iObj<vectObj.size(); iObj++ ) {

      if ( string( vectObj[iObj]->ClassName() ) == "RooDataSet" ) {
	RooAbsData* ds=0;
	//	vectObj[iObj]->Print();
	TIterator* dataItr = ((RooDataSet*) vectObj[iObj])->split(*cat, true)->MakeIterator();
	while ((ds = (RooAbsData*)dataItr->Next())) { // loop over all channels
	  if ( string( ds->GetName() ) != cat->getLabel() ) continue;
	  //	  ds->Print();
	  outVectObj.push_back( ds );

	  TIterator* iter = ds->get()->createIterator();
	  RooAbsPdf* parg;
	  while((parg=(RooAbsPdf*)iter->Next()) ) {
	    if ( TString( parg->GetName() ).Contains( ds->GetName() ) ) {
	      varFrame = (RooRealVar*) parg;
	      break;
	    }

	  }
	  break;
	}
      }//end if data
      else if ( string( vectObj[iObj]->ClassName() ) == "RooSimultaneous" ) {
	RooSimultaneous *pdf = (RooSimultaneous*) vectObj[iObj];
	if ( !pdf->getPdf( cat->getLabel() ) ) continue;
	outVectObj.push_back( pdf->getPdf( cat->getLabel() ) );
      }//end if roosimultaneous
      else {
	cout << vectObj[iObj]->ClassName() << " type not planned" << endl;
	exit(0);
      }
    }//end iObj

    string name = prefix + "_" + cat->getLabel();
    DrawPlot( varFrame, outVectObj, name , tmpOptions );
    plotNames.push_back( name );
  }//end iCat
  return plotNames;
}

//================================================
int ChrisLib::DrawPlot( vector< TGraphErrors* > &inGraph,  
	      string outName, 
	      vector<string> inOptions
	       ) {

  //================ SOME CHECKS
  if ( DEBUG ) cout << "DrawPlot" << endl;

  map<string, int >  mapOptionsInt;
  map<string, double > mapOptionsDouble;
  vector<string> inLegend, inLatex; 
  vector< vector< double > > latexPos;
  vector< double > legendCoord, rangeUserX, rangeUserY;
  map<string, string> mapOptionsString;
  ReadOptions( inGraph.size(), inOptions, mapOptionsDouble, mapOptionsInt, mapOptionsString,
	       inLegend, inLatex, latexPos, legendCoord, rangeUserX, rangeUserY );
  SetAtlasStyle();
  if ( DEBUG ) cout << "Options read" << endl;

  // //================ PAD DEFINITION
  TCanvas canvas;
  if ( !legendCoord.size() ) legendCoord={ 0.7, 0.9  };

  TLine line( 0, 0.005, 100, 0.005);
  line.SetLineColor( kBlack );
  line.SetLineStyle( 3 );
  if ( DEBUG ) cout << "defined pads" << endl;

  //============ LOOP OTHER INPUT HIST
  //Find the extremum of the histograms to choose rangeUser if not given
  double minVal=0, maxVal=0;
  double minX=0, maxX=0;
  int refGraph= -1;

  for ( unsigned int iGraph = 0; iGraph < inGraph.size(); iGraph++ ) {
    if ( !inGraph[iGraph] ) continue;
    if ( refGraph == -1 ) refGraph = iGraph;
    inGraph[iGraph]->UseCurrentStyle();
    if ( (int) iGraph == refGraph ) {
      if ( mapOptionsString["xTitle"]== "" ) mapOptionsString["xTitle"] = inGraph[refGraph]->GetXaxis()->GetTitle();
      ParseLegend(  mapOptionsString["xTitle"] );
      inGraph[refGraph]->GetXaxis()->SetTitle( mapOptionsString["xTitle"].c_str() );
      if ( mapOptionsString["yTitle"]!="" ) {
  	ParseLegend( mapOptionsString["yTitle"] );
  	inGraph[refGraph]->GetYaxis()->SetTitle( mapOptionsString["yTitle"].c_str() );
      }
      if ( DEBUG ) cout << "titles set" << endl;
    }
    //Set color and style of histogram
    //If only one histograms is plotted, plot it in red
    switch ( mapOptionsInt["drawStyle"] ) {
    case 1 :
      inGraph[iGraph]->SetLineColor( colors[ max( 0, (int) (iGraph/2 +mapOptionsInt["shiftColor"] ) )] );
      inGraph[iGraph]->SetMarkerColor( colors[ max( 0 , (int) (iGraph/2 + mapOptionsInt["shiftColor"]) ) ] );
      inGraph[iGraph]->SetMarkerStyle( (iGraph%2) ? 4 : 8 ); 
      break;
    default :
      inGraph[iGraph]->SetLineColor(  colors[ max( 0, (int) ( (inGraph.size()==1 ? 1 : iGraph) + mapOptionsInt["shiftColor"])) ]  );
      inGraph[iGraph]->SetMarkerColor( colors[ max( 0, (int) ((inGraph.size()==1 ? 1 : iGraph) + mapOptionsInt["shiftColor"] ) )] );
      inGraph[iGraph]->SetFillColor( inGraph[iGraph]->GetLineColor() );
      inGraph[iGraph]->SetMarkerStyle( 4  ); 
    }

    //============= RESCALE THE GRAPH IF OFFSET

    //============ LOOK FOR Y EXTREMAL VALUES AND DEFINE Y RANGE
    //Update the maximum range of the plot with extremum of current plot
    double minY=0;
    for ( int bin = 0; bin < inGraph[iGraph]->GetN(); bin++ ) {
      double x, y;
      inGraph[iGraph]->GetPoint( bin, x, y );
      minY = ( !bin && (int)iGraph==refGraph ) ? y : min( y, minY );
    }

    if ( mapOptionsInt["offset"] ) { 
      for ( int bin = 0; bin < inGraph[iGraph]->GetN(); bin++ ) {
	double x, y;
	inGraph[iGraph]->GetPoint( bin, x, y );
	inGraph[iGraph]->SetPoint( bin, x, y-minY );
      }
    }

    if ( (int)iGraph==refGraph ) {
      inGraph[iGraph]->GetPoint(0, minX, minVal );
      inGraph[iGraph]->GetPoint(0, maxX, maxVal );
    }
    for ( int bin = 0; bin < inGraph[iGraph]->GetN(); bin++ ) {
      double x, y;
      inGraph[iGraph]->GetPoint( bin, x, y );
      minVal =  min( y - inGraph[iGraph]->GetErrorY( bin ), minVal );
      maxVal =  max( y + inGraph[iGraph]->GetErrorY( bin ), maxVal );
      minX =  min( x - inGraph[iGraph]->GetErrorX( bin ), minX );
      maxX =  max( x + inGraph[iGraph]->GetErrorX( bin ), maxX );
    }
    if ( DEBUG ) cout << "extremal Y and Y values defined and set " << endl;

    if ( mapOptionsInt["orderX"] ) inGraph[iGraph]->Sort();

    //========== LOOK FOR X EXTREMAL VALUES AND DEFINE X RANGE
  }//end iGraph

  if ( DEBUG ) cout << "drawing" << endl;
  while ( rangeUserY.size() < 2 ) rangeUserY.push_back( pow(-1, rangeUserY.size()+1)*0.99 );
  if ( rangeUserY.front() == -0.99 ) rangeUserY.front() = minVal - ( maxVal - minVal ) *0.05;
  if ( rangeUserY.back() == 0.99 ) rangeUserY.back() = maxVal + ( maxVal - minVal ) *0.05;
  rangeUserY.back() += (rangeUserY.back() - rangeUserY.front()) * mapOptionsDouble["extendUp"];
  if ( rangeUserX.size() == 2 ) inGraph[refGraph]->GetXaxis()->SetRangeUser( rangeUserX[0], rangeUserX[1] );
  else {
    rangeUserX.clear();
    rangeUserX.push_back( minX );
    rangeUserX.push_back( maxX );
  }

  TH1F* dumHist = 0;
  dumHist = canvas.DrawFrame( rangeUserX.front(), rangeUserY.front(), rangeUserX.back(), rangeUserY.back() );
  dumHist->GetXaxis()->SetTitle( inGraph[refGraph]->GetXaxis()->GetTitle() );
  dumHist->GetYaxis()->SetTitle( inGraph[refGraph]->GetYaxis()->GetTitle() );
  
  //Plotting histograms
  for ( unsigned int iGraph = refGraph; iGraph < inGraph.size(); iGraph++ ) {
    if ( !inGraph[iGraph] ) continue;

    string drawOpt =  "SAME,L";
    switch ( mapOptionsInt["drawStyle"] ){
    default : drawOpt += "E"; 
    }

    if ( inLegend.size() > iGraph && TString( inLegend[iGraph].c_str() ).Contains( "__NOPOINT" ) ) {
      inGraph[iGraph]->SetLineColorAlpha( 0, 0 );
      inGraph[iGraph]->SetMarkerColorAlpha( 0, 0 );
    }

    inGraph[iGraph]->Draw( drawOpt.c_str() );

    if( !iGraph && mapOptionsDouble["line"] != -99 ) {
      double rangeMin = rangeUserX.size()== 2 ? rangeUserX[0] : (mapOptionsInt["centerZoom"] ? minX : inGraph[refGraph]->GetXaxis()->GetXmin() );
      double rangeMax = rangeUserX.size()== 2 ? rangeUserX[1] : ( mapOptionsInt["centerZoom"] ? maxX :inGraph[refGraph]->GetXaxis()->GetXmax() );
      line.DrawLine( rangeMin , mapOptionsInt["line"], rangeMax, mapOptionsInt["line"]);
    }
    //========== ADD HISTOGRAM TO LEGEND
  }//end iGraph

  if ( DEBUG ) cout << "drawn" << endl;

  if ( mapOptionsInt["logy"] ) {
    int topVal = ceil( log10( maxVal ) );
    int lowVal = minVal==0 ? topVal-5 : floor( log10( minVal ) );
    if ( rangeUserY[0] < 0 ) rangeUserY[0]=pow( 10, lowVal );
    rangeUserY[1] = pow( 10, topVal + ( topVal - lowVal ) * (0.05 + mapOptionsDouble["extendUp"] ) );
    inGraph[refGraph]->GetYaxis()->SetRangeUser( rangeUserY[0], rangeUserY[1] );    
    canvas.SetLogy(1);
    if ( DEBUG ) cout << "logy done" << endl;
  }

  // =========== PRINT LEGENDS AND LATEX
  for ( unsigned int iLegend=0; iLegend<inLegend.size(); iLegend++ ) {
    if ( !inGraph[iLegend] ) continue;
    ParseLegend( inGraph[iLegend] , inLegend[iLegend] );
    //    myMarkerText( legendCoord[0], legendCoord[1]-0.05*iLegend, inGraph[iLegend]->GetMarkerColor(), inGraph[iLegend]->GetMarkerStyle(), inLegend[iLegend].c_str() ); 
    myLineText( legendCoord[0], legendCoord[1]-0.05*iLegend, inGraph[iLegend]->GetLineColor(), inGraph[iLegend]->GetLineStyle(), inLegend[iLegend].c_str() ); 
  }
  if ( DEBUG )  cout << "legend drawn" << endl;
  for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
    if ( latexPos[iLatex].size() != 2 ) continue;
    bool doLabel = TString( inLatex[iLatex] ).Contains("__ATLAS");
    ParseLegend( inLatex[iLatex] );
    if ( doLabel ) ATLASLabel( latexPos[iLatex][0], latexPos[iLatex][1], inLatex[iLatex].c_str() );
    else myText( latexPos[iLatex][0], latexPos[iLatex][1], 1, inLatex[iLatex].c_str() );
  }
  if ( DEBUG ) cout << "latex drawn" << endl;


  if ( DEBUG ) cout << "saving" << endl;
  string canOutName = outName + "." + mapOptionsString["extension"];
  cout << "canOutName : " << canOutName << endl;
  cout << "extensions : " << mapOptionsString["extension"] << endl;
  canvas.SaveAs( canOutName.c_str() );

  //  canvas.SaveAs( TString(outName) + ".pdf" );
  

  //========== CLEANING 
  
  return 0;
}

//==================================================
void ChrisLib::ReadOptions( unsigned nHist, 
		  const vector<string> &inOptions,
		  map<string,double> &mapDouble,
		  map<string,int> &mapInt,
		  map<string,string> &mapString,
		  vector<string> &inLegend,
		  vector<string> &inLatex,
		  vector<vector<double>> &latexPos,
		  vector<double> &legendCoord,
		  vector<double> &rangeUserX,
		  vector<double> &rangeUserY
		  ) {
  
  if ( DEBUG ) cout << "ChrisLib::ReaOptions" << endl;

  list<string> allowedInt = { "doRatio", "shiftColor", "doChi2", "centerZoom", "drawStyle", "logy", "stack", "offset", "orderX", "grid" };
  for ( auto it=allowedInt.begin(); it!=allowedInt.end(); ++it ) mapInt[*it]=0;

  mapDouble["extendUp"]=0;
  mapDouble["normalize"]=0;
  mapDouble["line"]=-99;
  mapDouble["clean"]=-99;

  mapString["xTitle"]="";
  mapString["yTitle"]="";
  mapString["extension"]="pdf";

  for ( auto iOption : inOptions ) {
    string option = iOption.substr( 0, iOption.find_first_of('=' ) );
    string value = iOption.substr( iOption.find_first_of("=")+1);
    if ( mapInt.find(option) != mapInt.end() ) mapInt[option] = atoi( value.c_str() );
    else if ( mapString.find(option) != mapString.end() ) mapString[option] = value;
    else if ( mapDouble.find(option) != mapDouble.end() ) {
      mapDouble[option] = std::stod( value.c_str() );
    }
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
      cout << "DrawPlotOption : " << option << " not known" << endl;
    }
  }

  if ( inLegend.size() && inLegend.size()!=nHist ) throw invalid_argument( "ChrisLib::ReadOptions : Legend do not match input" );
  if ( inLatex.size() != latexPos.size() ) throw invalid_argument( "ChrisLib::ReadOptions : Number of latex names and positions do not match" );

  if ( nHist == 1 ) mapInt["drawStyle"] = 0;
  if ( nHist < 2 ) mapInt["doRatio"] = 0;

  list<string> allowedExtension = { "pdf", "root", "png" };
  if ( find(allowedExtension.begin(), allowedExtension.end(), mapString["extension"] ) == allowedExtension.end() ) throw runtime_error( "DrawPlot : Wrong output file extension provided" );

  if ( DEBUG ) cout << "ChrisLib::ReaOptions end" << endl;
}
//==============================================
void SetHistProperties( TH1* hist ) {
  vector<string> functionNames = { "cubicFit", "quadraticFit" };
  TIter next(hist->GetListOfFunctions());
  while (TObject *obj = next()) {
    hist->GetFunction( obj->GetName() )->SetLineColor( hist->GetLineColor() );
  }
  
}
//================================================
bool IsHist( TObject* obj ) {
  if (  string(obj->ClassName()) == "TGraphErrors" ) return false;
  else return true;
}
//==============================================
void SetProperties( TObject* obj, map<string,int> &mapInt, map<string,string> &mapString, int iHist ) {
  TH1* hist=0;
  TGraphErrors *graph=0;
  if (  !IsHist( obj ) ) graph = static_cast<TGraphErrors*>(obj);
  else hist=static_cast<TH1*>(obj);
  
  if ( !iHist ) {
    for ( unsigned iAxis=0; iAxis<2; ++iAxis ) {
      map<string,string>::iterator title = mapString.find( string(iAxis?"y":"x") + "Title" );
      if ( title->second != "" ) {
	ParseLegend( title->second );
	TAxis *axis = 0;
	if ( hist ) axis = iAxis ? hist->GetYaxis() : hist->GetXaxis();
	else axis = iAxis ? graph->GetYaxis() : graph->GetXaxis();
	axis->SetTitle( title->second.c_str() );
      }
    }
    if ( graph && mapInt["orderX"] ) graph->Sort();


  }

  //If only one histograms is plotted, plot it in red
  TAttLine *attLine= hist ? static_cast<TAttLine*>(hist) : static_cast<TAttLine*>(graph);
  TAttMarker *attMarker = hist ? static_cast<TAttMarker*>(hist) : static_cast<TAttMarker*>(graph);
  attMarker->SetMarkerSize( 0.5 );
  switch ( mapInt["drawStyle"] ) {
  case 1 :
    attLine->SetLineColor( colors[ max( 0, iHist/2 +mapInt["shiftColor"] )] );
    attMarker->SetMarkerColor( colors[ max( 0 , (int) (iHist/2 + mapInt["shiftColor"]) ) ] );
    attMarker->SetMarkerStyle( (iHist%2) ? 4 : 8 ); 
    break;
  default :
    attLine->SetLineColor( colors[iHist+mapInt["shiftColor"]] );
    attMarker->SetMarkerColor( colors[ iHist + mapInt["shiftColor"] ] );
    attMarker->SetMarkerStyle( 8 ); 
  }

  if ( hist ) SetHistProperties( hist );

}
//==============================================
void GetMaxValue( TObject *obj, double &minVal, double &maxVal, double &minX, double &maxX, bool takeError, bool isRef ) {

  TH1* hist=0;
  TGraphErrors *graph=0;
  if (  !IsHist( obj ) ) graph = static_cast<TGraphErrors*>(obj);
  else hist=static_cast<TH1*>(obj);

  if ( isRef ) {
    if ( hist ) {
      minVal = hist->GetBinContent(1);
      minX = hist->GetXaxis()->GetXmin();
    }
    else graph->GetPoint( 0, minX, minVal );
    maxVal=minVal;
    maxX = minX;
  }

  if ( hist ) {
    minX = min( minX, hist->GetXaxis()->GetXmin() );
    maxX = max( maxX, hist->GetXaxis()->GetXmax() );
  }

  //Update the maximum range of the plot with extremum of current plot
  int nBins = hist ? hist->GetNbinsX() : graph->GetN();
  for ( int bin = 0; bin < nBins; ++bin ) {
      double val = 0, err=0;
      if ( hist ) {
	val = hist->GetBinContent( bin+1 );
	if ( takeError ) err = hist->GetBinError( bin+1 );
      }
      else {
	double xGraph=0;
	graph->GetPoint(bin, xGraph, val);
	if ( takeError ) err = graph->GetErrorY(bin);
	double errX = takeError ? graph->GetErrorX(bin) : 0;
	minX = min( minX, xGraph - errX );
	maxX = max( maxX, xGraph + errX );
      }
      minVal = min( val - err , minVal );
      maxVal = max( val + err , maxVal );
    }
}
//==============================================
void DrawText( vector<TObject*> &inHist, 
	       vector<string> &inLegend, 
	       vector<string> &inLatex,
	       const map<string,int> &mapOptionsInt,
	       const vector<double> &legendCoord,
	       const vector<vector<double>> &latexPos
	       ) {
  
  for ( unsigned int iLegend=0; iLegend<inLegend.size(); iLegend++ ) {
    if ( !inHist[iLegend] ) continue;
    TH1* hist=0;
    TGraphErrors *graph=0;
    if ( !IsHist(inHist[iLegend] ) ) graph = static_cast<TGraphErrors*>(inHist[iLegend]);
    else hist=static_cast<TH1*>(inHist[iLegend]);
    TAttLine *attLine= hist ? static_cast<TAttLine*>(hist) : static_cast<TAttLine*>(graph);
    TAttMarker *attMarker = hist ? static_cast<TAttMarker*>(hist) : static_cast<TAttMarker*>(graph);
    int color = attLine->GetLineColor();
    int lineStyle = attLine->GetLineStyle();
    int markerStyle = attMarker->GetMarkerStyle();
    bool doFill = inLegend.size() > iLegend && TString( inLegend[iLegend].c_str() ).Contains( "__FILL" );
    if ( hist ) ParseLegend( hist, inLegend[iLegend] );
    else ParseLegend( graph, inLegend[iLegend] );
    if ( doFill ) {
      color = hist ? hist->GetFillColor() : graph->GetFillColor();
      myBoxText( legendCoord[0], legendCoord[1]-0.04*iLegend, color, inLegend[iLegend].c_str() ); 
    }
    else if ( mapOptionsInt.at("drawStyle") ) myMarkerText( legendCoord[0], legendCoord[1]-0.05*iLegend, color, markerStyle, inLegend[iLegend].c_str()  ); 
    else myLineText( legendCoord[0], legendCoord[1]-0.05*iLegend, color, lineStyle, inLegend[iLegend].c_str()  ); 
    if (mapOptionsInt.at("drawStyle")==4) {
  	myLineText( legendCoord[0]-0.005, legendCoord[1]-0.05*iLegend, color, lineStyle, ""  ); 
  	myMarkerText( legendCoord[0], legendCoord[1]-0.05*iLegend, color, markerStyle, inLegend[iLegend].c_str()  );
    }
  }
  if ( DEBUG )  cout << "legend drawn" << endl;

  for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
    if ( latexPos[iLatex].size() != 2 ) continue;
    bool doLabel = TString( inLatex[iLatex] ).Contains("__ATLAS");
    ParseLegend( inLatex[iLatex] );
    if ( doLabel ) ATLASLabel( latexPos[iLatex][0], latexPos[iLatex][1], inLatex[iLatex].c_str(),1 , 0.04 );
    else myText( latexPos[iLatex][0], latexPos[iLatex][1], 1, inLatex[iLatex].c_str() );
  }
  if ( DEBUG ) cout << "latex drawn" << endl;

}
//==============================================
void ChrisLib::DrawPlot( vector< TObject* > &inHist,  
	      string outName, 
	      vector<string> inOptions
	       ) {

  cout << "DrawPlotObj" << endl;
  //================ SOME CHECKS
  if ( DEBUG ) cout << "DrawPlot" << endl;

  map<string, int >  mapOptionsInt;
  map<string, double > mapOptionsDouble;
  vector<string> inLegend, inLatex; 
  vector< vector< double > > latexPos;
  vector< double > legendCoord, rangeUserX, rangeUserY;
  map<string, string> mapOptionsString;
  ReadOptions( inHist.size(), inOptions, mapOptionsDouble, mapOptionsInt, mapOptionsString,
	       inLegend, inLatex, latexPos, legendCoord, rangeUserX, rangeUserY );


  SetAtlasStyle();

  if ( DEBUG ) cout << "Options read" << endl;
  vector< TH1* > ratio;

  //================ PAD DEFINITION
  TCanvas canvas;
  canvas.SetGrid( mapOptionsInt["grid"]%2, mapOptionsInt["grid"]/2 );
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

  TLine line( 0, 0.005, 100, 0.005);
  line.SetLineColor( kBlack );
  line.SetLineStyle( 3 );
  if ( DEBUG ) cout << "defined pads" << endl;

  //============ LOOP OTHER INPUT HIST
  //Find the extremum of the histograms to choose rangeUser if not given
  double minVal=0, maxVal=0, minX=0, maxX=0;
  vector<THStack*> stack;
  int refHist= -1;
  //  unsigned int totEventStack=0;

  TAxis *refXAxis=0, *refYAxis=0;

  if ( mapOptionsDouble["clean"] !=-99 ) {
    vector<TH1*> hists;
    for ( auto it=inHist.begin(); it!=inHist.end(); ++it ) hists.push_back( static_cast<TH1*>(*it));
    CleanHist( hists, mapOptionsDouble["clean"] );
    for ( unsigned i=0; i<inHist.size(); ++i ) inHist[i] = hists[i];
  }
  // if ( DEBUG ) cout << "Cleaned" << endl;
  //  bool isNegativeValue = false;
  for ( unsigned int iHist = 0; iHist < inHist.size(); iHist++ ) {
    if ( !inHist[iHist] ) continue;
    TH1* hist=0;
    TGraphErrors *graph=0;
    if ( !IsHist(inHist[iHist] ) ) graph = static_cast<TGraphErrors*>(inHist[iHist]);
    else hist=static_cast<TH1*>(inHist[iHist]);

    if ( refHist == -1 ) {
      refHist = iHist;
      refXAxis = hist ? hist->GetXaxis() : graph->GetXaxis();
      refYAxis = hist ? hist->GetYaxis() : graph->GetYaxis();
    }

    SetProperties( inHist[iHist], mapOptionsInt, mapOptionsString, iHist-refHist );

    if ( hist && mapOptionsInt["doChi2"] && inLegend.size() && iHist ){
      TH1* refObj=0;
      switch ( mapOptionsInt["drawStyle"] ) {
      case 1 : 
	if ( !IsHist(inHist[iHist-1]) ) throw runtime_error( "ChrisLib::DrawPlot : Chi2 on different types" );
	refObj  = static_cast<TH1*>( inHist[iHist-1] );
	if ( iHist % 2 ) inLegend[iHist] += " : chi2=" + TString::Format( "%2.2f", ComputeChi2( hist, static_cast<TH1*>(inHist[iHist-1]) )/hist->GetNbinsX() );
	break;
      default :

	if ( !IsHist( inHist[refHist] ) ) throw runtime_error( "ChrisLib::DrawPlot : Chi2 on different types" );
	refObj  = static_cast<TH1*>( inHist[refHist] );
	inLegend[iHist] += " : chi2=" + TString::Format( "%2.2f", ComputeChi2( hist, refObj)/refObj->GetNbinsX() );

      }
    }
    

    if ( hist && mapOptionsDouble["normalize"] && hist->Integral() && !mapOptionsInt["stack"] )  {
      hist->Sumw2();
      hist->Scale( mapOptionsDouble["normalize"]/hist->Integral() );
    }


    GetMaxValue( inHist[iHist], minVal, maxVal, minX, maxX, 0, static_cast<int>(iHist)==refHist );

  }    


  if ( DEBUG ) cout << "setting range" << endl;
  while ( rangeUserY.size() < 2 ) rangeUserY.push_back( pow(-1, rangeUserY.size()+1)*0.99 );
  if ( rangeUserY.front() == -0.99 ) rangeUserY.front() = minVal - ( maxVal - minVal ) *0.05;
  if ( rangeUserY.back() == 0.99 ) rangeUserY.back() = maxVal + ( maxVal - minVal ) *0.05;
  rangeUserY.back() += (rangeUserY.back() - rangeUserY.front()) * mapOptionsDouble["extendUp"];

  if ( rangeUserX.size() == 2 ) refXAxis->SetRangeUser( rangeUserX[0], rangeUserX[1] );
  else rangeUserX = { minX, maxX };

  TH1F* dumHist = 0;
  if ( !strcmp( refXAxis->GetBinLabel(1), "" ) ) {
    if ( mapOptionsInt["doRatio"] ) dumHist = padUp.DrawFrame( rangeUserX.front(), rangeUserY.front(), rangeUserX.back(), rangeUserY.back() );
    else dumHist = canvas.DrawFrame( rangeUserX.front(), rangeUserY.front(), rangeUserX.back(), rangeUserY.back() );
    dumHist->GetXaxis()->SetTitle( refXAxis->GetTitle() );
    dumHist->GetYaxis()->SetTitle( refYAxis->GetTitle() );

    if (mapOptionsInt["doRatio"]) {
      dumHist->GetYaxis()->SetTitleOffset( 0.6 );
      dumHist->GetYaxis()->SetTitleSize( 0.06 );
    }
  }
  else refYAxis->SetRangeUser( rangeUserY.front(), rangeUserY.back() );


  //Plotting histograms
  for ( unsigned int iHist = refHist; iHist < inHist.size(); ++iHist ) {
    if ( !inHist[iHist] ) continue;
    TH1* hist=0;
    TGraphErrors *graph=0;
    if ( !IsHist(inHist[iHist] ) ) graph = static_cast<TGraphErrors*>(inHist[iHist]);
    else hist=static_cast<TH1*>(inHist[iHist]);

    string drawOpt = strcmp( refXAxis->GetBinLabel(1), "" ) && static_cast<int>(iHist)==refHist ?  "" :"SAME,";
    switch ( mapOptionsInt["drawStyle"] ){
    case 2 : drawOpt += "HIST"; break;
    case 3 : drawOpt += "HISTL"; break;
    // case 4 : 
    //   inHist[0]->SetMarkerStyle(8);
    //   inHist[1]->SetMarkerStyle(25);
    //   inHist[iHist]->SetMarkerSize(1.3);
    //   break;
    default : drawOpt += "E"; 
    }

    if ( inLegend.size() > iHist && TString( inLegend[iHist].c_str() ).Contains( "__NOPOINT" ) ) {
      cout << "transparent" << endl;
      if ( hist ) {
	hist->SetLineColorAlpha( 0, 0 );
	hist->SetMarkerColorAlpha( 0, 0 );
      }
      else {
	graph->SetLineColorAlpha( 0, 0 );
	graph->SetMarkerColorAlpha( 0, 0 );
      }
    }

    if ( inLegend.size() > iHist && TString( inLegend[iHist].c_str() ).Contains( "__FILL" ) ) {
      drawOpt += "2";
      hist ? hist->SetFillColor( fillColors[iHist] ) : graph->SetFillColor( fillColors[iHist] );
    }
    
    if( !iHist && mapOptionsDouble["line"] != -99 ) {
      double rangeMin = rangeUserX.size()== 2 ? rangeUserX[0] : minX;
      double rangeMax = rangeUserX.size()== 2 ? rangeUserX[1] : maxX;
      line.DrawLine( rangeMin , mapOptionsDouble["line"], rangeMax, mapOptionsDouble["line"]);
    }

    inHist[iHist]->Draw( drawOpt.c_str() );
    //========== ADD HISTOGRAM TO LEGEND
  }//end iHist

  if ( DEBUG ) cout << "drawn" << endl;
  
  if ( mapOptionsInt["logy"] ) {
    int topVal = ceil( log10( maxVal ) );
    int lowVal = minVal==0 ? topVal-5 : floor( log10( minVal ) );
    if ( rangeUserY[0] < 0 ) rangeUserY[0]=pow( 10, lowVal );
    rangeUserY[1] = pow( 10, topVal + ( topVal - lowVal ) * (0.05 + mapOptionsDouble["extendUp"] ) );
    refYAxis->SetRangeUser( rangeUserY[0], rangeUserY[1] );    
    if ( mapOptionsInt["doRatio"] ) {
      padUp.SetLogy(1);
    }
    else {
      canvas.SetLogy(1);
    }
    if ( DEBUG ) cout << "logy done" << endl;
  }

  // =========== PRINT LEGENDS AND LATEX
  canvas.cd();
  DrawText( inHist, inLegend, inLatex, mapOptionsInt, legendCoord, latexPos);
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
  	ratio.back() = static_cast<TH1*>(inHist[iHist]->Clone());
  	ratio.back()->Add( static_cast<TH1*>(inHist[iHist-1]), -1 );
  	if ( mapOptionsInt["doRatio"] == 1 ) ratio.back()->Divide( static_cast<TH1*>(inHist[iHist-1]) );
  	yTitle = ( mapOptionsInt["doRatio"]==1 ) ? "#frac{h_{2n+1}-h_{2n}}{h_{2n}}" : "h_{2n+1}-h_{2n}";
  	break;
      default : 
  	ratio.push_back( 0 );
  	ratio.back() = static_cast<TH1*>(inHist[iHist]->Clone());
  	ratio.back()->Add( static_cast<TH1*>(inHist[refHist]), -1 );
  	if ( mapOptionsInt["doRatio"] == 1 ) ratio.back()->Divide( static_cast<TH1*>(inHist[refHist]) );
  	yTitle = ( mapOptionsInt["doRatio"]==1 ) ? "#frac{h_{n}-h_{0}}{h_{0}}" : "h_{n}-h_{0}";
      }
      if ( DEBUG ) cout << "ratio created" << endl;
      //Set graphics properties of first hitogram
      if ( !setTitle ) {
  	ratio.front()->GetXaxis()->SetTitle( refXAxis->GetTitle() );
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
      if ( DEBUG ) cout << "plot ratio" << endl;
      for ( unsigned int iHist = 0; iHist < ratio.size(); iHist++ ) {
  	ratio[iHist]->Draw( ( iHist ) ? "e,same" : "e" );
      }
      //Create a line at 0 to visualize deviations
      line.DrawLine( mapOptionsInt["centerZoom"] ? minX : ratio.front()->GetXaxis()->GetXmin(), 0, mapOptionsInt["centerZoom"] ? maxX :ratio.front()->GetXaxis()->GetXmax(), 0);
    }
  }//end doRatio

  if ( DEBUG ) cout << "saving" << endl;
  string canOutName = outName + "." + mapOptionsString["extension"];
  canvas.SaveAs( canOutName.c_str() );

}


