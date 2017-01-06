#ifndef RATIOPLOTS_H
#define RATIOPLOTS_H

#include "PlotFunctions/DrawOptions.h"

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

  /** \brief Plot a set of histograms on the same pad.
    
     Common algorithm to perform simple plotting of histograms, TProfile and TGraphErrors. 
     There should be no mixing of TGraph's with TH1's in the same vector.
     
     The available options accepted by the algorithm are documented in ChrisLib::DrawOptions. 
     A wrong option will create a printed warning and be ignored for the remaining of the algorithm.
     
     
     To add an option to a DrawOption object, see ChrisLib::DrawOption::AddOption.
  */
  //  void DrawPlot( std::vector< TObject* > &inHist, ChrisLib::DrawOptions &drawOpt );
  
  /** \brief Plot a set of histograms on the same pad.
      
      This function allows for the passing of options through a vector of string.
      The vector of options is converted into a ChrisLib::DrawOptions and ChrisLib::DrawPlot is called.
      
      Options must be strings with the following nomenclature
      ```
      optionName=optionValue
      ```
  */
  /* void DrawPlot( std::vector< TObject* > &inHist,   */
  /* 		 std::string outName,  */
  /* 		 std::vector<std::string> inOptions=std::vector<std::string>() */
  /* 		 ); */
  
  /* int DrawPlot( std::vector< TH1* > &inHist, */
  /* 		std::string outName, */
  /* 		std::vector<std::string> inOptions=std::vector<std::string>() */
  /* 		); */

  /* int DrawPlot( std::vector< TGraphErrors* > &inGraph, */
  /* 		std::string outName, */
  /* 		std::vector<std::string> inOptions=std::vector<std::string>() */
  /* 		); */
  
  int DrawPlot( RooRealVar *frameVar,
		std::vector<TObject*> inObj,
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
