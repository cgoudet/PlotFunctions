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
