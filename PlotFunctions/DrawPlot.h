#ifndef RATIOPLOTS_H
#define RATIOPLOTS_H

#include "TH2D.h"
#include "TH1D.h"
#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooAbsData.h"
#include "RooCategory.h"
#include "TGraphErrors.h"
#include "TObject.h"

#include "boost/multi_array.hpp"
using boost::multi_array;

#include <string>
#include <vector>
#include <map>

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
  int DrawPlot( std::vector< TH1* > &inHist,  
		std::string outName, 
		std::vector<std::string> inOptions=std::vector<std::string>()
		);

  int DrawPlot( std::vector< TGraphErrors* > &inGraph,
		std::string outName, 
		std::vector<std::string> inOptions=std::vector<std::string>()
		);

  int DrawPlot( RooRealVar *frameVar,
		std::vector<TObject*> inObj,
		std::string outName,
		std::vector<std::string> inOptions=std::vector<std::string>()
		);

  void DrawPlot( std::vector< TObject* > &inHist,  
		std::string outName, 
		std::vector<std::string> inOptions=std::vector<std::string>()
		);

  std::vector<std::string> PlotPerCategory( //RooRealVar *varFrame, 
				 std::vector<TObject*> vectObj, RooCategory *cat, std::string prefix = "", std::vector<std::string> options = std::vector<std::string>() );

  //============================
  void ReadOptions( unsigned nHist, 
		    const std::vector<std::string> &inOptions,
		    std::map<std::string,double> &mapDouble,
		    std::map<std::string,int> &mapInt,
		    std::map<std::string,std::string> &mapString,
		    std::vector<std::string> &inLegend,
		    std::vector<std::string> &inLatex,
		    std::vector<std::vector<double>> &latexPos,
		    std::vector<double> &legendCoord,
		    std::vector<double> &rangeUserX,
		    std::vector<double> &rangeUserY
		    );

		    
}

#endif
