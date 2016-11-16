#ifndef PLOTFUNCTIONS_H
#define PLOTFUNCTIONS_H

#include "PlotFunctions/InputCompare.h"
#include "PlotFunctions/MapBranches.h"
#include "PlotFunctions/DrawPlot.h"

#include "boost/multi_array.hpp"

#include "TH1.h"
#include "TObject.h"
#include "TGraphErrors.h"

#include <stdlib.h>

namespace ChrisLib {

  void PlotHist( const InputCompare &inputCompare, std::vector<std::vector<TObject*>> &vectHist );
  void PlotTree( const InputCompare &inputCompare, std::vector<std::vector<TH1*>> &vectHist, std::vector<std::vector<TGraphErrors*>> &vectGraph );
  void PlotTree( const InputCompare &inputCompare, std::vector<std::vector<TObject*>> &vectHist );
  void PlotTextFile( const InputCompare &inputCompare, std::vector<std::vector<TObject*>> &vectHist );
  void SplitTree( const InputCompare &inputCompare );
  void PlotMatrix( const InputCompare &inputCompare, std::vector<std::vector<TObject*>> &vectObj );
  
  //=======================
  enum class OutMode { none, hist, histEvent, profile, graphErrors };

  //=======================
  OutMode GetOutMode( const InputCompare &inputCompare );
  std::string WriteOutMode ( const OutMode outMode );
  
  void PrintOutputCompareEvents( const boost::multi_array<double,2> &varValues, 
				 const boost::multi_array<long long,2> &IDValues, 
				 const std::vector<std::string> &eventID, 
				 const std::vector<std::vector<TObject*>> &vectHist, 
				 const std::string &outName );

  int FillCompareEvent( const InputCompare &inputCompare, boost::multi_array<long long,2> &IDValues, const MapBranches &mapBranch, const int iPlot, const int iEvent );
  TObject* InitHist( const InputCompare &inputCompare, unsigned iPlot, unsigned iHist );
  void TestInputs( const InputCompare &inputCompare );
  void DrawVect( std::vector<std::vector<TObject*>> &vectGraph, const InputCompare &inputCompare );
  void SetTGraphsTitle( const InputCompare &inputCompare, const unsigned iPlot, std::vector<std::vector<TGraphErrors*>> &vectGraph );

  void FillObject( const InputCompare &inputCompare, 
		   const MapBranches &mapBranch, 
		   std::vector<std::vector<TObject*>> &vectObject,
		   boost::multi_array<long long,2> &IDValues,
		   boost::multi_array<double,2> &varValues,
		   const unsigned iPlot,
		   const unsigned iEntry
		   );

  //==================================
  
}

#endif
