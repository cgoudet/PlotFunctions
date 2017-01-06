#include "PlotFunctions/SideFunctions.h"
#include "PlotFunctions/SideFunctionsTpp.h"
#include "PlotFunctions/DrawPlot.h"
#include "PlotFunctions/AtlasStyle.h"
#include "PlotFunctions/AtlasUtils.h"
#include "PlotFunctions/AtlasLabels.h"

#include "TCanvas.h"
#include "TLegend.h"
#include "TLine.h"
#include "TLatex.h"
#include <TROOT.h>
#include "THStack.h"
#include "RooPlot.h"
#include "RooDataSet.h"
#include "RooAbsPdf.h"
#include "RooSimultaneous.h"
#include "TF1.h"
#include "TObject.h"

#include <iterator>
#include <iostream>
#include <map>
#include <list>

using std::invalid_argument;
using std::runtime_error;
using std::map;
using std::cout;
using std::endl;
using std::min;
using std::max;
using std::list;

using namespace ChrisLib;
using namespace RooFit;

#define DEBUG 0
// enum EColor { kWhite =0,   kBlack =1,   kGray=920,
//               kRed   =632, kGreen =416, kBlue=600, kYellow=400, kMagenta=616, kCyan=432,
//               kOrange=800, kSpring=820, kTeal=840, kAzure =860, kViolet =880, kPink=900 };


int colors[] = {1, 
		632, 600, 616, 416, 800, 
		921,
		629, 597, 613, 413, 797, 
		635, 603, 619, 419, 807 };

int fillColors[] = { 3, 5 };

//==================================================
void ChrisLib::ReadOptions( unsigned nHist,
			    const vector<string> &inOptions,
			    map<string,double> &mapDouble,
			    map<string,int> &mapInt,
			    map<string,string> &mapString,
			    vector<string> &inLegend,
			    vector<string> &inLatex,
			    vector<vector<double>> &latexPos,
			    vector<double> &legendCoord,
			    vector<double> &rangeUserX,
			    vector<double> &rangeUserY
			    ) {

  if ( DEBUG ) cout << "ChrisLib::ReaOptions" << endl;

  list<string> allowedInt = { "doRatio", "shiftColor", "doChi2", "centerZoom", "drawStyle", "logy", "stack", "offset", "orderX", "grid" };
  for ( auto it=allowedInt.begin(); it!=allowedInt.end(); ++it ) mapInt[*it]=0;

  mapDouble["extendUp"]=0;
  mapDouble["normalize"]=0;
  mapDouble["scale"]=0;
  mapDouble["line"]=-99;
  mapDouble["clean"]=-99;

  mapString["xTitle"]="";
  mapString["yTitle"]="";
  mapString["extension"]="pdf";

  for ( auto iOption : inOptions ) {
    string option = iOption.substr( 0, iOption.find_first_of('=' ) );
    string value = iOption.substr( iOption.find_first_of("=")+1);
    if ( mapInt.find(option) != mapInt.end() ) mapInt[option] = atoi( value.c_str() );
    else if ( mapString.find(option) != mapString.end() ) mapString[option] = value;
    else if ( mapDouble.find(option) != mapDouble.end() ) {
      mapDouble[option] = std::stod( value.c_str() );
    }
    else if ( option == "legend" ) inLegend.push_back( value );
    else if ( option == "latex" ) inLatex.push_back( value );
    else if ( option == "latexOpt" ) {
      latexPos.push_back( vector<double>() );
      ParseVector( value, latexPos.back() );
    }
    else if ( option == "legendPos" ) ParseVector( value, legendCoord );
    else if ( option == "rangeUserX" ) ParseVector( value, rangeUserX );
    else if ( option == "rangeUserY" ) ParseVector( value, rangeUserY );
    else {
      cout << "DrawPlotOption : " << option << " not known" << endl;
    }
  }

  if ( inLegend.size() && inLegend.size()!=nHist ) throw invalid_argument( "ChrisLib::ReadOptions : Legend do not match input" );
  if ( inLatex.size() != latexPos.size() ) throw invalid_argument( "ChrisLib::ReadOptions : Number of latex names and positions do not match" );

  if ( nHist == 1 ) mapInt["drawStyle"] = 0;
  if ( nHist < 2 ) mapInt["doRatio"] = 0;

  list<string> allowedExtension = { "pdf", "root", "png" };
  if ( find(allowedExtension.begin(), allowedExtension.end(), mapString["extension"] ) == allowedExtension.end() ) throw runtime_error( "DrawPlot : Wrong output file extension provided" );

  if ( DEBUG ) cout << "ChrisLib::ReaOptions end" << endl;
}

