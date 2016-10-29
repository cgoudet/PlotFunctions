#ifndef ROBUSTMINIMIZE_H
#define ROBUSTMINIMIZE_H

#include "RooMinimizer.h"
#include "RooAbsPdf.h"
#include "RooAbsData.h"


int robustMinimize(RooAbsReal &nll, RooMinimizer &minim, int const verbosity=0);
void FitData( RooAbsData * data, RooAbsPdf* pdf, int const verbosity=0 );

#endif
