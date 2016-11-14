#ifndef PLOTFUNCTIONS_H
#define PLOTFUNCTIONS_H

#include "PlotFunctions/InputCompare.h"
#include "PlotFunctions/MapBranches.h"
#include "boost/multi_array.hpp"

#include "TH1.h"
#include "TObject.h"
#include "TGraphErrors.h"

namespace ChrisLib {

  void PlotHist( const InputCompare &inputCompare, std::vector<std::vector<TH1*>> &vectHist );
  void PlotTree( const InputCompare &inputCompare, std::vector<std::vector<TH1*>> &vectHist, std::vector<std::vector<TGraphErrors*>> &vectGraph );
  void PlotTextFile( const InputCompare &inputCompare, std::vector<std::vector<TH1*>> &vectHist );
  void SplitTree( const InputCompare &inputCompare );

  //=======================
  void PrintOutputCompareEvents( const boost::multi_array<double,2> &varValues, 
				 const boost::multi_array<long long,2> &IDValues, 
				 const std::vector<std::string> &eventID, 
				 const std::vector<std::vector<TH1*>> &vectHist, 
				 const std::string &outName );

  void DrawVect( std::vector<std::vector<TH1*>> &vectHist, const InputCompare &inputCompare );
  int FillCompareEvent( const InputCompare &inputCompare, boost::multi_array<long long,2> &IDValues, const MapBranches &mapBranch, const int iPlot, const int iEvent );
  TObject* InitHist( const InputCompare &inputCompare, int outMode, int iPlot, int iHist );
}

#endif
