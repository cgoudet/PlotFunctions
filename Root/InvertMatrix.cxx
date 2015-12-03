#include <iostream>
#include "PlotFunctions/InvertMatrix.h"
#include "TMath.h"
#include "boost/multi_array.hpp"
#include "RooRealVar.h"
#include <vector>
#include "RooArgList.h"
#include "RooCategory.h"
#include "RooSimultaneous.h"
#include "RooGaussian.h"
#include "RooFormulaVar.h"
#include "RooArgSet.h"
#include <map>
#include "RooDataSet.h"
#include <string>
#include "RooConstVar.h"
#include "PlotFunctions/SideFunctions.h"
#include "Math/MinimizerOptions.h"

using std::copysign;
using std::string;
using std::map;
using std::vector;
using TMath::Power;
using std::cout;
using std::endl;
using boost::multi_array;
using boost::extents;

using namespace RooFit;

void Style_Christophe();


void InvertMatrix( TMatrixD &combinMatrix, TMatrixD &combinErrMatrix, TMatrixT<double>& outMatrix, TMatrixT<double> &outErrMatrix, unsigned int inversionProcedure ) {
  /*
    InputType :
    0 : alpha
    1 : sigma
  
    Inversion Procedure 
    0 : inversion matrix
    1 : fit
  */

  // cout << "combinMatrix" << endl;
  // combinMatrix.Print();
  // cout << "combinErrMatrix" << endl;
  // combinErrMatrix.Print();
  cout << "inversion method : " << inversionProcedure << endl;

  unsigned int nBins = (unsigned int ) combinMatrix.GetNrows();

  switch ( inversionProcedure/10 ) {

  case 0 : { // Exact linear matrix inversion
    cout << "linear matrix inversion" << endl;
    //Create the B matrix for alpha
    TMatrixD *bMatrix = new TMatrixD( nBins, 1);
    TMatrixD *UMatrix = new TMatrixD( nBins, nBins );  

    //Inversion code stolen directly from Template::InvertMatrix the 22nd July 2015
    //No modification except for variables names
    for ( unsigned int line = 0; line < nBins; line++ ) {
      for ( unsigned int col = 0; col < nBins; col++ ) {
	if ( combinErrMatrix( line, col ) <= 0 ) {
	  cout << "combinErrMatrix has negative values." << endl;
	  return ;
	}
 
	switch ( inversionProcedure%10 ) {
	case 0 : { // default linear case
	  (*bMatrix)(line, 0 ) += combinMatrix(line, col ) /  combinErrMatrix( line, col) / combinErrMatrix( line, col) * ( 1 + Delta( col, line ) ); 
	  (*UMatrix)( line, col ) += 1. / 2 / combinErrMatrix( line, col ) / combinErrMatrix( line, col ) * ( 1 +Delta( line, col ) );
	  (*UMatrix)( line, line ) += 1. / 2 / combinErrMatrix( line, col ) / combinErrMatrix( line, col ) * ( 1 +  Delta( line, col ) );
	  break;	
	}
	case 1 : { // linear inversio nof squares
	  if ( combinMatrix(line, col) < 0 ) {
	    cout << "combinMatrix in Sigma inputType has negative value." << endl;
	    return ;
	  }

	  (*bMatrix)( line, 0)    += ( 1. + Delta( col, line ) ) * combinMatrix( line, col ) * combinMatrix( line, col )  / Power( 2 * combinErrMatrix( line, col) * combinMatrix( line, col) + Power( combinErrMatrix( line, col), 2 ), 2 ) ;
	  (*UMatrix)( line, col)  += (1+Delta( col, line )) / 2. / Power( 2 * combinErrMatrix( line, col) * combinMatrix( line, col) + Power( combinErrMatrix( line, col), 2 ), 2 );
	  (*UMatrix)( line, line) += ( 1. + Delta( col, line ) ) / 2. / Power( 2 * combinErrMatrix( line, col) * combinMatrix( line, col) + Power( combinErrMatrix( line, col), 2 ), 2 );
	  break;
	}
	default :
	  cout << "Not supporting inversion procedure : " << inversionProcedure << endl;
	  return;
	}

      }}
  
    UMatrix->Invert();
    //    UMatrix=2*UMatrix;
    outMatrix = TMatrixD( nBins, 1);
    outErrMatrix = TMatrixD( nBins, 1);
    outMatrix = (*UMatrix) * (*bMatrix);
    for ( unsigned int iBin=0; iBin < nBins; iBin++ ) {
      switch ( inversionProcedure%10 ) {
      case 0 :
	outErrMatrix(iBin, 0) = sqrt( 2* (*UMatrix)(iBin, iBin) );
	break;
      case 1 :
	outMatrix(iBin,0) = SignSquare( outMatrix(iBin, 0) );
	outErrMatrix(iBin, 0) = ErrC( outMatrix(iBin,0), sqrt(2*(*UMatrix)(iBin, iBin)) );
	break;
      }//end switch inputType
    }
    break;
  }//end case 0 inversionProcedure/10

  case 1 : { //inversionProcedure/10
    cout << "fitMethod" << endl;
    // method with likeliood fit
    //Defining observables
    RooRealVar *alpha = new RooRealVar( "alpha", "alpha", -0.1, 0.1 );
    //    RooRealVar *alphaErr = new RooRealVar( "alphaErr", "alphaErr", 0, 1e3 );
    RooArgSet *observables = new RooArgSet( RooArgSet( *alpha ), "observables" );
    map< string, RooDataSet*> datasetMap;
    
    vector<RooRealVar*> alphaBin;
    multi_array<RooFormulaVar*, 2> alphaConfig;
    multi_array<RooConstVar*, 2> alphaErrConfig;
    RooCategory* channellist = new RooCategory("channellist","channellist");
    RooSimultaneous *combinedPdf = new RooSimultaneous("CombinedPdf","",*channellist); 
    vector< RooGaussian*> configPdf;
    cout << "header" << endl;
    for (unsigned int iLine = 0; iLine < (unsigned int) combinMatrix.GetNrows(); iLine ++) {
      alphaBin.push_back(0);
      alphaConfig.resize( extents[iLine+1][iLine+1] );
      alphaErrConfig.resize( extents[iLine+1][iLine+1] );

      for ( unsigned iCol = 0; iCol<=iLine ; iCol++ ) {

	//Create the roorealvar for bin iLine
	TString configName = TString::Format( "Config_%d_%d", iLine, iCol );      
	channellist->defineType( configName );
	TString alphaName;
	switch ( inversionProcedure%10 ) {
	case 0 : //alpha
	  if ( !alphaBin.back() ) {
	    alphaName = TString::Format( "alpha_%d", iLine );
	    alphaBin.back() = new RooRealVar( alphaName, alphaName, 0, -0.1, 0.1 );
	  }
	  if ( iLine == iCol ) alphaBin[iLine]->setVal( combinMatrix[iLine][iLine] );	
	  alphaName = TString::Format( "alphaConf_%d_%d", iLine, iCol );
	  alphaConfig[iLine][iCol] = new RooFormulaVar( alphaName, alphaName, "(@0+@1)/2.", RooArgList( *alphaBin[iLine], *alphaBin[iCol] ) );
	  alphaName = TString::Format( "alphaErrConf_%d_%d", iLine, iCol );
	  alphaErrConfig[iLine][iCol] = new RooConstVar( alphaName, alphaName, combinErrMatrix( iLine, iCol ) );

	  alpha->setVal( combinMatrix(iLine, iCol) );

	  break;

	case 1 : {//constant term
	  if ( !alphaBin.back() ) {
	    alphaName = TString::Format( "C_%d", iLine );
	    alphaBin.back() = new RooRealVar( alphaName, alphaName, combinMatrix( iLine, iCol ), 0, 0.1 );
	  }
	  if ( iLine == iCol ) alphaBin[iLine]->setVal( combinMatrix[iLine][iLine] );	
	  alphaName = TString::Format( "CConf_%d_%d", iLine, iCol );
	  alphaConfig[iLine][iCol] = new RooFormulaVar( alphaName, alphaName, "TMath::Sqrt((@0*@0+@1*@1)/2.)", RooArgList( *alphaBin[iLine], *alphaBin[iCol] ) );
	  alphaName = TString::Format( "CErrConf_%d_%d", iLine, iCol );
	  alphaErrConfig[iLine][iCol] = new RooConstVar( alphaName, alphaName, combinErrMatrix( iLine, iCol ) );
	  alpha->setVal( combinMatrix(iLine, iCol) );
	  break;}
	case 2 : {
	  if ( !alphaBin.back() ) {
	    alphaName = TString::Format( "C2_%d", iLine );
	    alphaBin.back() = new RooRealVar( alphaName, alphaName, combinMatrix( iLine, iCol ), 0, 0.1 );
	  }
	  if ( iLine == iCol ) alphaBin[iLine]->setVal( combinMatrix[iLine][iLine]*combinMatrix[iLine][iLine] );	
	  alphaName = TString::Format( "C2Conf_%d_%d", iLine, iCol );
	  alphaConfig[iLine][iCol] = new RooFormulaVar( alphaName, alphaName, "(@0+@1)/2.", RooArgList( *alphaBin[iLine], *alphaBin[iCol] ) );
	  alphaName = TString::Format( "C2ErrConf_%d_%d", iLine, iCol );
	  alphaErrConfig[iLine][iCol] = new RooConstVar( alphaName, alphaName, 2*combinErrMatrix( iLine, iCol )*combinMatrix( iLine, iCol )+combinErrMatrix( iLine, iCol )*combinErrMatrix( iLine, iCol ) );
	  alpha->setVal( combinMatrix(iLine, iCol)*combinMatrix(iLine, iCol) );
	  break;}
	default :
	  cout << "Not supporting inversionProcedure : " << inversionProcedure << endl;
	  return;
	}// end switch inputType

	//Create the model for the configuration
	configPdf.push_back( 0 );
	configPdf.back() = new RooGaussian( "Gauss_" + configName, "Gauss_" + configName , *alpha, *alphaConfig[iLine][iCol], *alphaErrConfig[iLine][iCol] ); 	
	combinedPdf->addPdf( *configPdf.back(), configName );
	//Create the dataset

	RooDataSet *configData = new RooDataSet( "Data_" + configName, "Data_" + configName, *observables );
	configData->add( *observables );
	//	configData->Print("v");
	datasetMap[string(configName)] = configData;
      }//end iCol
    }// end iLine

    
    RooDataSet* obsData = new RooDataSet("obsData","combined data ",*observables, Index(*channellist), Import(datasetMap)); 
    combinedPdf->Print();
    combinedPdf->fitTo( *obsData );

    outMatrix = TMatrixD( nBins, 1 );
    outErrMatrix = TMatrixD( nBins, 1 );
    //FillThe result Matrix
    for (unsigned int iLine = 0; iLine < nBins; iLine ++) {
      switch ( inversionProcedure%10 ) {
      case 0 : //alpha
	outMatrix(iLine, 0) = alphaBin[iLine]->getVal();
	outErrMatrix(iLine, 0) = alphaBin[iLine]->getError();
	break;
      case 1 ://sigma
	outMatrix(iLine, 0) = alphaBin[iLine]->getVal();
	outErrMatrix(iLine, 0) = alphaBin[iLine]->getError();
	break;
      case 2 :
	outMatrix(iLine, 0) = SignSquare( alphaBin[iLine]->getVal() );
	outErrMatrix(iLine, 0) = ErrC( outMatrix(iLine, 0), alphaBin[iLine]->getError() );
	break;
      }//end switch inversionprocedure%10
    }//end for

    delete alpha;
    delete observables;
    for ( auto ent1 : datasetMap) delete ent1.second;
    for ( auto bin : alphaBin ) delete bin;
    delete channellist;
    for(auto pdf : configPdf) delete pdf;
    delete combinedPdf;
    for ( unsigned int iLine = 0; iLine < alphaConfig.size(); iLine++ ) {
      for ( unsigned int iCol = 0; iCol < alphaConfig[iLine].size(); iCol++ ) {
    	delete alphaConfig[iLine][iCol];
	delete alphaErrConfig[iLine][iCol];
      }}
    break;
  }//end case 1 invettionProcedure

    
  default : 
    cout << "input inversionProcedure/10 do not corresponds do any case available : " << inversionProcedure << endl;
  }//end switch inversionProcedure 
  
}


//============================
double SignSquare( double inVal ) {
  return copysign( sqrt( fabs( inVal ) ), inVal );
}

//==========================================
double ErrC( double c, double errC2 ) {
  return sqrt( c*c + errC2 ) - fabs(c);

}
