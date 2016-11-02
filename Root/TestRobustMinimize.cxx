#include <boost/test/unit_test.hpp>

#include "PlotFunctions/RobustMinimize.h"

#include "TH1D.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooArgSet.h"
#include "RooGaussian.h"

// The name of the suite must be a different name to your class
BOOST_AUTO_TEST_SUITE( RobustMinimizeSuite )

BOOST_AUTO_TEST_CASE( FitDataTest ) {

  // TH1D hist("hist", "hist", 100,-5, 5 );
  // hist.FillRandom( "gaus", 1e6 );

  // RooRealVar mass( "mass", "mass", 0, -5, 5 );
  // RooRealVar mean( "mean", "mean", 0, -5, 5 );
  // RooRealVar sigma( "sigma", "sigma", 1, 0, 3 );

  // RooDataHist dataHist( "datahist", "datahist", RooArgSet(mass), &hist );
  // RooGaussian pdf( "gaus", "gaus", mass, mean, sigma );
  // FitData( &dataHist, &pdf, -1 );

  // BOOST_CHECK( fabs(mean.getVal()) < 0.001 );
  // BOOST_CHECK( fabs(sigma.getVal()-1) < 0.001 );
}

BOOST_AUTO_TEST_SUITE_END()