//===================================

int ChrisLib::DrawPlot( RooRealVar *frameVar,
	      vector<TObject*> inObj,
	      string outName,
	      vector<string> inOptions
	      ) {
  //  cout << "DrawPlot frame" << endl;

  vector<string> inLegend, inLatex; 
  vector< vector< double > > latexPos;
  vector< double > legendCoord, rangeUserX, rangeUserY;
  map<string, int> mapOptionsInt;
  mapOptionsInt["nComparedEvents"]=100;
  map<string, double> mapOptionsDouble;
  map<string, string> mapOptionsString;
  mapOptionsString["xTitle"]="";
  mapOptionsString["yTitle"]="";
  

  for ( auto iOption : inOptions ) {
    string option = iOption.substr( 0, iOption.find_first_of('=' ) );
    string value = iOption.substr( iOption.find_first_of("=")+1);
    if ( mapOptionsInt.find(option) != mapOptionsInt.end() ) mapOptionsInt[option] = atoi( value.c_str() );
    else if ( mapOptionsString.find(option) != mapOptionsString.end() ) mapOptionsString[option] = value;
    else if ( mapOptionsDouble.find(option) != mapOptionsDouble.end() ) mapOptionsDouble[option] =  (double) std::atof( value.c_str() );
    else if ( option == "legend" ) inLegend.push_back( value );
    else if ( option == "latex" ) inLatex.push_back( value );
    else if ( option == "latexOpt" ) {
      latexPos.push_back( vector<double>() );
      ParseVector( value, latexPos.back() );
    }
    else if ( option == "legendPos" ) ParseVector( value, legendCoord );
    else if ( option == "rangeUserX" ) ParseVector( value, rangeUserX );
    else if ( option == "rangeUserY" ) ParseVector( value, rangeUserY );
    else {
      cout << "Option : " << option << " not known" << endl;
    }
  }

  if ( inLegend.size() && inLegend.size()!=inObj.size() ) { cout << "Legend do not match input" << endl;return  1;}
  if ( inLatex.size() != latexPos.size() ) {cout << "Number of latex names and positions do not match" << endl << inLatex.size() << " " << latexPos.size() << endl; return 2; }

  SetAtlasStyle();

  if ( DEBUG ) cout << "Options read" << endl;

  SetAtlasStyle();
  TCanvas *canvas = new TCanvas();
  if ( rangeUserX.size() == 2 ) frameVar->setRange( rangeUserX.front(), rangeUserX.back() );

  RooPlot* frame=frameVar->frame(mapOptionsInt["nComparedEvents"]);
  frame->SetTitle(""); //empty title to prevent printing "A RooPlot of ..."
  frame->SetXTitle(frameVar->GetTitle());

  vector<map<string,int>> legendInfo;
  for ( unsigned int iPlot=0; iPlot<inObj.size(); iPlot++ ) {
    legendInfo.push_back( map<string, int>());
    legendInfo.back()["color"] = colors[iPlot];
    if ( string(inObj[iPlot]->ClassName() ) == "RooDataSet" ) {
      //      ( (RooDataSet*) inObj[iPlot])->Print();
      ( (RooDataSet*) inObj[iPlot])->plotOn( frame, LineColor(  colors[iPlot] ), DataError( RooAbsData::SumW2 ) );
      legendInfo.back()["doLine"] = 0;
      legendInfo.back()["style"] = frame->getAttLine(frame->getObject(iPlot)->GetName())->GetLineStyle();
    }
    else {
      //      ( (RooAbsPdf*) inObj[iPlot])->Print();
      ( (RooAbsPdf*) inObj[iPlot])->plotOn( frame, LineColor(  colors[iPlot] ) );
      legendInfo.back()["doLine"] = 1;
      legendInfo.back()["style"] = frame->getAttMarker(frame->getObject(iPlot)->GetName())->GetMarkerStyle();
    }
  }

  frame->Draw();
  for ( unsigned int iPlot=0; iPlot<inObj.size(); iPlot++ ) {
    if ( legendInfo[iPlot]["doLine"] )    myMarkerText( 0.7, 0.9-0.05*iPlot, legendInfo[iPlot]["color"], legendInfo[iPlot]["style"], inLegend.size() ? inLegend[iPlot].c_str() : "" ); 
    else  myLineText( 0.7, 0.9-0.05*iPlot, legendInfo[iPlot]["color"], legendInfo[iPlot]["style"], inLegend.size() ? inLegend[iPlot].c_str() : "" ); 

  }

  for ( unsigned int iLatex = 0; iLatex < inLatex.size(); iLatex++ ) {
    if ( latexPos[iLatex].size() != 2 ) continue;
    bool doLabel = TString( inLatex[iLatex] ).Contains("__ATLAS");
    ParseLegend( inLatex[iLatex] );
    if ( doLabel ) ATLASLabel( latexPos[iLatex][0], latexPos[iLatex][1], inLatex[iLatex].c_str() );
    else myText( latexPos[iLatex][0], latexPos[iLatex][1], 1, inLatex[iLatex].c_str() );
  }

  string canOutName = outName + "." + mapOptionsString["extension"];
  canvas->SaveAs( canOutName.c_str() );

  //  canvas->SaveAs( TString( outName + ".pdf") );
  delete frame;
  delete canvas; canvas=0;
  return 0;
}

