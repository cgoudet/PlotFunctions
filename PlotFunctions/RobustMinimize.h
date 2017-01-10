#ifndef ROBUSTMINIMIZE_H
#define ROBUSTMINIMIZE_H

#include "RooMinimizer.h"
#include "RooAbsPdf.h"
#include "RooAbsData.h"
#include "RooDataSet.h"
#include "RooDataHist.h"

namespace ChrisLib {

  int robustMinimize(RooAbsReal &nll, RooMinimizer &minim, int const verbosity=-1);
  void FitData( RooAbsData* data, RooAbsPdf* pdf, int const verbosity=0 );
}

#endif
