#ifndef PLOTFUNCTIONS_H
#define PLOTFUNCTIONS_H

#include "PlotFunctions/InputCompare.h"
#include "boost/multi_array.hpp"

namespace ChrisLib {

  void PlotHist( const ChrisLib::InputCompare &inputCompare );
  void PlotTree( const InputCompare &inputCompare );


  //=======================
  void PrintOutputCompareEvents( const boost::multi_array<double,2> &varValues, 
				 const boost::multi_array<long long,2> &IDValues, 
				 const std::vector<string> &eventID, 
				 const std::vector<std::vector<TH1*>> &vectHist, 
				 const std::string &outName );

  void DrawVect( std::vector<std::vector<TH1*>> &vectHist, const InputCompare &inputCompare );
}

#endif