//================================================
vector<string> ChrisLib::PlotPerCategory( vector<TObject*> vectObj, RooCategory *cat, string prefix, vector<string> options ) {

  vector<string> plotNames;

  for ( int iCat = 0; iCat < cat->numTypes(); iCat++ ) {
    RooRealVar *varFrame=0;
    vector<string> tmpOptions( options );
    cat->setIndex( iCat );
    tmpOptions.push_back( "latex=" + string( cat->getLabel() ) );
    tmpOptions.push_back( "latexOpt=0.18 0.9" );
    vector<TObject*> outVectObj;
    for ( unsigned int iObj=0; iObj<vectObj.size(); iObj++ ) {

      if ( string( vectObj[iObj]->ClassName() ) == "RooDataSet" ) {
	RooAbsData* ds=0;
	//	vectObj[iObj]->Print();
	TIterator* dataItr = ((RooDataSet*) vectObj[iObj])->split(*cat, true)->MakeIterator();
	while ((ds = (RooAbsData*)dataItr->Next())) { // loop over all channels
	  if ( string( ds->GetName() ) != cat->getLabel() ) continue;
	  //	  ds->Print();
	  outVectObj.push_back( ds );

	  TIterator* iter = ds->get()->createIterator();
	  RooAbsPdf* parg;
	  while((parg=(RooAbsPdf*)iter->Next()) ) {
	    if ( TString( parg->GetName() ).Contains( ds->GetName() ) ) {
	      varFrame = (RooRealVar*) parg;
	      break;
	    }

	  }
	  break;
	}
      }//end if data
      else if ( string( vectObj[iObj]->ClassName() ) == "RooSimultaneous" ) {
	RooSimultaneous *pdf = (RooSimultaneous*) vectObj[iObj];
	if ( !pdf->getPdf( cat->getLabel() ) ) continue;
	outVectObj.push_back( pdf->getPdf( cat->getLabel() ) );
      }//end if roosimultaneous
      else {
	cout << vectObj[iObj]->ClassName() << " type not planned" << endl;
	exit(0);
      }
    }//end iObj

    string name = prefix + "_" + cat->getLabel();
    DrawPlot( varFrame, outVectObj, name , tmpOptions );
    plotNames.push_back( name );
  }//end iCat
  return plotNames;
}

