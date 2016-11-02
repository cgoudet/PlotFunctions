#ifndef RATIOPLOTS_H
#define RATIOPLOTS_H

#include "boost/multi_array.hpp"
using boost::multi_array;
#include "TH2D.h"
#include <string>
#include <vector>
#include "TH1D.h"
using std::vector;
using std::string;
#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooAbsData.h"
#include "RooCategory.h"
#include "TGraphErrors.h"

namespace ChrisLib { 
  /**\brief Plot histograms and save them in png format
     \param inHist vector of histograms to be superimposed
     \param outName name of the output png plot without extension
     \param inLegend vector of legend texts to print for each histogram
     \param switch ratio mode
     \param normalize normalize all input histograms
     \param doChi2 switch chi2 mode
     \param centerZoom center the X axis on non empty bins
     \param rangeUser vector of low and high limit of Y axis range
     \param legendCoord vector containing coordinate of the legend
     \param inLate vector of latex text to be drawn
     \param latexPos Coordinates of latex texts to be drawn
     \drawStyle swith drawStyle mode

     There is some correlation between the variables. Effects can be added.

     doChi2 mode :
     Compute the chi2 between two histograms (depending on drawStyole) and add it to the legend
     if drawStyle=1, should make chi2 between paired histograms
     Not working properly yet

     ratio mode :
     Create a low pad to hold ratio or difference plot. 
     default mode : compare the histogram i with the histogram 0
     1 : (h1-h0)/h0
     2 : (h1-h0)

     drawStyle mode :
     default mode : give one different colour per histogram
     1 : pair histogram by 2 and give same colour with different marker. IOn case of ratio, make ratio of second pair compared to first

     latexPos :


  */
  int DrawPlot( vector< TH1* > &inHist,  
		string outName, 
		vector<string> inOptions=vector<string>()
		);

  int DrawPlot( vector< TGraphErrors* > inGraph,  
		string outName, 
		vector<string> inOptions=vector<string>()
		);

  int DrawPlot( RooRealVar *frameVar,
		vector<TObject*> inObj,
		string outName,
		vector<string> inOptions=vector<string>()
		);

  vector<string> PlotPerCategory( //RooRealVar *varFrame, 
				 vector<TObject*> vectObj, RooCategory *cat, string prefix = "", vector<string> options = vector<string>() );

}

#endif
