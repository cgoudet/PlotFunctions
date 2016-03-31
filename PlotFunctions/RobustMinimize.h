#ifndef ROBUSTMINIMIZE_H
#define ROBUSTMINIMIZE_H
#include "RooMinimizer.h"


int robustMinimize(RooAbsReal &nll, RooMinimizer &minim, int const verbosity=0);

#endif
