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
  void SplitTree( const InputCompare &inputCompare );
  void PlotMatrix( const InputCompare &inputCompare, std::vector<std::vector<TObject*>> &vectObj );

  //=======================
  enum class OutMode { none, hist, histMultiBranch, histEvent, profile, graphErrors };

  //=======================
  OutMode GetOutMode( const InputCompare &inputCompare );
  std::string WriteOutMode ( const OutMode outMode );

  void PrintOutputCompareEvents( const boost::multi_array<double,2> &varValues,
                                 const boost::multi_array<long long,2> &IDValues,
                                 const std::vector<std::string> &eventID,
                                 const std::vector<std::vector<TObject*>> &vectHist,
                                 const std::string &outName );

  /**\brief Fills the bin of an histogram with a function of its content and totweight.

     # Code
     - 0 : Weighted sum (like usual histogram)

     - 1 : Weighted Oplus f(old, add, weight)=\sqrt{old^2+weight add^2}

     - 2 : Weighted quadartic sum
   */
  void FillFunctionHisto( TH1* filledHist, const unsigned int bin, const double value, const double weight =1, const unsigned code = 0);

  /**\brief  Fill the variable identifying all events of interest and return the index of the event with the same properties as the current
   */
  int FillCompareEvent( const InputCompare &inputCompare, boost::multi_array<long long,2> &IDValues, const MapBranches &mapBranch, const int iPlot, const int iEvent );
  TObject* InitHist( const InputCompare &inputCompare, unsigned iPlot, unsigned iHist );

  /**\brief Test the mandatory variables for each option file reading options (0<inputType<6).

     The code consists of a bitset for which each bin consists in a mandatory variable :
     - 0 : A binning option

     - 1 : eventID

     - 2 : Requires varYName

     - 3 : varName and varYName must have same size

     - 4 : requires varName

     - 5 : requires non null nEvent

     Each possible OutMode switches on its own requirements (see ChrisLib::InputCompare )

     Some other requirements are coded into the function which use them.
   */
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
  bool IsTH1( OutMode outMode );
  void IncreaseHist( TH1** hist, const std::string &label );
  //==================================

}

#endif
